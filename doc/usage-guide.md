
# Contents
- [Configuring RouDi](#configuring-roudi)

# Configuring RouDi

RouDi can optionally be build with support to read the mempool config from a configuration file.
To build the feature, the cmake option `-DTOML_CONFIG=on` must be used.

The file must be passed to RouDi with the `-c` command line option.
```
./iox-roudi -c /absolute/path/to/config/file.toml
```

RouDi support several shared memory segments with different access right, to limit the read and write access between different applications.
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

When no config file is specified, a hard-coded version similar to [default config](../iceoryx_posh/etc/iceoryx/roudi_config_example.toml) will be used.
