
# Gateway to DDS Networks
A gateway for bridging between iceoryx systems and DDS networks.

## **Note**: Currently only the iceoryx->dds direction is implemented. The opposite direction is coming soon!

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

Then, simply run the gateway executable.
e.g.
```bash
$INSTALL_DIR/bin/gateway_iox2dds
```


# Internals

The Gateway is split across two applications, one for for each direction of communication.

In the iceoryx->dds direction, `gateway_iox2dds`, the gateway translates iceoryx topics to DDS topics using a known convention and forwards all data received to the DDS network on these topics. 

In the dds->iceoryx direction, `gateway_dds2iox`, iceoryx topics available in the DDS network are identified and the gateway forwards all data received on these topics to local iceoryx subscribers. 

The common building block logic for these applications are consolidated in a library, `libioxdds`.