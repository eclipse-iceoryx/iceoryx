# Chunk-Header

## Summary

Chunks are the transport capsules in iceoryx. They store data from a publisher as payload and are sent to one or more subscriber. Furthermore, there is some meta information which is stored alongside the payload, e.g. the size and the origin of the chunk. This data is composed in the `ChunkHeader` and located at the front of the chunk. Custom meta information can be added to extend the data from the `ChunkHeader` and tailor the chunk to specific use cases. While this makes the chunk layout more complex, this complexity would otherwise be distributed at different locations all over the code base, e.g. in the request/response feature. Additionally, the adjustments for the user-header makes arbitrary alignments of the user-payload trivial to implement.

## Terminology

| Name              | Description                                              |
| :---------------- | :------------------------------------------------------- |
| Chunk             | a piece of memory                                        |
| Chunk-Header      | contains meta information related to the chunk           |
| Chunk-Payload     | the part of the chunk for the payload which is split into user-header and user-payload |
| User-Header       | contains custom meta information, e.g. timestamps        |
| User-Payload      | the user data with custom alignment                      |
| Back-Offset       | offset stored in front of the user-payload to calculate back to the chunk-header (for the most simple case it will overlap with the user-payload offset stored in the Chunk-Header) |

Framing with terminology
```
+=============================================================================+
|                                 Chunk                                       |
+===================+=========================================================+
|                   |                    Chunk-Payload                        |
|   Chunk-Header    +===============+=======+====================+============+
|                   |  User-Header  |   ¦ ᶺ |    User-Payload    |  Padding   |
+===================+===============+=====|=+====================+============+
                                          └ Back-Offset
```

## Design

### Considerations

- it's not uncommon to record chunks for a later replay -> detect incompatibilities on replay
- iceoryx runs on multiple platforms -> endianness of recorded chunks might differ
- for tracing, a chunk should be uniquely identifiable -> store origin and sequence number
- the chunk is located in the shared memory, which will be mapped to arbitrary positions in the address space of various processes -> no absolute pointer are allowed
- aligning the `ChunkHeader` to 32 bytes will ensure that all member are on the same cache line and will improve performance
- in order to reduce complexity, the alignment of the user-header must not exceed the alignment of the `ChunkHeader`

### Solution

#### ChunkHeader Definition
```
struct alignas(32) ChunkHeader
{
    uint32_t chunkSize;
    uint8_t chunkHeaderVersion;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint64_t originId;
    uint64_t sequenceNumber;
    uint32_t userPayloadSize;
    uint32_t userPayloadOffset;
};
```

- **chunkSize** is the size of the whole chunk
- **chunkHeaderVersion** is used to detect incompatibilities for record&replay functionality
- **reserved1**, **reserved2**, **reserved3** are currently not used and set to `0`
- **originId** is the unique identifier of the publisher the chunk was sent from
- **sequenceNumber** is a serial number for the sent chunks
- **userPayloadSize** is the size of the chunk occupied by the user-payload
- **userPayloadOffset** is the offset of the user-payload relative to the begin of the chunk

#### Framing

For back calculation from the user-payload pointer to the `ChunkHeader` pointer, the user-payload offset must be accessible from a defined position relative to the user-payload. Lets call this `back-offset`. This is solved by storing the offset in the 4 bytes in front of the user-payload. In case of a simple layout where the `ChunkHeader` is adjacent to the user-payload, this nicely overlaps with the position of `userPayloadOffset` and no memory is wasted. In more complex cases, the offset has to be stored a second time. If the user-payload alignment requires some padding from the header extension, this memory is used to store the offset.

1. No user-header and user-payload alignment doesn't exceed the `ChunkHeader` alignment

```
 sizeof(ChunkHeader)    userPayloadSize
|------------------>|--------------------->|
|                   |                      |
+===================+======================+==================================+
| Chunk-Header  ¦ * |     User-Payload     |  Padding                         |
+===================+======================+==================================+
|                   |                                                         |
| userPayloadOffset |                                                         |
|------------------>|                                                         |
|                                 chunkSize                                   |
|---------------------------------------------------------------------------->|

*) userPayloadOffset from ChunkHeader and back-offset are overlapping
```

