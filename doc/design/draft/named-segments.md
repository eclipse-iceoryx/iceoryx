# Named Segments

## Summary and problem description

Shared memory configuration, as handled by the RouDi config and described in detail in 
[the configuration guide](../../website/advanced/configuration-guide.md) currently works by 
declaring one or more **segments** backed each by one or more **memory pools**.

Segments currently support configuring access controls, allowing one to specify which processes are allowed to read from/write to them.
These access controls are also used to determine which segment a publisher may use to publish messages, and 
from which segments a subscriber is allowed to receive messages. This coupling presents a number of implications:

* The name of shared memory segments created under `/dev/shm` is automatically deduced as the name of the 
POSIX user group that has write access to it.
* The above means that there can only be one segment that any given group has write access to.
* In order to determine which segment a publisher shall publish to, the permissions of the containing process are 
matched against the permissions of each segment to identify a *unique match*.
* It is therefore not possible to have a single publisher access multiple segments.
* It is also not possible to have multiple publishers in the same process which access different segments

Because of the last point, it is impractical to split topics across several segments - doing so requires publishers 
of different topics to be run in processes under different users with different group membership. Instead, applications
must rely on configuring differently sized memory pools to support topic data of different sizes. 
This has a number of drawbacks, including:

* Processes communicating over otherwise independent topics share the same memory pool. 
If any one process loans too many chunks, or fails to release chunks, it can cause starvation of ALL processes 
relying on this memory pool.
* Topic data allocation is assigned to memory pools via "bucketing" - or finding the memory pool with the smallest 
chunk size to support the loan request. This means that even if one attempts to cater a memory pool to a specific topic,
any other topics with the same size requirement will be forced to use the same memory pool.
* Faulty publishers which e.g. write past the end of their loaned message may cause faults in any process communicating
over the same shared memory segment. These faults will be difficult to trace, especially because the faulty publisher
may never write past the end of the mapped shared memory segment, and therefore may never trigger a segfault.

