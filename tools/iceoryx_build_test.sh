#!/bin/bash

# Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script builds iceoryx_utils und iceoryx_posh and executes all tests

set -e

#====================================================================================================
#==== Step 0 : Setup ================================================================================
#====================================================================================================

# The absolute path of the directory assigned to the build
WORKSPACE=$(git rev-parse --show-toplevel)
BUILD_DIR=$WORKSPACE/build

CLEAN_BUILD=false
BUILD_TYPE=""
STRICT_FLAG="off"
TEST_FLAG="off"
DOWNLOAD_GTEST=true
DOWNLOAD_CPPTOML=true
DOWNLOAD_CYCLONEDDS=true

while (( "$#" )); do
  case "$1" in
    -b|--builddir)
        BUILD_DIR=$(realpath $2)
        shift 2
        ;;
    "clean")
        CLEAN_BUILD=true
        shift 1
        ;;
    "release")
        BUILD_TYPE="Release"
        shift 1
        ;;
    "debug")
        BUILD_TYPE="Debug"
        shift 1
        ;;
    "test")
        BUILD_TEST=true
        TEST_FLAG="on"
        shift 1
        ;;
    "no-gtest-download")
        DOWNLOAD_GTEST=false
        shift 1
        ;;
    "no-cpptoml-download")
        DOWNLOAD_CPPTOML=false
        shift 1
        ;;
    "no-cyclonedds-download")
        DOWNLOAD_CYCLONEDDS=false
        shift 1
        ;;
    "help")
        echo "Build script for iceoryx."
        echo "By default, iceoryx and the examples are build."
        echo ""
        echo "Usage: iceoryx_build_test.sh [options]"
        echo "Options:"
        echo "    clean                     Cleans the build directory"
        echo "    release                   Build release configuration"
        echo "    debug                     Build debug configuration"
        echo "    test                      Builds and runs the tests"
        echo "    no-gtest-download         Gtest will not be downloaded, but searched in the system"
        echo "                              Be careful, there might be problems due to incompatible versions"
        echo "    no-cpptoml-download       Cpptoml will not be downloaded, but searched in the system"
        echo "                              Be careful, there might be problems due to incompatible versions"
        echo "    no-cyclonedds-download    CycloneDDS will not be downloaded, but searched in the system"
        echo "                              Be careful, there might be problems due to incompatible versions"
        echo "    help                      Prints this help"
        echo ""
        echo "e.g. iceoryx_build_test.sh clean test release"
        exit 0
        ;;
    *)
        echo "Invalid argument '$arg'. Try 'help' for options."
        exit -1
        ;;
  esac
done

# define directories dependent on the build directory
ICEORYX_INSTALL_PREFIX=$BUILD_DIR/install/prefix/
DEPENDENCIES_INSTALL_PREFIX=$BUILD_DIR/dependencies/

echo " [i] Building in $BUILD_DIR"

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# detect number of course if possible
NUM_CORES=1
if nproc >/dev/null 2>&1; then
    NUM_CORES=`nproc`
fi

# # clean build folder
# if [ $CLEAN_BUILD == true ]
# then
#     echo " [i] Cleaning build directory"
#     cd $WORKSPACE
#     rm -rf $BUILD_DIR/*
# fi

# # create a new build directory and change the current working directory
# echo " [i] Create a new build directory and change the current working directory"
# cd $WORKSPACE
# mkdir -p $BUILD_DIR
# cd $BUILD_DIR

# echo " [i] Current working directory: $(pwd)"

# # Download and install googletest
# if [[ $TEST_FLAG == "on" && $DOWNLOAD_GTEST == true ]]
# then
#     cd $BUILD_DIR
#     mkdir -p googletest
#     cd googletest

#     echo ">>>>>> Start building googletest dependency <<<<<<"
#     cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$DEPENDENCIES_INSTALL_PREFIX -Dtest=$TEST_FLAG $WORKSPACE/cmake/googletest
#     echo ">>>>>> finished building googletest dependency <<<<<<"
# fi

# # Download and install cpptoml
# if [ $DOWNLOAD_CPPTOML == true ]
# then
#     cd $BUILD_DIR
#     mkdir -p cpptoml
#     cd cpptoml

