
# Gateway to DDS Networks
A gateway for bridging between iceoryx systems and DDS networks.
The gateway enables iceoryx systems running on separate hosts to communicate with eachother.

i.e. Data published by a publisher on `Host A` can be received by a matching subscriber on `Host B`.

# Organization
This module exports the following binaries:
* `gateway_iceoryx2dds`
* `gateway_dds2iceoryx`

Each binary manages a POSH runtime that runs the gateway logic for a single direction of communication.

The common building blocks logic for these applications are consolidated in the exported library, `libiceoryx_dds`.

Applications may directly embed the gateway by using the exported lib.

# Building
The DDS stack used by the gateway is abstracted and needs to made explicit at compile time.

## Building for Cyclone DDS
### Pre-requisites
* Java is installed
* Maven is installed
* CMake is installed

### Instructions
A script is provided to make the build process easy.
To build, simply run:
```bash
./scripts/build-for-cyclonedds.sh 
```

> **NOTE:** This script will soon be deprecated in favor of a pure CMake build

You may want to specify the build & install directories, this can be done like so:
```bash
BUILD_DIR=/path/to/build/dir
INSTALL_DIR=/path/to/install/dir

./scripts/build-for-cyclonedds.sh $BUILD_DIR $INSTALL_DIR
```

# Running
The build script will install the gateway binaries in the install directory.
Before running, however, you need to add the install directory to the library load path if it is not standard (so that the runtime dependencies can be found).
i.e.
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR/lib
```

Then, simply run the gateway executables as desired.

e.g.
```bash
$INSTALL_DIR/bin/gateway_iceoryx2dds
$INSTALL_DIR/bin/gateway_dds2iceoryx
```
