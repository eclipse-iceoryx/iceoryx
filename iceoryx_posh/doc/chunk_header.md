# Summary

Chunks are the transport capsules in iceoryx. They store data from a publisher as payload and are sent to one or more subscriber. Furthermore, there is some meta information which is stored alongside the payload, e.g. the size and the origin of the chunk. This data is composed in the `ChunkHeader` and located at the front of the chunk. Custom meta information can be added to extend the data from the `ChunkHeader` and tailor the chunk to specific use cases. While this makes the chunk layout more complex, this complexity would otherwise be distributed at different locations all over the code base, e.g. in the request/response feature. Additionally, the adjustments for the custom header makes arbitrary alignments of the payload trivial to implement.

# Terminology

| Name              | Description                                              |
| :---------------- | :------------------------------------------------------- |
| Chunk             | a piece of memory                                        |
| Chunk Header      | contains meta information related to the chunk           |
| Custom Header     | contains custom meta information, e.g. timestamps        |
| Payload           | the user data                                            |
| Back-Offset       | offset stored in front of the payload to calculate back to the chunk header |

# Design

## Considerations

- it's not uncommon to record chunks for a later replay -> detect incompatibilities on replay
- iceoryx runs on multiple platforms -> endianness of recorded chunks might differ
- for tracing, a chunk should be uniquely identifiable -> store origin and sequence number
- the chunk is located in the shared memory, which will be mapped to arbitrary positions in the address space of various processes -> no absolute pointer are allowed
- aligning the `ChunkHeader` to 32 bytes will ensure that all member are on the same cache line and will improve performance
- in order to reduce complexity, the alignment of the custom header must not exceed the alignment of the `ChunkHeader`

## Solution

### ChunkHeader Definition
```
struct alignas(32) ChunkHeader
{
    uint32_t m_chunkSize;
    uint8_t m_chunkHeaderVersion;
    uint8_t m_reserved1;
    uint8_t m_reserved2;
    uint8_t m_reserved3;
    uint64_t m_originId;
    uint64_t m_sequenceNumber;
    uint32_t m_payloadSize;
    uint32_t m_payloadOffset;
};
```

- **m_chunkSize** is the size of the whole chunk
- **m_chunkHeaderVersion** is used to detect incompatibilities for record&replay functionality
- **m_reserved1**, **m_reserved2**, **m_reserved3** are currently not used and set to `0`
- **m_originId** is the unique identifier of the publisher the chunk was sent from
- **m_sequenceNumber** is a serial number for the sent chunks
- **m_payloadSize** is the size of the used payload of the chunk
- **m_payloadOffset** is the offset of the payload relative to the begin of the chunk

### Framing

For back calculation from the payload pointer to the `ChunkHeader` pointer, the payload offset must be accessible from a defined position relative to the payload. Lets call this `back-offset`. This is solved by storing the offset in the 4 bytes in front of the payload. In case of a simple layout where the `ChunkHeader` is adjacent to the payload, this nicely overlaps with the position of `m_payloadOffset` and no memory is wasted. In more complex cases, the offset has to be stored a second time. If the payload alignment requires some padding from the header extension, this memory is used to store the offset.

1. No custom header and alignment doesn't exceed the `ChunkHeader` alignment

```
  sizeof(ChunkHeader)     m_payloadSize
|-------------------->|------------------->|
|                     |                    |
+=====================+====================+==================================+
|                     |                    |                                  |
|     ChunkHeader     |      Payload       |  Padding                         |
|                     |                    |                                  |
+=====================+====================+==================================+
|                     |                                                       |
|   m_payloadOffset   |                                                       |
|-------------------->|                                                       |
|                                m_chunkSize                                  |
|---------------------------------------------------------------------------->|
```

2. No custom header and alignment exceeds the `ChunkHeader` alignment

```
  sizeof(ChunkHeader)            back-offset    m_payloadSize
|-------------------->|                |<---|------------------->|
|                     |                |    |                    |
+=====================+=====================+====================+============+
|                     |                .    |                    |            |
|     ChunkHeader     |                .    |      Payload       |  Padding   |
|                     |                .    |                    |            |
+=====================+=====================+====================+============+
|                                           |                                 |
|           m_payloadOffset                 |                                 |
|------------------------------------------>|                                 |
|                                                                             |
|                                m_chunkSize                                  |
|---------------------------------------------------------------------------->|
```

Depending on the address of the chunk there is the chance that `ChunkHeader` is the still adjacent to the payload. In this case, the framing looks exactly like it case 1.

3. Custom header is used

```
  sizeof(ChunkHeader)            back-offset    m_payloadSize
|-------------------->|                |<---|------------------->|
|                     |                |    |                    |
+=====================+=====================+====================+============+
|                     |   Custom    |  .    |                    |            |
|     ChunkHeader     | ChunkHeader |  .    |      Payload       |  Padding   |
|                     |  Extension  |  .    |                    |            |
+=====================+=====================+====================+============+
|                                           |                                 |
|           m_payloadOffset                 |                                 |
|------------------------------------------>|                                 |
|                                                                             |
|                                m_chunkSize                                  |
|---------------------------------------------------------------------------->|
```

### Payload Offset Calculation

