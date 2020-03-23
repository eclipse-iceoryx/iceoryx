#!/bin/bash

# ========== Parse Arguments ========== #
if [ -z $2 ];
then
    INSTALL_DIR=$(pwd)/install
else
    INSTALL_DIR=$(realpath $2)
fi
if [ -z $1 ];
then
    BUILD_DIR=$(pwd)/build
else
    BUILD_DIR=$(realpath $1)
fi

echo ""
echo "Building DDS Gateway"
echo ""
echo "    BUILD_DIR=$BUILD_DIR"
echo "    INSTALL_DIR=$INSTALL_DIR"
echo ""

sleep 2

# ========== Prepare Workspace ========== #
mkdir -p $BUILD_DIR
mkdir -p $INSTALL_DIR
cd $BUILD_DIR

# ========== Build Dependencies ========== #
# Build CycloneDDS
git clone https://github.com/eclipse-cyclonedds/cyclonedds.git 
mkdir ./cyclonedds/build
cd ./cyclonedds/build
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ..
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

# Build IDL Compiler
git clone https://github.com/ADLINK-IST/idlpp-cxx.git
mkdir ./idlpp-cxx/build
cd ./idlpp-cxx/build
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" .. 
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

# Build CycloneDDS C++ API
git clone https://github.com/ThijsSassen/cdds-cxx.git
mkdir ./cdds-cxx/build
cd ./cdds-cxx/build
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DBUILD_TESTING=OFF ..
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

# iceoryx
git clone https://github.com/eclipse/iceoryx.git 

# iceoryx_utils
mkdir -p ./iceoryx/build/iceoryx_utils
cd ./iceoryx/build/iceoryx_utils
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ../../iceoryx_utils
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

#iceoryx_posh
mkdir -p ./iceoryx/build/iceoryx_posh
cd ./iceoryx/build/iceoryx_posh
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ../../iceoryx_posh
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

# ========== Build Gateway ========== #
mkdir -p ./iceoryx/build/iceoryx_dds_gateway
cd ./iceoryx/build/iceoryx_dds_gateway
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ../../iceoryx_dds_gateway
cmake --build . -j8
cmake --build . --target install
cd $BUILD_DIR