2. No user-header and user-payload alignment exceeds the `ChunkHeader` alignment

```
 sizeof(ChunkHeader)             back-offset   userPayloadSize
|------------------>|                  |<---|------------------->|
|                   |                  |    |                    |
+===================+=======================+====================+============+
|   Chunk-Header    |                  ¦    |    User-Payload    |  Padding   |
+===================+=======================+====================+============+
|                                           |                                 |
|          userPayloadOffset                |                                 |
|------------------------------------------>|                                 |
|                                                                             |
|                                 chunkSize                                   |
|---------------------------------------------------------------------------->|
```

Depending on the address of the chunk there is the chance that `ChunkHeader` is the still adjacent to the user-payload. In this case, the framing looks exactly like in case 1.

3. User-Header is used

```
 sizeof(ChunkHeader)             back-offset   userPayloadSize
|------------------>|                  |<---|------------------->|
|                   |                  |    |                    |
+===================+===============+=======+====================+============+
|   Chunk-Header    |  User-Header  |  ¦    |    User-Payload    |  Padding   |
+===================+===============+=======+====================+============+
|                                           |                                 |
|          userPayloadOffset                |                                 |
|------------------------------------------>|                                 |
|                                                                             |
|                                 chunkSize                                   |
|---------------------------------------------------------------------------->|
```

#### User-Payload Offset Calculation

1. No user-header and user-payload alignment doesn't exceed the `ChunkHeader` alignment

```
userPayloadOffset = sizeof(ChunkHeader);
```

2. No user-header and user-payload alignment exceeds the `ChunkHeader` alignment, which means the user-payload is either adjacent to the `ChunkHeader` or there is a padding with at least the size of `ChunkHeader` alignment in front of the user-payload and therefore enough space to store the `back-offset`

```
chunkHeaderEndAddress = addressof(chunkHeader) + sizeof(chunkHeader);
alignedUserPayloadAddress = align(chunkHeaderEndAddress, userPayloadAlignment);
userPayloadOffset = alignedUserPayloadAddress - addressof(chunkHeader);
```

3. User-Header is used

```
chunkHeaderEndAddress = addressof(chunkHeader) + sizeof(chunkHeader) + sizeof(userHeader);
anticipatedBackOffsetAddress = align(chunkHeaderEndAddress, alignof(userPayloadOffset));
unalignedUserPayloadAddress = anticipatedBackOffsetAddress + sizeof(userPayloadOffset);
alignedUserPayloadAddress = align(unalignedUserPayloadAddress, userPayloadAlignment);
userPayloadOffset = alignedUserPayloadAddress - addressof(chunkHeader);
```

#### Required Chunk Size Calculation

In order to fit the user-header and the user-payload into the chunk, a worst case calculation has to be done. We can assume that a chunk is sufficiently aligned for the `ChunkHeader`.

1. No user-header and user-payload alignment doesn't exceed the `ChunkHeader` alignment

```
chunkSize = sizeof(chunkHeader) + userPayloadSize;
```

2. No user-header and user-payload alignment exceeds the `ChunkHeader` alignment

Worst case scenario is when a part of the `ChunkHeader` crosses the user-payload alignment boundary, so that the user-payload must be aligned to the next boundary. Currently this is not possible, since the size equals the alignment of the `ChunkHeader`. This might change if more member are added to the `ChunkHeader` or the alignment is reduced. The following drawing demonstrates this scenario.

```
                               ┌ back-offset
    +===============+==========|+===============================+
    |  ChunkHeader  |         ¦ᵛ|        User-Payload           |
    +===============+===========+===============================+

⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥ <- ChunkHeader alignment boundaries
⊥               ⊥               ⊥               ⊥ <- user-payload alignment boundaries
    <-----------|--------------->
        pre       user-payload  |------------------------------->
    user-payload   alignment           user-payload size
     alignment
      overhang
```

The following formula is used to calculate the required chunk size.

```
preUserPayloadAlignmentOverhang = sizeof(chunkHeader) - alignof(chunkHeader);
chunkSize = preUserPayloadAlignmentOverhang + userPayloadAlignment + userPayloadSize;
```

