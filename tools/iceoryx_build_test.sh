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

# List of possible values for BUILD_ARGS : "buildtest", "notest", "clean"
BUILD_ARGS=${1:-no_test_incremental}

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# The absolute path of the directory assigned to the build
WORKSPACE=$(git rev-parse --show-toplevel)
test_flag="-Dtest=off"
download_gtest_flag="-Ddownload_gtest=off"
cd $WORKSPACE

# Clean build folder
if [ $BUILD_ARGS = "clean" ]
then
    rm -rf build
elif [ $BUILD_ARGS = "test" ]
then
    test_flag="-Dtest=on"
fi

# create a new build directory and change the current working directory
echo " [i] Create a new build directory and change the current working directory"
mkdir -p build
cd build

echo " [i] Current working directory:"
pwd

# Download and build googletest

if [ $BUILD_ARGS = "test" ]
then
    test_flag="-Dtest=on"
    download_gtest_flag="-Ddownload_gtest=on"
    cmake -DCMAKE_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/ $test_flag $download_gtest_flag $WORKSPACE/cmake/googletest
    cmake --build . --target install
fi


# Build iceoryx_utils
mkdir -p utils
cd utils

echo ">>>>>> Start building iceoryx utils package <<<<<<"
cmake -DCMAKE_PREFIX_PATH=$WORKSPACE/install/prefix -DCMAKE_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/ $test_flag $WORKSPACE/iceoryx_utils
cmake --build . --target install
cd $WORKSPACE/build
echo ">>>>>> finished building iceoryx utils package <<<<<<"

# Build iceoryx_posh
mkdir -p posh
cd posh

echo ">>>>>> Start building iceoryx posh and utils <<<<<<"
cmake -DCMAKE_PREFIX_PATH=$WORKSPACE/build/install/prefix -DCMAKE_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/ $test_flag -Droudi_environment=on $WORKSPACE/iceoryx_posh
cmake --build . --target install
cd $WORKSPACE/build
echo ">>>>>> finished building iceoryx posh package <<<<<<"

# Build iceoryx_introspection
mkdir -p iceoryx_introspection
cd iceoryx_introspection

echo ">>>>>> Start building iceoryx introspection <<<<<<"
cmake -DCMAKE_PREFIX_PATH=$WORKSPACE/build/install/prefix -DCMAKE_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/ $test_flag -Droudi_environment=on $WORKSPACE/tools/introspection
cmake --build . --target install
cd $WORKSPACE/build
echo ">>>>>> finished building iceoryx introspection package <<<<<<"

echo ">>>>>> Start building iceoryx examples <<<<<<"
mkdir -p iceoryx_examples
echo ">>>>>>>> icedelivery"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p icedelivery
cd icedelivery
cmake -DCMAKE_PREFIX_PATH=$WORKSPACE/build/install/prefix $WORKSPACE/iceoryx_examples/icedelivery
cmake --build .
echo ">>>>>>>> iceperf"
cd $WORKSPACE/build/iceoryx_examples
mkdir -p iceperf
cd iceperf
cmake -DCMAKE_PREFIX_PATH=$WORKSPACE/build/install/prefix $WORKSPACE/iceoryx_examples/iceperf
cmake --build .
echo ">>>>>> finished building iceoryx examples <<<<<<"

#====================================================================================================
#==== Step 2 : Run all Tests  =======================================================================
#====================================================================================================

if [ $BUILD_ARGS = "test" ]
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


