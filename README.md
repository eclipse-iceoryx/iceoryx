# iceoryx - true zero-copy inter-process-communication

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/70233652-4aa6d180-175f-11ea-8524-2344e0d3935c.png" width="50%">
</p>

[![Build & Test](https://github.com/eclipse/iceoryx/workflows/Build%20&%20Test/badge.svg?branch=master)](https://github.com/eclipse/iceoryx/actions)
[![Gitter](https://badges.gitter.im/eclipse/iceoryx.svg)](https://gitter.im/eclipse/iceoryx)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Introduction

Great that you've made it to this little Eclipse project! Let's get you started by providing a quick background
tour, introducing the project scope and guide you through the examples.

So first off: What is iceoryx?

iceoryx is an inter-process-communication (IPC) middleware for various operating systems (currently we support Linux, MacOS and QNX).
It has its origins in the automotive industry, where large amounts of data have to be transferred between different processes
when it comes to driver assistance or automated driving systems. However, the efficient communication mechanisms can also be applied
to a wider range of use cases, e.g. in the field of robotics or game development. 

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/74612998-b962bc80-510a-11ea-97f0-62f41c5d287b.gif" width="100%">
</p>

iceoryx uses a true zero-copy, shared memory approach that allows to transfer data from publishers to subscribers without a single copy. 
This ensures data transmissions with constant latency, regardless of the size of the payload. For more information have a look at the 
[1000 words iceoryx introduction](https://www.eclipse.org/community/eclipse_newsletter/2019/december/4.php)

<p align="center">
<img src="https://user-images.githubusercontent.com/55156294/91751530-35af7f80-ebc5-11ea-9aed-eed590b229df.png" width="80%">
</p>

It's all about the API!

Don't get too frighten of the API when strolling through the codebase. Think of iceoryx's API as a "plumbing" one
("plumbing" as defined in Git, which means low-level). We're not using the "plumbing" API ourselves, but instead a typed API.
An example for a "porcelain" API would be [ROS2](https://www.ros.org/). Others are listed in the next section.

### Where is Eclipse iceoryx used?

|Framework | Description |
|---|---|
| [ROS2](https://github.com/ros2/rmw_iceoryx) | Eclipse iceoryx can be used inside the [robot operating system](https://www.ros.org/) with rmw_iceoryx |
| [eCAL](https://github.com/continental/ecal) | Open-source middleware from [Continental AG](https://www.continental.com/) supporting pub/sub and various message protocols |
| [RTA-VRTE](https://www.etas.com/en/products/rta-vrte.php) | [Adaptive AUTOSAR](https://www.autosar.org/standards/adaptive-platform/) platform software framework for vehicle computer from [ETAS GmbH](https://www.etas.com) |
| [Cyclone DDS](https://github.com/eclipse-cyclonedds/cyclonedds) | Performant and robust open-source DDS implementation maintained by [ADLINK Technology Inc.](https://www.adlinktech.com/) |

## Installation

iceoryx_utils and iceoryx_posh are deployed as independent cmake packages. Posh is using some functions from utils and is depending on it. You are able to build posh and utils and integrate in into existing cmake projects.

### Prerequisites

#### Mac OS

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

#### Linux

Although we strive to be fully POSIX-compliant, we recommend using Ubuntu 18.04 and at least GCC 7.5.0 for development.

You will need to install the following packages:
    ```
    sudo apt install cmake libacl1-dev libncurses5-dev pkg-config
    ```

Additionally, there is an optional dependency to the MIT licensed cpptoml library, which is used to parse a RouDi config file for the mempool config.
[cpptoml](https://github.com/skystrife/cpptoml)

### Build with CMake (all supported platforms)

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

#### With the following CMake switches you can add additional features

 |  switch  |  description |
 |:---------|:-------------|
 | `dds_gateway` | builds the iceoryx dds gateway using the cyclonedds dds stack, cyclonedds will be fetched and built as part of the build, see [cyclonedds](https://github.com/eclipse-cyclonedds/cyclonedds) for details |
 | `examples` | builds all examples |
 | `introspection` | the console introspection client which requires an installed ncurses library with terminfo support |
 | `test` | enables module-, integration- and component-tests |
 | `TOML_CONFIG` | activates config file support by using toml, if this is deactivated the central broker `RouDi` is not being build |

#### With the following CMake switches you can customize the iceoryx_posh build

 |  switch  |  description |
 |:---------|:-------------|
 | `IOX_MAX_PORT_NUMBER` | the maximum number of publisher and subscriber ports `RouDi` can distribute to the clients |
 | `IOX_MAX_INTERFACE_NUMBER` | the maximum number for interface ports, which are used for e.g. gateways |
 | `IOX_MAX_SUBSCRIBERS_PER_PUBLISHER` | the maximum number of subscriber a publisher can deliver chunks |
 | `IOX_MAX_CHUNKS_ALLOCATE_PER_SENDER` | the maximum number of chunks a sender can hold at a given time |
 | `IOX_MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR` | the maximum number chunks available for the chunk history |
 | `IOX_MAX_CHUNKS_HELD_PER_RECEIVER` | the maximum number of chunks a receiver can hold at a given time |

Have a look at `iceoryx_posh/cmake/iceoryx_posh_deployment.cmake` for the default values of this constants.

### Build with the build script (only Linux and QNX)

As an alternative we provide our build-test script which we use to integrate iceoryx into our infrastructure.

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

### Build with colcon

Alternatively, iceoryx can be built with [colcon](https://colcon.readthedocs.io/en/released/user/installation.html) to provide a smooth integration for ROS2 developers.

```
mkdir -p iceoryx_ws/src
cd $_
git clone https://github.com/eclipse/iceoryx.git
cd ..
colcon build
```

This build method makes the most sense in combination with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git)

Congrats! You've build all the necessary things to continue playing around with the examples.

### Examples

You can find our examples [here](./iceoryx_examples).

### Build and run in a Docker environment

If you want to avoid installing anything on your host machine but you have Docker installed, it is possible to use it to build and run iceoryx applications.

Please see the dedicated [README.md](tools/docker/README.md) for information on how to do this.

## Documentation

* [Concepts](doc/conceptual-guide.md)
* [Usage Guide](doc/usage-guide.md)
* [Ice0ryx Utils Hacker Guide](iceoryx_utils/README.md)

### Targeted quality levels & platforms

> [Quality level](./CONTRIBUTING.md#quality-levels) are 5..1, where 1 is highest

|CMake project/target   | QNX  | Linux, Windows, MacOS | Comment                             |
|-----------------------|:----:|:---------------------:|:-----------------------------------:|
| example_icedelivery   | 5    | 5                     |                                     |
| example_iceperf       | 5    | 5                     |                                     |
| example_singleprocess | 5    | 5                     |                                     |
| iceoryx_dds           | 4    | 4                     |                                     |
| iceoryx_meta          | 5    | 5                     |                                     |
| iceoryx_posh          | 1, 2 | 4                     | Will be split into separate targets |
| iceoryx_utils         | 1    | 4                     |                                     |
| iceoryx_introspection | 5    | 5                     |                                     |

## Innovations enabled by iceoryx

|Name | Description | Technologies |
|---|---|---|
| [Larry.Robotics](https://gitlab.com/larry.robotics) | An iceoryx demonstrator for tinker, thinker and toddler | Demonstrator |
| [iceoryx-rs](https://github.com/elBoberido/iceoryx-rs) | Experimental Rust wrapper for iceoryx | Rust |
| [IceRay](https://github.com/elBoberido/iceray) | An iceoryx instrospection client written in Rust | Rust |

Is something missing or you've got ideas for other nifty examples? Jump right away to the next section!

## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) for a quick read-up about what to consider if you want to contribute.

### Planned features

Get to know the upcoming features and the project scope in [PLANNED_FEATURES.md](./PLANNED_FEATURES.md).

## Maintainers

* Michael Pöhnl (michael.poehnl@de.bosch.com)
* Christian Eltzschig (christian.eltzschig2@de.bosch.com)
* Dietrich Krönke (dietrich.kroenke2@de.bosch.com)
* Mathias Kraus (mathias.kraus2@de.bosch.com)
* Matthias Killat (matthias.killat2@de.bosch.com)
* Martin Hintz (martin.hintz@de.bosch.com)
* Simon Hoinkis (simon.hoinkis2@de.bosch.com)
