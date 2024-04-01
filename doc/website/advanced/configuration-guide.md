# Configuration guide

## CMake switches for configuring iceoryx_hoofs and iceoryx_posh build

There are several configuration options set by default when iceoryx_hoofs
and iceoryx_posh are build. These options adjust the minimal log level compiled
into the binary and the global maximum amount of resources like Publisher and
Subscriber Ports which can have a huge impact on the memory footprint of iceoryx
since they define the size of the management structures
in the shared memory segment called `iceoryx_mgmt` when RouDi is started.

 |  switch  |  description |
 |:---------|:-------------|
 | `IOX_MIN_LOG_LEVEL` | Minimal log level which will be compiled into the binary. Lower log levels will be optimized away during compilation |
 | `IOX_MAX_PUBLISHERS` | Maximum number of publishers in one iceoryx system |
 | `IOX_MAX_SUBSCRIBERS_PER_PUBLISHER` | Maximum number of connections one publisher port can handle |
 | `IOX_MAX_PUBLISHER_HISTORY` | Maximum size of a publishers history |
 | `IOX_MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY` | Maximum number of chunks a publisher can allocate in parallel |
 | `IOX_MAX_SUBSCRIBERS` | Maximum number of subscribers in one iceoryx system |
 | `IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY` | Maximum number of chunks a subscriber can take in parallel|
 | `IOX_MAX_INTERFACE_NUMBER` | Maximum number of interface ports which are used by gateways |
 | `IOX_MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY` | Maximum number of server can process request in parallel |

Have a look at [IceoryxHoofsDeployment.cmake](../../../iceoryx_hoofs/cmake/IceoryxHoofsDeployment.cmake) and
[IceoryxPoshDeployment.cmake](../../../iceoryx_posh/cmake/IceoryxPoshDeployment.cmake) for the default values of the constants.

!!! hint
    With the default values set, the size of `iceoryx_mgmt` is ~64.5 MByte. You
    can reduce the size by decreasing the values from the table via the CMake
    options. The current values are printed in the CMake stage when building iceoryx.

Example:

```bash
cmake -Bbuild -Hiceoryx_meta -DIOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY=64
```

With that change, the footprint of the management segment is reduced to ~52.7 MBytes.
For larger use cases you can increase the value to avoid that samples are dropped
on the subscriber side (see also [#615](https://github.com/eclipse-iceoryx/iceoryx/issues/615)).

## Configuring Mempools for RouDi

RouDi supports several shared memory segments with different access rights, to
limit the read and write access between different applications. Memory pools
manage those segments and organize the user payload data required
for communication.

!!! note
    Actually only the chunk-payload size is configured and the size of the
    `ChunkHeader` will be added to the configured size. If a user-header or a
    user-payload alignment larger than 8 is used, the available size for the
    user-payload will be smaller than the configured chunk-payload since some
    space is needed for the other functionality.
    Please have a look at the
    [chunk_header.md](../../design/chunk_header.md)
    design document for a formula how to determine the necessary chunk-payload
    size with user-header and extended user-payload alignment.

For building RouDi, iceoryx ships a library named `iceoryx_posh_roudi`. This lib
provides you an API for compiling your own RouDi application and is part of `iceoryx_posh`.

!!! note
    The chunk size for the memory pools needs to comply with the following restrictions:

    1. Chunksize needs to be greater than the alignment
    2. Chunksize needs to be a multiple of the alignment

The value for the alignment is set to 8.

### Dynamic configuration

One way is to read a configuration dynamically during the startup of RouDi.
Using the TOML Config in RouDi is not mandatory for configuring segments and
mempools, but a comfortable alternative.

The CMake option `-DTOML_CONFIG`, which is `ON` by default, enables the TOML config
for iceoryx.
The `iox-roudi` application provided by iceoryx is compiled with TOML support and
can be used out of the box.

If you create your own RouDi application you need to link against `iceoryx_posh_config`:

```cmake
target_link_libraries(custom-roudi
    PRIVATE
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh::iceoryx_posh_config
)
```

The TOML config file can be passed to RouDi with the `-c` command-line option.

```bash
./iox-roudi -c /absolute/path/to/config/file.toml
```

This is an examplary config file with format version 1:

```TOML
[general]
version = 1

[[segment]]

[[segment.mempool]]
size = 32
count = 10000

[[segment.mempool]]
size = 128
count = 10000

[[segment.mempool]]
size = 1024
count = 1000
```

With this configuration, one payload segment will be created. The access rights
are set to the RouDi group id as default.
There are three mempools within this segment. One with 10000 chunks of 32 byte
payload size, one with 10000 chunks of 128 bytes, and one with 1000 chunks of
1024 bytes.

To restrict the access, a reader and writer group can be set:

```TOML
[general]
version = 1

[[segment]]
reader = "foo"
writer = "bar"

[[segment.mempool]]
size = 32
count = 10000

[[segment.mempool]]
size = 128
count = 10000

[[segment.mempool]]
size = 1024
count = 1000
```

With this configuration, only applications from the `bar` group have write access
and can allocate chunks. Applications from the `foo` group have only read access.

This is an example with multiple segments:

```TOML
[general]
version = 1

[[segment]]
reader = "foo"
writer = "bar"

[[segment.mempool]]
size = 32
count = 10000

[[segment]]
reader = "alice"
writer = "eve"

[[segment.mempool]]
size = 1024
count = 100
```

When no configuration file is specified a hard-coded version similar to the
[default config](../../../iceoryx_posh/etc/iceoryx/roudi_config_example.toml)
will be used.

### Static configuration

Another way is to have a static configuration that is compiled into the roudi application.
As a consequence you have to recompile your RouDi application if you want to change
your config (not the `iceoryx_posh_roudi` lib).
You can create your custom configuration in the `main()` function where you then pass
it as constructor argument to the RouDi instance.
In the cmake file entry of the custom RouDi executable you need to ensure that it
is **not** linking against `iceoryx_posh_config` to ensure using the static configuration.

```cpp
int main(int argc, char* argv[])
{
    iox::IceoryxConfig config;

    // create mempools
    iox::mepoo::MePooConfig mepooConfig;
    mepooConfig.addMemPool({128, 10000}); // payload in bytes, chunk count
    mepooConfig.addMemPool({265, 10000});

    auto currentGroup = iox::PosixGroup::getGroupOfCurrentProcess();
    config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mepooConfig});

    // configure the chunk count for the introspection; each introspection topic gets this number of chunks
    config.introspectionChunkCount = 10;

    // configure the chunk count for the service discovery
    config.discoveryChunkCount = 10;

    // create a roudi instance
    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    IceOryxRouDiApp roudi(cmdLineParser.parse(argc, argv).expect("Valid CLI parameter"), config);

    // run roudi
    return roudi.run();
}
```

A working example of a static config can be found
[here](../../../iceoryx_examples/iceperf/roudi_main_static_config.cpp).
