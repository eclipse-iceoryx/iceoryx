# Usage guide

## Configuring RouDi

RouDi support several shared memory segments with different access rights, to limit the read and write access between different applications. Inside of these segments reside mempools where the user payload data for transfer is stored.
Based on the [conceptual guide](../../conceptual-guide.md) the end-user may want to configure the mempools with the amount of chunks and their size.

Iceoryx ships a library for RouDi named in cmake `iceoryx_posh_roudi`. This lib gives you an API for compiling your own RouDi application if needed and is part of `iceoryx_posh`. 

**NOTE**
The chunk size for the mempool needs to follow these restrictions:
1. Chunksize needs to be greater than the alignment
2. Chunksize needs to be a multiple of alignment

The value for the alignment is set to 32.

### Dynamic configuration

One way is to read a configuration dynamically at RouDi runtime (startup).
Using TOML Config in RouDi is not mandatory for configuring segments and mempools, but a comfortable alternative.

RouDi can optionally be build with support to read the mempool config from a configuration file.
To build the feature in iceoryx, the cmake option `-DTOML_CONFIG=ON` must be used. 
The `iox-roudi` build by iceoryx is with TOML support and can be used out of the box.

If you create your own RouDi application you need to link against `iceoryx_posh_config`:
```cmake
target_link_libraries(custom-roudi
    PRIVATE
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh::iceoryx_posh_config
)
```

The TOML config file can be passed to RouDi with the `-c` command line option.
```
./iox-roudi -c /absolute/path/to/config/file.toml
```

This is a common config file with format version 1:
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
With this configuration, one payload segment will be created. The access rights are set to the RouDi group id.
There are three mempools within this segment. One with 10000 chunks of 32 byte payload size, one with 10000 chunks of 128 bytes and one with 1000 chunks of 1024 bytes.

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
With this configuration, only applications from the `bar` group have write access and can allocate chunks. Applications from the `foo` group have only read access.

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

When no config file is specified, a hard-coded version similar to [default config](../../../iceoryx_posh/etc/iceoryx/roudi_config_example.toml) will be used.

### Static configuration

Another way is to have a static config which is compile-time dependent, this means that you have to recompile your RouDi application if you want to change your config (not the iceoryx_posh_roudi lib).
You can have your own sourcefile with `main()` method where you can create your custom configuration and pass it to a RouDi instantiation. 
In your CMake file for you custom RouDi you need to ensure that it is **not** linking against `iceoryx_posh_config` to have a static config.

A good example how a static config could look like can be found [here](../../../iceoryx_examples/iceperf/roudi_main_static_config.cpp).
