
# Gateway to DDS Networks
A gateway for bridging between iceoryx systems and DDS networks.
The gateway enables iceoryx systems running on separate hosts to communicate with each other.

i.e. Data published by a publisher on `Host A` can be received by a matching subscriber on `Host B`.

# Organization
This module exports the following executables:
* `iox-gw-iceoryx2dds`
* `iox-gw-dds2iceoryx`

Each executable manages a POSH runtime that runs the gateway logic for a single direction of communication.

The common building blocks logic for these binaries are consolidated in the exported library, `libiceoryx_dds`.

Applications may instead directly embed the gateway by using the exported lib.

# Building
The DDS stack used by the gateway is abstracted and needs to made explicit at compile time. 

## Pre-requisites
* Java is installed
* Maven is installed
* CMake is installed

```bash
sudo apt install cmake maven openjdk-14-jdk-headless
```
## Scripted Build
The easiest way to build the gateway is via the script `iceoryx/tools/iceoryx_build_test.sh`.

To build, simply run:
```bash
iceoryx/tools/iceoryx_build_test.sh dds-gateway
```

You may want to specify the build directory, this can be done via a flag. e.g.
```bash
iceoryx/tools/iceoryx_build_test.sh --build-dir ./my-build dds-gateway
```

Once complete, the gateway binaries can be found in `./my-build/install/prefix/bin`.

## CMake Build
Alternatively, you may like to manually run the build via CMake. This option is useful especially during development.

First, all of the gateway dependencies must be fetched, built, and installed into a common location.  The install location shall be referred to as `$INSTALL_DIR` from hereon.

This can be done manually, in which case the following dependencies must be installed to `$INSTALL_DIR`:
* cpptoml
* gtest (if testing)
* cyclonedds
* cyclonedds-cxx
* idlpp-cxx
* iceoryx_utils
* iceoryx_posh

A potentially easier method is, again, to take advantage of the script `iceoryx/tools/iceoryx_build_test.sh`.

Through building the gateway once via the script, all dependencies will be automatically fetched and installed.
They will be be found in, for example,  `./my-build/install/prefix`.

Then, `iceoryx_dds` can be built via CMake like so:
```
INSTALL_DIR=./my-build/install/prefix
mkdir -p ./my-build/dds_gateway
cd ./my-build/dds_gateway
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ../../
cmake --build . -j8
cmake --build . --target install
```
# Running
Before running, you may need to add the install directory to the library load path if it is not standard (so that the runtime dependencies can be found).
i.e.
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR/lib
```

Then, simply run the gateway executables as desired.

e.g.
```bash
$INSTALL_DIR/bin/iox-gw-iceoryx2dds
$INSTALL_DIR/bin/iox-gw-dds2iceoryx
```
