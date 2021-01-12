# Installation

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

If you would like to use our Cyclone DDS Gateway you have to install [Cyclone DDS](https://github.com/eclipse-cyclonedds/cyclonedds) first.

### Linux

Although we strive to be fully POSIX-compliant, we recommend using Ubuntu 18.04 and at least GCC 7.5.0 for development.

You will need to install the following packages:
```
sudo apt install gcc g++ cmake libacl1-dev libncurses5-dev pkg-config
```

Additionally, there is an optional dependency to the MIT licensed [cpptoml](https://github.com/skystrife/cpptoml) library, which is used to parse a RouDi config file for the mempool config.


## Build with CMake

**NOTE:** Requires CMake version 3.5 or higher. Building from CMake is the preferred way, for more complex actions like a coverage scan
is a script available (see chapter below).

The `CMakeLists.txt` from `iceoryx_meta` can be used to easily develop iceoryx with an IDE.

 1. Clone the repository
    ```bash
    git clone https://github.com/eclipse-iceoryx/iceoryx.git
    ```

 2. Generate the necessary build files
    ```bash
    cd iceoryx
    cmake -Bbuild -Hiceoryx_meta  #tip: to build all iceoryx components add -DBUILD_ALL to the cmake command
    # when you have installed external dependencies like ncurses you have to add them
    # to your prefix path
    cmake -Bbuild -Hiceoryx_meta -DCMAKE_PREFIX_PATH=$(PWD)/build/dependencies/
    ```

 3. Compile the source code
    ```bash
    cmake --build build
    ```
    Tip: You can fasten up the build by appending `-j 4` where 4 stands for the number of parallel build processes.
    You can choose more or less depending on your available CPU cores on your machine.

**NOTE:** Iceoryx is build in Release mode with `-O3` optimization by default. If you want to have debug symbols please
set `CMAKE_BUILD_TYPE=Debug`.

### Build options

Please take a look at the cmake file [build_options.cmake](../iceoryx_meta/build_options.cmake) to get an overview of the available build options for enabling additional features.

### Available CMake switches you can customize for the iceoryx_posh build

 |  switch  |  description |
 |:---------|:-------------|
 | `IOX_MAX_PUBLISHERS` | the maximum number of publishers one `RouDi` instance can manage |
 | `IOX_MAX_SUBSCRIBERS_PER_PUBLISHER` | the maximum number of subscriber a publisher can deliver chunks to|
 | `IOX_MAX_PUBLISHER_HISTORY` | the maximum number chunks available for the publisher history |
 | `IOX_MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY` | the maximum number of chunks a publisher can allocate at a given time |
 | `IOX_MAX_SUBSCRIBERS` | the maximum number of subscribers one `RouDi` instance can manage |
 | `IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY` | the maximum number of chunks a subscriber can hold at a given time |
 | `IOX_MAX_INTERFACE_NUMBER` | the maximum number for interface ports, which are used for e.g. gateways |

Have a look at [iceoryx_posh_deployment.cmake](../iceoryx_posh/cmake/iceoryx_posh_deployment.cmake) for the default values of this constants.

## Build with script

As an alternative we provide our build-test script which we use to integrate iceoryx into our infrastructure.
The intention of the script is to more than just building with iceoryx. This is for doing a code coverage scan or for using the adress-sanitizer.
The script currently only works for Linux and QNX, it is planned to offer a multi-platform solution.

 1. Clone the repository
    ```
    git clone https://github.com/eclipse-iceoryx/iceoryx.git
    ```

 2. Build everything
    ```
    cd iceoryx
    ./tools/iceoryx_build_test.sh build-all
    ```

You can use the help for getting an overview over the available options:
    ```
    ./tools/iceoryx_build_test.sh help
    ```

## Build with colcon

Alternatively, iceoryx can be built with [colcon](https://colcon.readthedocs.io/en/released/user/installation.html) to provide a smooth integration for ROS2 developers.

```
mkdir -p iceoryx_ws/src
cd $_
git clone https://github.com/eclipse-iceoryx/iceoryx.git
cd ..
colcon build
```

This build method makes the most sense in combination with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git)
