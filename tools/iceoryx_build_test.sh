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

ICEORYX_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/
DEPENDENCIES_INSTALL_PREFIX=$WORKSPACE/build/dependencies/

CLEAN_BUILD=false
BUILD_TYPE=""
TEST_FLAG="off"
RUN_TEST=false
BUILD_INTROSPECTION=true
DOWNLOAD_GTEST=true
DOWNLOAD_CPPTOML=true

for arg in "$@"
do
    case "$arg" in
        "clean")
            CLEAN_BUILD=true
            ;;
        "release")
            BUILD_TYPE="Release"
            ;;
        "debug")
            BUILD_TYPE="Debug"
            ;;
        "test")
            RUN_TEST=true
            TEST_FLAG="on"
            ;;
        "build-test")
            RUN_TEST=false
            TEST_FLAG="on"
            ;;            
        "skip-introspection")
            BUILD_INTROSPECTION=false
            ;;
        "no-gtest-download")
            DOWNLOAD_GTEST=false
            ;;
        "no-cpptoml-download")
            DOWNLOAD_CPPTOML=false
            ;;
        "help")
            echo "Build script for iceoryx."
            echo "By default, iceoryx and the examples are build."
            echo ""
            echo "Usage: iceoryx_build_test.sh [options]"
            echo "Options:"
            echo "    clean                 Cleans the build directory"
            echo "    release               Build release configuration"
            echo "    debug                 Build debug configuration"
            echo "    test                  Builds and runs the tests"
            echo "    build-test            Builds the tests (doesn't tun)"
            echo "    skip-introspection    Skips building iceoryx introspection"
            echo "    no-gtest-download     Gtest will not be downloaded, but searched in the system"
            echo "                          Be careful, there might be problems due to incompatible versions"
            echo "    no-cpptoml-download   Cpptoml will not be downloaded, but searched in the system"
            echo "                          Be careful, there might be problems due to incompatible versions"
            echo "    help                  Prints this help"
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

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# Clean build folder
if [ $CLEAN_BUILD == true ]
then
    echo " [i] Cleaning build directory"
    cd $WORKSPACE
    rm -rf build/*
fi

# create a new build directory and change the current working directory
echo " [i] Create a new build directory and change the current working directory"
cd $WORKSPACE
mkdir -p build
cd build

echo " [i] Current working directory:"
pwd

# Download and build googletest
if [[ $TEST_FLAG == "on" && $DOWNLOAD_GTEST == true ]]
then
    cd $WORKSPACE/build
    mkdir -p googletest
    cd googletest

    echo ">>>>>> Start building googletest dependency <<<<<<"
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$DEPENDENCIES_INSTALL_PREFIX -Dtest=$TEST_FLAG $WORKSPACE/cmake/googletest
    echo ">>>>>> finished building googletest dependency <<<<<<"
fi

# Build download and build cpptoml
if [ $DOWNLOAD_CPPTOML == true ]
then
    cd $WORKSPACE/build
    mkdir -p cpptoml
    cd cpptoml

    echo ">>>>>> Start building cpptoml dependency <<<<<<"
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$DEPENDENCIES_INSTALL_PREFIX $WORKSPACE/cmake/cpptoml
    echo ">>>>>> finished building googletest dependency <<<<<<"
fi

# Build iceoryx_utils
cd $WORKSPACE/build
mkdir -p utils
cd utils

echo ">>>>>> Start building iceoryx utils package <<<<<<"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$DEPENDENCIES_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -Dtest=$TEST_FLAG $WORKSPACE/iceoryx_utils
cmake --build . --target install
echo ">>>>>> finished building iceoryx utils package <<<<<<"

# Build iceoryx_posh
cd $WORKSPACE/build
mkdir -p posh
cd posh

echo ">>>>>> Start building iceoryx posh package <<<<<<"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$DEPENDENCIES_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DTOML_CONFIG=on -Dtest=$TEST_FLAG -Droudi_environment=on $WORKSPACE/iceoryx_posh
cmake --build . --target install
echo ">>>>>> finished building iceoryx posh package <<<<<<"

if [ $BUILD_INTROSPECTION == true ]
then
    # Build iceoryx_introspection
    cd $WORKSPACE/build
    mkdir -p iceoryx_introspection
    cd iceoryx_introspection

    echo ">>>>>> Start building iceoryx introspection <<<<<<"
    cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -Dtest=$TEST_FLAG -Droudi_environment=on $WORKSPACE/tools/introspection
    cmake --build . --target install
    echo ">>>>>> finished building iceoryx introspection package <<<<<<"
else
    echo ">>>>>> Skipping iceoryx introspection <<<<<<"
fi

echo ">>>>>> Start building iceoryx examples <<<<<<"
cd $WORKSPACE/build
mkdir -p iceoryx_examples
echo ">>>>>>>> icedelivery"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p icedelivery
cd icedelivery
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/icedelivery
cmake --build .
echo ">>>>>>>> iceperf"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p iceperf
cd iceperf
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/iceperf
cmake --build .
echo ">>>>>> finished building iceoryx examples <<<<<<"

#====================================================================================================
#==== Step 2 : Run all Tests  =======================================================================
#====================================================================================================

if [ $RUN_TEST == true ]
then

# The absolute path of the directory assigned to the build
cd $WORKSPACE/build

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