3. User-Header is used

Similar to case 2, but in this case it is the `back-offset` which might cross the user-payload alignment boundary.

```
                                               ┌ back-offset with same alignment
                                               | as userPayloadOffset
    +===============+===========+==============|+===============================+
    |  ChunkHeader  | UserHeader|             ¦ᵛ|         User-Payload          |
    +===============+===========+===============+===============================+

                            ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ <- userPayloadOffset alignment boundaries
⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥   ⊥ <- ChunkHeader alignment boundaries
⊥               ⊥               ⊥               ⊥ <- userPayload alignment boundaries
    <---------------------------|--------------->
     pre user-payload alignment   user-payload  |------------------------------->
            overhang               alignment            user-payload size
```

The following formula is used to calculate the required chunk size.

```
chunkHeaderSize = sizeof(chunkHeader) + sizeof(userHeader)
preUserPayloadAlignmentOverhang = align(chunkHeaderSize, alignof(userPayloadOffset));
maxPadding = max(sizeof(userPayloadOffset), userPayloadAlignment);
chunkSize = preUserPayloadAlignmentOverhang + maxPadding + userPayloadSize;
```

#### Accessing Chunk-Header Extension

The `ChunkHeader` has a template method to get user-header. There is a risk to use this wrong by accident, but there is currently no better solution for this.

Since the user-header is always adjacent to the `ChunkHeader`, the formula to get the user-header is:
```
userHeader = addressOf(chunkHeader) + sizeof(chunkHeader);
```

#### ChunkHeader Methods

- `void* userPayload()` returns a pointer to the user-payload
- `template <typename T> T* userHeader()` returns a pointer to the user-header
- `static ChunkHeader* fromUserPayload(const void* const userPayload)` returns a pointer to the `ChunkHeader` associated to the user-payload

#### Integration Into Publisher/Subscriber API

The `Publisher` has additional template parameters for the user-header and user-payload alignment. Alternatively this could also be done with the `allocate` method, but that increases the risk of using it wrong by accident.

Additionally, to fully integrate the user-header into the regular control flow, a pointer to a `ChunkHeaderHook` can be passed to the `Publisher` c'tor.

The `ChunkHeaderHook` is not in the shared memory and has the following virtual methods:
- `virtual void allocateHook(ChunkHeader& chunkHeader)` can be used to initialize the user-header
- `virtual void deliveryHook(ChunkHeader& chunkHeader)` can be used to set members of the user-header, e.g. a timestamp

Furthermore, the `Publisher` and `Subscriber` have access to the `ChunkHeader` and can use the `userHeader()` method to gain access to the user-header.

#### Pitfalls & Testing

- when the user-payload is adjacent to the `ChunkHeader`, it must be ensured that `userPayloadOffset` overlaps with the `back-offset`, which is `sizeof(userPayloadOffset)` in front of the user-payload
- to simplify calculation, it is assumed that the alignment of the user-header doesn't exceed the alignment of the `ChunkHeader`. This has to be enforced with an `assert`

## Open Issues

- the design was done with the intention to have a user-header of arbitrary size, if the size is limited to e.g. 32 bytes, some things could be simplified
- publisher/subscriber API proposal
```
// publisher
auto pub = iox::popo::Publisher<MyPayload, MyUserHeader>(serviceDescription);
pub.loan()
    .and_then([&](auto& sample) {
        sample.getHeader()->userHeader<MyUserHeader>()->data = 42;
        sample->a = 42;
        sample->b = 13;
        sample.publish();
    })
    .or_else([](iox::popo::AllocationError) {
        // Do something with error.
    });

// subscriber
auto sub = iox::popo::Subscriber<MyPayload, MyUserHeader>(serviceDescription);
sub->take()
    .and_then([](auto& sample){
        std::cout << "User-Header data: " << sample.getHeader()->userHeader<MyUserHeader>()->data << std::endl;
        std::cout << "User-Payload data: " << static_cast<const MyPayload*>(sample->get())->data << std::endl;
    });
```
    - the publisher/subscriber would have a default parameter for the user-header to be source compatible with our current API
    - the drawback is that the user could use the wrong user-header. Maybe `Sample` also needs an additional template parameter
    - additionally, a `ChunkHeaderHook` could be used on the publisher side
