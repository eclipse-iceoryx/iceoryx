
# Contents
- [Configuring RouDi](#configuring-roudi)
- [CaPro protocol](#capro-protocol)


# Configuring RouDi

RouDi can optionally be build with support to read the mempool config from a configuration file.
To build the feature, the cmake option `-DTOML_CONFIG=ON` must be used.

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

When no config file is specified, a hard-coded version similar to [default config](../../../iceoryx_posh/etc/iceoryx/roudi_config_example.toml) will be used.

# CaPro protocol

<!-- @todo Move this section to overview.md >

<!-- @todo Add table and explain different terminologies used in AUTOSAR, ROS and DDS -->

This service model comes from AUTOSAR. It is maybe not the best fit for typical publish/subscribe APIs
but it allows us a matching to different technologies. The event can be compared to a topic in other
publish/subscribe approaches. The service is not a single request/response thing but an element
for grouping of events and/or methods that can be discovered as a service. Service and instance are like
classes and objects in C++. So you always have a specific instance of a service during runtime.

# Iceoryx library build

Iceoryx consists of several libraries which have dependencies to each other. The goal is here to have self-encapsulated library packages available
where the end-user can easily find it with the cmake command `find-package(...)`.
In the default case the iceoryx libraries are installed by `make install` into `/usr/lib` which need root access. To avoid that cmake gives you the possibility to install the libs into a custom folder.
This can be done by setting `-DCMAKE_INSTALL_PREFIX=/custom/install/path` as build-flag for the CMake file in iceoryx_meta.

Iceoryx_meta is a Cmake file which collects all libraries (utils, posh etc.) and extensions (binding_c, dds) together to have a single point for building. 
The alternate solution is provided for Ubuntu-users by having a build script `iceoryx_build_test.sh` in the tools folder.

Per default iceoryx is build as shared libraries because it is a cleaner solution for resolving dependency issues and it reduces the linker time while building.
This is done by the flag `BUILD_SHARED_LIBS` which is set to ON per default. If you want to have static libraries, just pass `-DBUILD_SHARED_LIBS=OFF` to Cmake or use `build-static` as flag in the build script.

If you want to share the iceoryx to other users, you can also create a debian package. You can create it by calling: `./tools/iceoryx_build_test.sh package` where it will place it in the folder `build_package`.