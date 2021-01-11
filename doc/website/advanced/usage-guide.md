# Usage guide

## Configuring RouDi

RouDi support several shared memory segments with different access rights, to limit the read and write access between different applications. Inside of these segments reside mempools where the user payload data for transfer is stored.
Based on the [conceptual guide](../../conceptual-guide.md) the end-user may want to configure the mempools with the amount of chunks and their size.

Iceoryx ships a library for RouDi named in cmake `iceoryx_posh_roudi`. This lib gives you an API for compiling your own RouDi application if needed and is part of `iceoryx_posh`. 
### dynamic configuration

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

### static configuration

Another way is to have a static config which is compile-time dependent, this means that you have to recompile your RouDi application if you want to change your config (not the iceoryx_posh_roudi lib).
You can have your own sourcefile with `main()` method where you can create your custom configuration and pass it to a RouDi instantiation. 
In your CMake file for you custom RouDi you need to ensure that it is **not** linking against `iceoryx_posh_config` to have a static config.

A good example how a static config could look like can be found [here](../../../iceoryx_examples/iceperf/roudi_main_static_config.cpp).

## Iceoryx library build

Iceoryx consists of several libraries which have dependencies to each other. The goal is to have self-encapsulated library packages available
where the end-user can easily find it with the cmake command `find-package(...)`.
In the default case the iceoryx libraries are installed by `make install` into `/usr/lib` which need root access. To avoid that cmake gives you the possibility to install the libs into a custom folder.
This can be done by setting `-DCMAKE_INSTALL_PREFIX=/custom/install/path` as build-flag for the CMake file in iceoryx_meta.

Iceoryx_meta is a Cmake file which collects all libraries (utils, posh etc.) and extensions (binding_c, dds) together to have a single point for building. 
The alternate solution is provided for Ubuntu-users by having a build script `iceoryx_build_test.sh` in the tools folder.

Per default iceoryx is build as shared libraries because it is a cleaner solution for resolving dependency issues and it reduces the linker time while building.
This is done by the flag `BUILD_SHARED_LIBS` which is set to ON per default. If you want to have static libraries, just pass `-DBUILD_SHARED_LIBS=OFF` to Cmake or use `build-static` as flag in the build script.

If you want to share the iceoryx to other users, you can also create a debian package. You can create it by calling: `./tools/iceoryx_build_test.sh package` where it will be build it in `build_package`.


