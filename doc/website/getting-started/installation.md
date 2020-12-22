# Installation
1. [Prerequisites](#prerequisites)
2. [Building with CMake](#build-with-cmake)
3. [Building with the build script](#build-with-the-build-script)
4. [Building with colcon](#build-with-colcon)
5. [Build and run tests](#build-and-run-tests)

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
sudo apt install gcc g++ cmake libacl1-dev libncurses5-dev pkg-config
```

Additionally, there is an optional dependency to the MIT licensed cpptoml library, which is used to parse a RouDi config file for the mempool config.
[cpptoml](https://github.com/skystrife/cpptoml)

## Build with CMake

**NOTE:** Requires CMake version 3.5 or higher. Building from CMake is the preferred way, for more complex actions like a coverage scan
is a script available (see chapter below).

The `CMakeLists.txt` from `iceoryx_meta` can be used to easily develop iceoryx with an IDE.

 1. Clone the repository
    ```bash
    git clone https://github.com/eclipse/iceoryx.git
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
    git clone https://github.com/eclipse/iceoryx.git
    ```

 2. Build everything
    ```
    cd iceoryx
    ./tools/iceoryx_build_test.sh build_all
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
git clone https://github.com/eclipse/iceoryx.git
cd ..
colcon build
```

This build method makes the most sense in combination with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git)

## Build and run tests

While developing on iceoryx you want to know if your changes are breaking existing functions or if your newly written googletests are passing.
For that purpose we are generating cmake targets which are executing the tests. But first we need to build them:
```
cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON
cmake --build build
```
Cmake is automatically installing googletest as dependency and build the tests against it. Please note that if you want to build tests for extensions like the C-Binding you need to enable that in the cmake build. To build all tests simply add `-DBUILD_ALL` to the cmake command

Now lets execute the all tests:
```
cd iceoryx/build
make all_tests
```
Some of the tests are timing critical and needs a stable environment. We call them timing tests and have them in a separate target available:
```
make timing_tests
```
In iceoryx we distinguish between different testlevels. The most important are: Moduletests and Integrationtests.
Moduletests are basically Unit-tests where the focus is on class level with black-box testing.
In Integrationtests are multiple classes within one component (e.g. iceoryx_posh) tested together.
The sourcecode of the tests is placed into the folder `test` within the different iceoryx components. You can find there at least a folder `moduletests` and sometimes ``integrationtests`.

when you now want to create a new test you can place the sourcefile directly into the right folder. Cmake will automatically detect the new file when doing a clean build and will add it to a executable. There is no need to add a gtest main function because we already provide it.
For every test level are executables created, for example `posh_moduletests`. They are placed into the corresponding build folder (e.g. `iceoryx/build/posh/test/posh_moduletests`).

If you want to execute only individual testcases then you can use these executables and a gtest filter. Let's assume you want to execute only the `ServiceDescription_test` from the posh_moduletests, then you can do the following:
```
./build/posh/test/posh_moduletests --gtest_filter="ServiceDescription_test*"
```

## Use Sanitizer Scan

Due to the fact that iceoryx works a lot with system memory it should be ensured that errors like memory leaks are not introduced.
To prevent these, we use the clang toolchain which offers several tools for scanning the codebase. One of them is the Adress-Sanitizer which checks for example on dangling pointers: [https://clang.llvm.org/docs/AddressSanitizer.html](https://clang.llvm.org/docs/AddressSanitizer.html)

In iceoryx are scripts available to do the scan on your own. Additionally the Scans are running on the CI in every Pull-Request.
As Requirement you should install the clang compiler: 
```
sudo apt install clang
```

Then you need to compile the iceoryx with the sanitizer flags:
```
./tools/iceoryx_build_test.sh build-strict build-all sanitize clang clean
```
After that we can run the tests with enabled sanitizer options:
```
cd build
../tools/run_all_tests.sh
```
When the tests are running without errors then it is fine, else an error report is shown with a stacktrace to find the place where the leak occurs. If the leak comes from an external dependency or shall be handled later then it is possible to set a function on a suppression list.
This should be only rarely used and only in coordination with an iceoryx maintainer.
