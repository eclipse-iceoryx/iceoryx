# icehello with minimal memory usage

## Introduction

In some cases, it is needed to run iceoryx on systems with limited memory
availability. For such cases, it is possible to configure the resource
management limits as documented in the
[configuration guide](../../doc/website/advanced/configuration-guide.md).

This configuration has two parts:
* The resource limits set in cmake during configuration with options in the
format `IOX_*`. This will influence the management shared memory segment size.
* The memory pool configuration, set either in source code when creating the
RouDi object, or when starting `iox-roudi` by specifying a TOML configuration
file. This will influence the individual memory pool shared memory segment
sizes.

This is an example guide showing how to run the basic hello world example with
truly minimal resource limits to achieve shared memory usage of less than 100
kB.

## How to build

Build iceoryx RouDi application and the hello world example with the given
CMake option configuration for minimal memory usage. For example, as follows:

```
cmake -Bbuild -Hiceoryx_meta -Ciceoryx_examples/small_memory/options.cmake -DEXAMPLES=ON
cmake --build build --target iox-roudi iox-cpp-publisher-helloworld iox-cpp-subscriber-helloworld
```

Please note that it might be impossible to build some other iceoryx examples,
since the decreased maximum length of service and runtime name strings might
cause some static assertion failures. This problem demonstrates that the user
has to be careful when decreasing the iceoryx resource limits set during cmake
configuration.

## How to run

Launch the RouDi process with the given TOML config file:

```
build/iox-roudi -c iceoryx_examples/small_memory/roudi_config.toml &
```

Optionally, it is possible to specify the `-l debug` flag to enable logging and
see the memory allocation size:

```
2022-11-24 11:51:20.564 [Debug]: Trying to reserve 88248 bytes in the shared memory [iceoryx_mgmt]
2022-11-24 11:51:20.980 [Debug]: Acquired 88248 bytes successfully in the shared memory [iceoryx_mgmt]
2022-11-24 11:51:20.094 [Debug]: Registered memory segment 0x7f0b73597000 with size 88248 to id 1
2022-11-24 11:51:20.618 [Debug]: Trying to reserve 5520 bytes in the shared memory [user]
2022-11-24 11:51:20.193 [Debug]: Acquired 5520 bytes successfully in the shared memory [user]
2022-11-24 11:51:20.236 [Debug]: Roudi registered payload data segment 0x7f0b73b53000 with size 5520 to id 2
RouDi is ready for clients
```

Here, RouDi is using ~88 kB for the management segment and ~5.5 kB for the
user memory segment. Note that the precise sizes can differ on every platform.

Next, we can run the basic hello world example as usual:

```
build/iceoryx_examples/icehello/iox-cpp-subscriber-helloworld &
build/iceoryx_examples/icehello/iox-cpp-publisher-helloworld &
```

This demonstrates that the demo is still fully functional, even with the
greatly reduced resource limits.

To terminate the demo, just kill all the processes:

```
killall -r iox
```

## Expected Output

[![asciicast](https://asciinema.org/a/KWK3IaXSYGqVvX2GG9zH2muaH.svg)](https://asciinema.org/a/KWK3IaXSYGqVvX2GG9zH2muaH)