1. No custom header and payload alignment doesn't exceed the `ChunkHeader` alignment

```
payloadOffset = sizeof(ChunkHeader);
```

2. No custom header and payload alignment exceeds the `ChunkHeader` alignment, which means the payload is either adjacent to the `ChunkHeader` or there is a padding with at least the size of `ChunkHeader` in front of the payload and therefore enough space to store the `back-offset`

```
headerEndAddress = addressof(chunkHeader) + sizeof(chunkHeader);
payloadAddress = align(headerEndAddress, payloadAlignment);
payloadOffset = payloadAddress - addressof(chunkHeader);
```

3. Custom header is used

```
headerEndAddress = addressof(chunkHeader) + sizeof(chunkHeader) + sizeof(customHeader);
potentialBackOffsetAddress = align(headerEndAddress, alignof(payloadOffset));
potentialPayloadAddress = potentialBackOffsetAddress + sizeof(payloadOffset);
payloadAddress = align(potentialPayloadAddress, payloadAlignment);
payloadOffset = payloadAddress - addressof(chunkHeader);
```

### Required Chunk Size Calculation

In order to fit the custom header and the payload into he chunk, a worst case calculation has to be done. We can assume that a chunk is aligned to 32 bytes, which is also the alignment of the `ChunkHeader`.

1. No custom header and payload alignment doesn't exceed the `ChunkHeader` alignment

```
chunkSize = sizeof(chunkHeader) + payloadSize;
```

2. No custom header and payload alignment exceeds the `ChunkHeader` alignment

Worst case scenario is when a part of the `ChunkHeader` crosses the payload alignment boundary, so that the payload must be aligned to the next boundary.
```
sizeLeftOfThePayloandAlignmentBoundary = sizeof(chunkHeader) - alignof(chunkHeader);
chunkSize = sizeLeftOfThePayloandAlignmentBoundary + payloadAlignment + payloadSize;
```

3. Custom header is used

Similar to case 2, but in this case it is the `back-offset` which might cross the payload alignment boundary.
```
headerSize = sizeof(chunkHeader) + sizeof(customHeader)
sizeLeftOfThePayloandAlignmentBoundary = align(headerSize, alignof(payloadOffset));
maxAlignment = max(alignof(payloadOffset), payloadAlignment);
chunkSize = sizeLeftOfThePayloandAlignmentBoundary + maxAlignment + payloadSize;
```

### Accessing Chunk Header Extension

The `ChunkHeader` has a template method to get custom header. There is a risk to use this wrong by accident, but there is currently no better solution for this.

Since the custom header is always adjacent to the `ChunkHeader`, the formula to get the custom header is:
```
customHeader = addressOf(chunkHeader) + sizeof(chunkHeader);
```

### ChunkHeader Methods

- `void* payload()` returns a pointer to the payload
- `template <typename T> T* customHeader()` returns a pointer to the custom header
- `static ChunkHeader* fromPayload(const void* const payload)` returns a pointer to the `ChunkHeader` associated to the payload

### Integration Into Publisher/Subscriber API

The `Publisher` has additional template parameters for the custom header and payload alignment. Alternatively this could also be done with the `allocate` method, but that increases the risk of using it wrong by accident.

Additionally, to fully integrate the custom header into the regular control flow, a pointer to a `ChunkHeaderHook` can be passed to the `Publisher` c'tor.

The `ChunkHeaderHook` is not in the shared memory and has the following virtual methods:
- `virtual void allocateHook(ChunkHeader& chunkHeader)` can be used to initialize the custom header
- `virtual void deliveryHook(ChunkHeader& chunkHeader)` can be used to set members of the custom header, e.g. a timestamp

Furthermore, the `Publisher` and `Subscriber` have access to the `ChunkHeader` and can use the `customHeader()` method to gain access to the custom header.

### Pitfalls & Testing

- when the payload is adjacent to the `ChunkHeader`, it must be ensured that `m_payloadOffset` overlaps with the `back-offset`, which is `sizeof(m_payloadOffset)` in front of the payload
- to simplify calculation, it is assumed that the alignment of the custom header doesn't exceed the alignment of the `ChunkHeader`. This has to be enforced with an `assert`

# Open Issues

- the design was done with the intention to have a custom header of arbitrary size, if the size is limited to e.g. 32 bytes, some things could be simplified
- untyped API and C bindings
    - PoC is needed
- user defined sequence number
    - this can probably be done by a `ChunkHeaderHook`
    - alternatively, a flag could be introduce into the `ChunkHeader`
- user defined timestamp
    - this should probably be in a custom header
- mempool configuration
    - currently we specify the payload size and the size of the `ChunkHeader` is added automatically
    - with the new approach, part of the specified payload size might be used for the custom header
    - part of the specified payload might also be used as padding for the payload alignment
    - shall we change this back to just specifying the full chunk size?
    - shall we add an option to specify the custom header size and the payload alignment?
- is it necessary to store a flag in `ChunkHeader` if a custom extension is used?
- do we want to store the version of the custom extension in the `ChunkHeader`, similarly to `m_chunkHeaderVersion`
- for record&replay the custom header is totally opaque, which might lead to problems if e.g. a timestamp is stored in the custom header and needs to be updated for the replay
