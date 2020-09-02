# Contents
1. [Prerequisites](#prerequisites)
2. [Building with CMake](#build-with-cmake)
3. [Building with the build script](#build-with-the-build-script)
3. [Building with colcon](#build-with-colcon)

iceoryx_utils and iceoryx_posh are deployed as independent cmake packages. Posh is using some functions from utils and is depending on it. You are able to build posh and utils and integrate in into existing cmake projects.

## Prerequisites

### Mac OS

Before installing iceoryx you need a XCode installation, git and optional an installed ncurses library for
the introspection client. To install ncurses locally into your build folder follow these steps
```
cd iceoryx
ICEORYX_DIR=$PWD
mkdir -p build
cd build
git clone https://github.com/mirror/ncurses.git
cd ncurses
git checkout v6.2
./configure  --prefix=$ICEORYX_DIR/build/dependencies/ --exec-prefix=$ICEORYX_DIR/build/dependencies/ --with-termlib
make -j12
make install
```

If you would like to use our Cyclone DDS Gateway you have to install Cyclone DDS first, see
[https://github.com/eclipse-cyclonedds/cyclonedds](https://github.com/eclipse-cyclonedds/cyclonedds).

### Linux

Although we strive to be fully POSIX-compliant, we recommend using Ubuntu 18.04 and at least GCC 7.5.0 for development.

You will need to install the following packages:
    ```
    sudo apt install cmake libacl1-dev libncurses5-dev pkg-config
    ```

Additionally, there is an optional dependency to the MIT licensed cpptoml library, which is used to parse a RouDi config file for the mempool config.
[cpptoml](https://github.com/skystrife/cpptoml)

## Build with CMake

**NOTE:** Requires CMake version 3.5 or higher.

The `CMakeLists.txt` from `iceoryx_meta` can be used to easily develop iceoryx with an IDE.

 1. Clone the repository
    ```
    git clone https://github.com/eclipse/iceoryx.git
    ```

 2. Generate the necessary build files
    ```bash
    cd iceoryx
    cmake -Bbuild -Hiceoryx_meta -DTOML_CONFIG=ON
    # when you have installed external dependencies like ncurses you have to add them
    # to your prefix path
    cmake -Bbuild -Hiceoryx_meta -DTOML_CONFIG=ON -DCMAKE_PREFIX_PATH=$(PWD)/build/dependencies/
    ```

 3. Compile the source code
    ```
    cmake --build build
    ```

### With the following CMake switches you can add additional features

 |  switch  |  description |
 |:---------|:-------------|
 | `dds_gateway` | builds the iceoryx dds gateway using the cyclonedds dds stack, cyclonedds will be fetched and built as part of the build, see [cyclonedds](https://github.com/eclipse-cyclonedds/cyclonedds) for details |
 | `examples` | builds all examples |
 | `introspection` | the console introspection client which requires an installed ncurses library with terminfo support |
 | `test` | enables module-, integration- and component-tests |
 | `TOML_CONFIG` | activates config file support by using toml, if this is deactivated the central broker `RouDi` is not being build |

### With the following CMake switches you can customize the iceoryx_posh build

 |  switch  |  description |
 |:---------|:-------------|
 | `IOX_MAX_PORT_NUMBER` | the maximum number of publisher and subscriber ports `RouDi` can distribute to the clients |
 | `IOX_MAX_INTERFACE_NUMBER` | the maximum number for interface ports, which are used for e.g. gateways |
 | `IOX_MAX_SUBSCRIBERS_PER_PUBLISHER` | the maximum number of subscriber a publisher can deliver chunks |
 | `IOX_MAX_CHUNKS_ALLOCATE_PER_SENDER` | the maximum number of chunks a sender can hold at a given time |
 | `IOX_MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR` | the maximum number chunks available for the chunk history |
 | `IOX_MAX_CHUNKS_HELD_PER_RECEIVER` | the maximum number of chunks a receiver can hold at a given time |

Have a look at `iceoryx_posh/cmake/iceoryx_posh_deployment.cmake` for the default values of this constants.

## Build with the build script

As an alternative we provide our build-test script which we use to integrate iceoryx into our infrastructure.
This currently only works for Linux and QNX.

 1. Clone the repository
    ```
    git clone https://github.com/eclipse/iceoryx.git
    ```

 2. Build everything
    ```
    cd iceoryx
    ./tools/iceoryx_build_test.sh
    ```

With the following arguments you can add additional features:

 |  switch  |  description |
 |:---------|:-------------|
 | `clean`  | Removes the build directory and performs a clean build. If you have installed ncurses locally into your build directory you have to reinstall it first. |
 | `test`   | Enables module-, integration- and component-tests. The Googletest-Framework will be automatically fetched from github and the test will be executed and the end of the script. |

## Build with colcon

Alternatively, iceoryx can be built with [colcon](https://colcon.readthedocs.io/en/released/user/installation.html) to provide a smooth integration for ROS2 developers.

```
mkdir -p iceoryx_ws/src
cd $_
git clone https://github.com/eclipse/iceoryx.git
cd ..
colcon build
```

This build method makes the most sense in combination with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git)