#     echo ">>>>>> Start building cpptoml dependency <<<<<<"
#     cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$DEPENDENCIES_INSTALL_PREFIX $WORKSPACE/cmake/cpptoml
#     echo ">>>>>> finished building cpptoml dependency <<<<<<"
# fi

# # Download and install cyclonedds
# if [ $DOWNLOAD_CYCLONEDDS == true ]
# then
#     cd $BUILD_DIR
#     mkdir -p cyclonedds
#     cd cyclonedds

#     echo ">>>>>> Start building cyclonedds dependency <<<<<<"
#     cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$DEPENDENCIES_INSTALL_PREFIX $WORKSPACE/cmake/cyclonedds
#     echo ">>>>>> finished building cyclonedds dependency <<<<<<"
# fi

# # Build iceoryx_utils
# cd $BUILD_DIR
# mkdir -p utils
# cd utils

# echo ">>>>>> Start building iceoryx utils package <<<<<<"
# cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$DEPENDENCIES_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -Dtest=$TEST_FLAG $WORKSPACE/iceoryx_utils
# cmake --build . -j$NUM_CORES --target install
# echo ">>>>>> finished building iceoryx utils package <<<<<<"

# # Build iceoryx_posh
# cd $BUILD_DIR
# mkdir -p posh
# cd posh

# echo ">>>>>> Start building iceoryx posh package <<<<<<"
# cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$DEPENDENCIES_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DTOML_CONFIG=on -Dtest=$TEST_FLAG -Droudi_environment=on $WORKSPACE/iceoryx_posh
# cmake --build . -j$NUM_CORES --target install
# echo ">>>>>> finished building iceoryx posh package <<<<<<"

# Build iceoryx_dds
cd $BUILD_DIR
mkdir -p dds
cd dds

echo ">>>>>> Start building iceoryx dds package <<<<<<"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$DEPENDENCIES_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DBUILD_TESTS=$TEST_FLAG $WORKSPACE/iceoryx_dds_gateway
cmake --build . -j$NUM_CORES --target install
echo ">>>>>> finished building iceoryx dds package <<<<<<"

# Build iceoryx_introspection
cd $BUILD_DIR
mkdir -p iceoryx_introspection
cd iceoryx_introspection

echo ">>>>>> Start building iceoryx introspection <<<<<<"
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -Dtest=$TEST_FLAG -Droudi_environment=on $WORKSPACE/tools/introspection
cmake --build . -j$NUM_CORES --target install
echo ">>>>>> finished building iceoryx introspection package <<<<<<"

echo ">>>>>> Start building iceoryx examples <<<<<<"
cd $BUILD_DIR
mkdir -p iceoryx_examples
echo ">>>>>>>> icedelivery"
cd $BUILD_DIR/iceoryx_examples
mkdir -p icedelivery
cd icedelivery
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/icedelivery
cmake --build . -j$NUM_CORES
echo ">>>>>>>> iceperf"
cd $BUILD_DIR/iceoryx_examples
mkdir -p iceperf
cd iceperf
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/iceperf
cmake --build . -j$NUM_CORES
echo ">>>>>> finished building iceoryx examples <<<<<<"

#====================================================================================================
#==== Step 2 : Run all Tests  =======================================================================
#====================================================================================================

if [ $TEST_FLAG == "on" ]
then

# The absolute path of the directory assigned to the build
cd $BUILD_DIR

# change the current working directory
mkdir -p tools
cp $WORKSPACE/tools/run_all_tests.sh $WORKSPACE/build/tools/run_all_tests.sh

echo " [i] Run all Tests:"
# call runAllTest shell script to run all tests for Iceoryx
$WORKSPACE/build/tools/run_all_tests.sh

for folder in $component_folder; do

    if [ ! -f testresults/"$folder"_ModuleTestResults.xml ]; then
        echo "xml:"$folder"_ModuletestTestResults.xml not found!"
        exit 1
    fi

    if [ ! -f testresults/"$folder"_ComponenttestTestResults.xml ]; then
        echo "xml:"$folder"_ComponenttestTestResults.xml not found!"
        exit 1
    fi

    if [ ! -f testresults/"$folder"_IntegrationTestResults.xml ]; then
        echo "xml:"$folder"_IntegrationTestResults.xml not found!"
        exit 1
    fi

done

fi
