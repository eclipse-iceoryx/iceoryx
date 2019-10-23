# Building the examples

A warm welcome to iceoryx examples Readme! You can build all the example either by using the Debian package or
build things from scratch.

## Debian package

In case you've installed the iceoryx debian package CMake's `find_package()` will find the installed version. To build
the examples, do the following:

    ./iceexample#          mkdir build && cd build
    ./iceexample/build#    cmake ..
    ./iceexample/build#    cmake --build .

## Build everything from source

When building all the bits from source, build RouDi and the runtime as described in the main
[Readme.md](../README.md#user-content-development) with `./tools/iceoryx_build-test.sh`. Just add the path of the
`build` directory:

    ./iceexample#          mkdir build && cd build
    ./iceexample/build#    cmake .. -DCMAKE_INSTALL_PREFIX="`pwd`/../../../build/"
    ./iceexample/build#    cmake --build .