```
template <typename UserHeader>
class MyChunkHeaderHook : public ChunkHeaderHook
{
  public:
    void deliveryHook(ChunkHeader& chunkHeader) override
    {
        chunkHeader.userHeader<UserHeader>().timestamp = myTimeProvider::now();
    }
};

auto userHeaderHook = MyChunkHeaderHook<MyUserHeader>();
auto pub = iox::popo::Publisher<MyPayload>(serviceDescription, userHeaderHook);
```
        - alternatively, instead of the ChunkHeaderHook class, the publisher could have a method `registerDeliveryHook(std::function<void(ChunkHeader&)>)`
        - allow the user only read access to the `ChunkHeader` and write access to the `UserHeader`
- untyped publisher/subscriber API proposal
```
// publisher option 1
auto pub = iox::popo::UntypedPublisher<MyUserHeader>(serviceDescription);

// publisher option 2
auto userHeaderSize = sizeOf(MyUserHeader);
auto pub = iox::popo::UntypedPublisher(serviceDescription, userHeaderSize);

auto payloadSize = sizeof(MyPayload);
auto payloadAlignment = alignof(MyPayload);
pub.loan(payloadSize, payloadAlignment)
    .and_then([&](auto& sample) {
        sample.getHeader()->userHeader<MyUserHeader>()->data = 42;
        auto payload = new (sample.get()) MyPayload();
        payload->data = 73;
        sample.publish();
    })
    .or_else([](iox::popo::AllocationError) {
        // Do something with error.
    });

// subscriber option 1
auto pub = iox::popo::UntypedPublisher<MyUserHeader>(serviceDescription);

// subscriber option 2
auto userHeaderSize = sizeOf(MyUserHeader);
auto pub = iox::popo::UntypedSubscriber(serviceDescription, userHeaderSize);
sub->take()
    .and_then([](auto& sample){
        std::cout << "User-Header data: " << sample.getHeader()->userHeader<MyUserHeader>()->data << std::endl;
        std::cout << "User-Payload data: " << static_cast<const MyPayload*>(sample->get())->data << std::endl;
    });
```
    - option 1 has the benefit to catch a wrong alignment of the user-header and would be necessary if we make the `Sample` aware of the user-header
- C bindings
    - PoC is needed
- user defined sequence number
    - this can probably be done by a `ChunkHeaderHook`
    - alternatively, a flag could be introduce into the `ChunkHeader`
- user defined timestamp
    - this should probably be in a user-header
- mempool configuration
    - currently we specify the chunk-payload size and the size of the `ChunkHeader` is added automatically
    - with the new approach, part of the specified chunk-payload size might be used for the user-header
    - part of the specified chunk-payload might also be used as padding for the user-payload alignment
    - the user will continue to specify the chunk-payload; if a user-header or custom user-payload alignment is used, the user needs to take this into account
- is it necessary to store a flag in `ChunkHeader` if a user extension is used?
    - we could maintain a list of known user-header IDs or ranges of IDs similar to `IANA` https://tools.ietf.org/id/draft-cotton-tsvwg-iana-ports-00.html#privateports
    - the IDs could be stored in the `ChunkHeader` and everything with an ID larger than `0xC000` is free to use
    - to make this somewhat save, the `ChunkHeaderHook` must be mandatory with e.g. a `virtual uint16_t getId() = 0;` method which will be called in the `ChunkSender`
        - alternatively, the user-header struct must have a `constexpr uint16_t USER_HEADER_ID`; if it's not present, we could set the ID to `0xC000` which is the first ID free to use
- do we want to store the version of the user extension in the `ChunkHeader`, similarly to `chunkHeaderVersion`
- for record&replay the user-header is totally opaque, which might lead to problems if e.g. a timestamp is stored in the user-header and needs to be updated for the replay
    - if we maintain a list of known user-header IDs and also store the user-header version, a record&replay framework could implement the required conversions
- for record&replay it is necessary to store the alignment of the user-payload; decide if a uint16_t should be used of if just the alignment power should be stored in a uint8_t
