# Custom iceoryx platforms

The `iceoryx_platform` represents the lowest layer in iceoryx. Its task is to establish
a uniform behavior of low level system calls across all supported platforms. When one would like
to add support for another platform one can use the CMake argument `-DIOX_PLATFORM_PATH` to provide
a path to a custom platform implementation.
This may become necessary to support an unsupported compiler for a specific
platform, adjust system calls to become posix compliant or to add a new operating system.

## Build with custom iceoryx platform

Let's assume you have your platform stored in the absolute path `/home/user/newIceoryxPlatform`
you can configure cmake with the command:
```sh
cd iceoryx
cmake -Bbuild -Hiceoryx_meta -DIOX_PLATFORM_PATH=/home/user/newIceoryxPlatform
```
and build iceoryx as usual.

## Structure of a custom iceoryx platform

 * `newIceoryxPlatform` - root folder
     * `include/iceoryx_platform/` - directory where all headers must be stored
        * must contain all of the headers which you can find in
         `iceoryx_platform/linux/include/iceoryx_platform`
        * the headers must declare the same functions
        * every header must provide a - not necessary valid - implementation for all the
          declared functions. If certain functionalities are not implemented one can identify
          the broken feature by executing the hoofs and posh module and integrationtests
     * `source/` - directory where the implementation must be stored
     * `cmake/IceoryxPlatformDeployment.cmake` - file which contains the platform compile time options
        * `IOX_PLATFORM_TEMP_DIR` - path to the temp dir
        * `IOX_PLATFORM_LOCK_FILE_PATH_PREFIX` - path to the dir which will be populated with the lock files
        * `IOX_PLATFORM_UDS_SOCKET_PATH_PREFIX` - path to the dir which will be populated with the UDS socket files
     * `cmake/platform_settings.hpp.in` - file which is used as template to generate the `platform_settings.hpp`
        * contains the values defined in `cmake/IceoryxPlatformDeployment.cmake`
        * contains additional constants not exposed as compile time option
     * `cmake/IceoryxPlatformSettings.cmake` - file which contains the platform compile configuration
        * `ICEORYX_CXX_STANDARD` - must be at least `17`
        * `ICEORYX_PLATFORM_STRING` - the name of the platform
        * `ICEORYX_C_WARNINGS` - [optional] a list of flags to enable c compiler warnings
        * `ICEORYX_CXX_WARNINGS` - [optional] a list of flags to enable c++ compiler warnings
        * the file can also contain platform restrictions, for instance it can fail when a
          compiler is selected which the platform does not support