The above points demonstrate how the current design violates the principal of [Freedom From Interference](https://heicon-ulm.de/en/iso26262-freedom-from-interference-what-is-that/).

### Proposal

In order to overcome the above limitations, shared memory segments shall be mapped by 
name instead of by access control. This will increase flexibility while still granting all the benefits of the current access control model.

## Requirements

* The RouDi configuration must support specifying names for shared memory segments.
* If a name is not configured, the fallback behavior must be the same as it is currently - assigning the name of the current process user.
* Producers should be able to specify which segments they intend to use.
  * Publishers may specify a single segment into which they will publish.
  * Clients and Servers may specify a single segment into which they send requests and responses, respectively.
* When no segment is specified, the fallback behavior must be the same as it is currently
  * Publishers will select the *unique* segment they have write access to.
  * Clients and Servers will select the *unique* segment they have write access to, for publishing requests and responses, respectively.
* Creation will fail if a producer requests to use a segment to which it does not have write access.

### Optional additional requirements

* When initializing the runtime, the user may specify via an enum whether they wish to map
  * All segments to which the current process has read access
  * All segments to which the current process has write access
  * All segments to which the current process has either read or write access (deprecated-default)
  * No segments (future default)
* The user may additionally map specific segments after initialization by calling
  * `mapSegmentForReading(name)`
    * This will check for existence of the specified segment and map it as long as user has read access to it
    * An informative error will be returned under the following circumstances
      * The segment has already been mapped
      * The segment does not exist
      * The segment exists but the current process does not have read access to it
  * `mapSegmentForWriting(name)`
    * This will check for the existence of the specified segment and map it as long as the user has write access to it
    * An informative error will be returned under the following circumstances
      * The segment has already been mapped
      * The segment does not exist
      * The segment exists but the current process does not have write access to it
* When the user attempts to create a producer that requests a segment which is not mapped, this will trigger the error handler and cause program termination.
  * In the future if we expose a builder-pattern method of creating producers, we could instead have the builder return an error indicating the segment is not mapped.
  * An alternative to the above is to add an additional enum with configuration options allowing for the runtime to map requested segments on the fly. This increases flexibility, but because it is already possible to tell the runtime to map a specific segment, this would not add much value. A user who wishes to ensure a producer's segment is already mapped may simply call `mapSegmentForWriting(name)`, ignoring the error indicating that the segment has already been mapped.
* When a consumer is created, an error will only occur if **no** segment has been mapped. Otherwise if a producer on the same channel as the consumer uses a segment the consumer does not have access to (whether because the segment is unmapped or because the consumer does not have read access), this will result in a fatal error. 
  * Improvements could be made to the RouDi logic determining if producers and consumers are compatible - to check if the consumer has access to the segment the producer writes to. This is however considered out of scope for the current design proposal.

## Design

### Updated RouDi Config

Supporting named segments will require adding an additional `name` field to the RouDi config under the `[[segment]]` heading, as well as updating the config version Example:

```
[general]
version = 2

[[segment]]
name = "foo"

[[segment.mempool]]
size = 32
count = 10000
```

Updating the version will be necessary for older versions of RouDi to fail informatively when presented with a newer config. Otherwise RouDi can support version 1 configs as if they are version 2 configs with no `name` fields specified.

### Requesting Segments

#### When creating a Publisher

A new field will be added to the [PublisherOptions struct](../../../iceoryx_posh/include/iceoryx_posh/popo/publisher_options.hpp) as follows:

```
struct PublisherOptions
{
  ...
  ShmName_t segmentName{""};
```

#### When creating Clients and Servers

Clients and Servers will have similar fields as publishers for the messages which they produce:

```
struct ClientOptions
{
  ...
  ShmName_t requestSegmentName{""};
```

```
struct ServerOptions
{
  ...
  ShmName_t responseSegmentName{""};
```

#### When initializing the runtime

We may wish to also additionally support requesting segments upon runtime initialization. The motivation here is that if we know we will only need to use certain segments, we can avoid expending unecessary resources mapping the ones we will not use. Additionally, this can be used as an error mitigation technique to prevent communication endpoints from requesting an unsupported segment.

Currently initialization of the runtime happens in the constructor, and any errors that occur result in a contract violation and program termination. 

While it is not directly in scope of this design, it would be beneficial to refactor the Runtime implementations to use the builder pattern such that any errors may be handled through returning errors types.

Tying this together with requesting segments, this could look something like:
```
class DefaultRuntimeBuilder
{
  ...
  IOX_BUILDER_PARAMETER(MappedSegments, premappedSegments, MappedSegments::ReadAndWrite)
  IOX_BUILDER_PARAMETER(vector<ShmName_t, MAX_SHM_SEGMENTS>, shmSegmentNames, {});
  IOX_BUILDER_PARAMETER(UnmappedSegmentBehavior, unmappedSegmentBehavior, UnmappedSegmentBehavior::LoadOnDemand);
  ...
  public:
    expected<DefaultRuntime, RuntimeBuilderError> create() noexcept;
```

The `create` method of this builder would then return a custom error if:
  * A segment is requested which does not exist
  * A segment is requested which the current process does not have access to
  * A segment is requested which has already been captured by the `premappedSegments` option

Additionally, when a producer is created, the `unamppedSegmentBehavior` option would determine whether or not we fail or try to map the newly requested segment on the fly.

### Segment Matching Algorithm

Ensuring backwards compatibility means carefully crafting how we register segments. The following pseudocode demonstrates how this should work in different scenarios

#### Producers requesting write access (Publisher, Client Request, Server Response)

In RouDi, given:
* `userGroups` - A list of POSIX user groups a publishing process belongs to called 
* `segmentName` - The (possibly empty) name of a shared memory segment specified in the publisher options call
* `mappedSegments` - A list of shared memory segments

1. If `segmentName` is not empty
    1. Find the segment in `mappedSegments` matching the `segmentName`
    2. If the containing process has write access to the segment via one of its `userGroups`, return the corresponding segment information
    3. Otherwise return an error indicating the publisher does not have write access to the requested segment
2. If it is empty:
    1. Iterate over all process `userGroups`
    2. Determine if the user group name matches the name of one of the segments
    3. If it does:
        1. If a match has already been previously made, return an error indicating that the publisher must only have write access to one segment
        2. If not, verify the process has access to the segment and record the match
    4. At the end of iteration, if a matching segment has been found, return the segment information
    5. Otherwise return an error indicating that no matching segment has been found

#### Runtime requesting segments to map

In the client process, while initializing the runtime, given:
* `userGroups` - A list of POSIX user groups a publishing process belongs to called 
* `segmentContainer` - A list of shared memory segment information
* `premappedSegments` - An enum indicating which group of segments should be mapped by default
* `segmentFilter` - A (possibly empty) list of segment names to filter against

In order to determine which segments to map:

1. Map segments according to the `premappedSegments` option
  1. If read access segments are requested, map every segment the user has read access to, but not write access
  2. If write access segments are requested, map every segment the user has write access to
  3. If read and write access segmeents are requested, map every segment the user has either read or write access to
  4. If no segments are requested, do nothing
2. If `segmentFilter` is not empty
    1. Iterate over each name in the filter
    2. If there is a segment that matches the name in the filter
        1. If the segment has already been mapped, return an error indicating that a segment has been requested twice. The error should contain access permissions and the value of the `premappedSegments` option the user understands why it has already been mapped.
        2. Otherwise, if the process has access rights to that segment, map it
        2. If the process does not have access rights, return an error indicating that a segment was requested the runtime does not have access to.
    3. If there is no segment that matches the name in the filter, return an error indicating that there is no segment matching the one requested to be mapped.

## Development Roadmap

- [ ] Extend the RouDi config to support specifying segment names to be added to the `SegmentConfig`
- [ ] Add the name to the `MePooSegment` data structure and allow multiple write-access segments to be mapped.
- [ ] Update producer options structs to include segment names
- [ ] Update producer segment selection logic to take the segment name into account

### Optional

- [ ] Refactor runtime initialization to use builder pattern
- [ ] Add segment filter and apply it to segment mapping during runtime initialization. 
- [ ] Add flag to specify whether endpoints requesting non-mapped segments should fail or whether segments should be created dynamically

