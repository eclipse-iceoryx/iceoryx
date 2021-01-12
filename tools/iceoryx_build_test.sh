#!/bin/bash

# Copyright (c) 2019-2020, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

# the absolute path of the directory assigned to the build
WORKSPACE=$(git rev-parse --show-toplevel)
BUILD_DIR=$WORKSPACE/build
NUM_JOBS=""
PACKAGE="OFF"
CLEAN_BUILD=false
BUILD_TYPE=""
BUILD_DOC="OFF"
STRICT_FLAG="OFF"
TEST_FLAG="OFF"
COV_FLAG="OFF"
TEST_SCOPE="all" #possible values for test scope: 'all', 'unit', 'integration'
RUN_TEST=false
DDS_GATEWAY_FLAG="OFF"
BINDING_C_FLAG="ON"
ONE_TO_MANY_ONLY_FLAG="OFF"
SANITIZE_FLAG="OFF"
ROUDI_ENV_FLAG="OFF"
OUT_OF_TREE_FLAG="OFF"
EXAMPLE_FLAG="OFF"
BUILD_ALL_FLAG="OFF"
BUILD_SHARED="ON"
TOML_FLAG="ON"
EXAMPLES="ice_multi_publisher icedelivery singleprocess waitset" 
COMPONENTS="iceoryx_posh iceoryx_utils iceoryx_introspection iceoryx_binding_c iceoryx_component iceoryx_dds" 

while (( "$#" )); do
  case "$1" in
    -b|--build-dir)
        BUILD_DIR=$(realpath $2)
        shift 2
        ;;
    -c|--coverage)
        echo "$2"
        TEST_SCOPE="$2"
        BUILD_TYPE="Debug"
        RUN_TEST=true
        COV_FLAG="ON"
        if [ -z "$2" ]
        then
            shift 1
        else
            shift 2
        fi
        ;;
    "clean")
        CLEAN_BUILD=true
        shift 1
        ;;
    "release")
        BUILD_TYPE="Release"
        shift 1
        ;;
    "relwithdebinfo")
        BUILD_TYPE="RelWithDebInfo"
        shift 1
        ;;
    "debug")
        BUILD_TYPE="Debug"
        shift 1
        ;;
    "build-strict")
        STRICT_FLAG="ON"
        shift 1
        ;;
    "test")
        RUN_TEST=true
        TEST_FLAG="ON"
        shift 1
        ;;
    "dds-gateway")
        echo " [i] Including DDS gateway in build"
        DDS_GATEWAY_FLAG="ON"
        shift 1
        ;;
    "binding-c")
        echo " [i] Including DDS gateway in build"
        BINDING_C_FLAG="ON"
        shift 1
        ;;
    "build-test")
        echo " [i] Building tests"
        TEST_FLAG="ON"
        shift 1
        ;;
    "package")
        PACKAGE="ON"
        shift 1
        ;;
    "roudi-env")
        echo " [i] Building RouDi Environment"
        ROUDI_ENV_FLAG="ON"
        shift 1
        ;;
    "build-all")
        echo " [i] Build complete iceoryx with all extensions and all examples"
        BUILD_ALL_FLAG="ON"
        shift 1
        ;;
    "build-static")
        echo " [i] Build iceoryx as static lib"
        BUILD_SHARED="OFF"
        shift 1
        ;;
    "examples")
        echo " [i] Build iceoryx with all examples"
        EXAMPLE_FLAG="ON"
        shift 1
        ;;
    "out-of-tree")
        echo " [i] Out-of-tree build"
        OUT_OF_TREE_FLAG="ON"
        shift 1
        ;;
    "one-to-many-only")
        echo " [i] Using 1:n communication only"
        ONE_TO_MANY_ONLY_FLAG="ON"
        shift 1
        ;;
    "toml-config-off")
        echo " [i] Build without TOML Support"
        TOML_FLAG="OFF"
        shift 1
        ;;
    "sanitize")
        echo " [i] Build with sanitizers"
        BUILD_TYPE="Debug"
        SANITIZE_FLAG="ON"
        shift 1
        ;;
    "clang")
        echo " [i] Build with clang compiler"
        export CC=$(which clang)
        export CXX=$(which clang++)
        shift 1
        ;;
    "doc")
        echo " [i] Build and generate doxygen"
        BUILD_DOC="ON"
        shift 1
        ;;
    "help")
        echo "Build script for iceoryx."
        echo "By default, iceoryx, the dds gateway and the examples are built."
        echo ""
        echo "Usage:"
        echo "    iceoryx_build_test.sh [--build-dir <dir>] [<args>]"
        echo "Options:"
        echo "    -b --build-dir         Specify a non-default build directory"
        echo "    -c --coverage         Builds gcov and generate a html/xml report. Possible arguments: 'all', 'unit', 'integration'"
        echo "Args:"
        echo "    clean                 Deletes the build/ directory before"
        echo "    release               Build with -O3"
        echo "    debug                 Build debug configuration -g"
        echo "    relwithdebinfo        Build with -O2 -DNDEBUG"
        echo "    examples              Build all examples"
        echo "    build-all             Build all extensions and all examples"
        echo "    out-of-tree           Out-of-tree build for CI build"
        echo "    build-strict          Build is performed with '-Werror'"
        echo "    build-static          Build static libs (iceoryx is build as shared lib per default)"
        echo "    build-test            Builds all tests (doesn't run)"
        echo "    package               Creates a debian package from clean build in build_package"
        echo "    test                  Builds and runs all tests in all iceoryx components"
        echo "    toml-config-off       Builds without TOML File support"
        echo "    dds-gateway           Builds the iceoryx dds gateway"
        echo "    binding-c             Builds the iceoryx C-Binding"
        echo "    one-to-many-only      Restricts to 1:n communication only"
        echo "    clang                 Build with clang compiler (should be installed already)"
        echo "    sanitize              Build with sanitizers"
        echo "    roudi-env             Build the roudi environment"
        echo "    help                  Prints this help"
        echo ""
        echo "e.g. iceoryx_build_test.sh -b ./build-scripted clean test"
        echo "for gcov report: iceoryx_build_test.sh clean -c unit"
        exit 0
        ;;
    *)
        echo "Invalid argument '$1'. Try 'help' for options."
        exit -1
        ;;
  esac
done

# define directories dependent on the build directory
ICEORYX_INSTALL_PREFIX=$BUILD_DIR/install/prefix/

echo " [i] Building in $BUILD_DIR"

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# set number of cores for building
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
    NUM_JOBS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    NUM_JOBS=$(sysctl -n hw.ncpu)
else
    NUM_JOBS=1
fi
echo " [i] Building with $NUM_JOBS jobs"


# clean build folders
if [ $CLEAN_BUILD == true ]
then
    echo " [i] Cleaning build directory"
    cd $WORKSPACE
    rm -rf $BUILD_DIR/*
fi

# create a new build directory and change the current working directory
echo " [i] Preparing build directory"
cd $WORKSPACE
mkdir -p $BUILD_DIR
cd $BUILD_DIR
echo " [i] Current working directory: $(pwd)"

if [ "$PACKAGE" == "OFF" ]; then
    echo ">>>>>> Start building iceoryx package <<<<<<"
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_ALL=$BUILD_ALL_FLAG -DBUILD_STRICT=$STRICT_FLAG -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX \
    -DBUILD_TEST=$TEST_FLAG -DCOVERAGE=$COV_FLAG -DROUDI_ENVIRONMENT=$ROUDI_ENV_FLAG -DEXAMPLES=$EXAMPLE_FLAG -DTOML_CONFIG=$TOML_FLAG -DBUILD_DOC=$BUILD_DOC \
    -DDDS_GATEWAY=$DDS_GATEWAY_FLAG -DBINDING_C=$BINDING_C_FLAG -DONE_TO_MANY_ONLY=$ONE_TO_MANY_ONLY_FLAG -DBUILD_SHARED_LIBS=$BUILD_SHARED -DSANITIZE=$SANITIZE_FLAG $WORKSPACE/iceoryx_meta

    cmake --build . --target install -- -j$NUM_JOBS
    echo ">>>>>> Finished building iceoryx package <<<<<<"
else
    echo ">>>>>> Start building iceoryx package <<<<<<"
    cd $WORKSPACE
    rm -rf build_package
    mkdir -p build_package
    cd build_package 

    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_STRICT=$STRICT_FLAG -DCMAKE_INSTALL_PREFIX=build_package/install/prefix/ $WORKSPACE/iceoryx_meta
    cmake --build . --target install -- -j$NUM_JOBS
    cpack
    echo ">>>>>> Finished building iceoryx package <<<<<<"    
fi


#====================================================================================================
#==== Step : Out-of-tree build  =====================================================================
#====================================================================================================

if [ "$OUT_OF_TREE_FLAG" == "ON" ]; then
    rm -rf $WORKSPACE/build_out_of_tree
    if [ "$BINDING_C_FLAG" == "ON" ]; then
        EXAMPLES="${EXAMPLES} icedelivery_on_c waitset_on_c iceperf"
    fi
    echo ">>>>>> Start Out-of-tree build <<<<<<"
    echo ${EXAMPLES}
    cd $WORKSPACE && mkdir -p build_out_of_tree && cd build_out_of_tree
        for ex in ${EXAMPLES}  ; do
            mkdir -p $ex && cd $ex
            cmake -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DTOML_CONFIG=$TOML_FLAG $WORKSPACE/iceoryx_examples/$ex
            cmake --build . --target install -- -j$NUM_JOBS
            if [ $? -ne 0 ]; then
                echo "Out of tree build failed"
                exit 1
            fi
            cd ..
        done
        echo ">>>>>> Finished Out-of-tree build<<<<<<"
fi

if [ "$COV_FLAG" == "ON" ]; then
    $WORKSPACE/tools/gcov/lcov_generate.sh $WORKSPACE initial #make an initial scan to cover also files with no coverage
fi

#====================================================================================================
#==== Step : Run all Tests  =========================================================================
#====================================================================================================
# the absolute path of the directory assigned to the build
cd $BUILD_DIR
mkdir -p tools
cp $WORKSPACE/tools/run_all_tests.sh $BUILD_DIR/tools/run_all_tests.sh

if [ $RUN_TEST == true ]; then
    echo " [i] Running all tests"
    $BUILD_DIR/tools/run_all_tests.sh $TEST_SCOPE
fi

for COMPONENT in $COMPONENTS; do

    case "$1" in
        "unit" | "all")
            if [ ! -f testresults/"$COMPONENT"_ModuleTestResults.xml ]; then
                echo "xml:"$COMPONENT"_ModuletestTestResults.xml not found!"
                exit 1
            fi
            ;;
        "integration" | "all")
            if [ ! -f testresults/"$COMPONENT"_IntegrationTestResults.xml ]; then
                echo "xml:"$COMPONENT"_IntegrationTestResults.xml not found!"
                exit 1
            fi
            ;;
    esac
done

#====================================================================================================
#==== Step : Coverage ===============================================================================
#====================================================================================================

if [ "$COV_FLAG" == "ON" ]; then
    echo ">>>>>> Generate Gcov Report <<<<<<"
    cd $WORKSPACE
    $WORKSPACE/tools/gcov/lcov_generate.sh $WORKSPACE capture #scan all files after test execution
    $WORKSPACE/tools/gcov/lcov_generate.sh $WORKSPACE combine #combine tracefiles from initial scan with the latest one
    $WORKSPACE/tools/gcov/lcov_generate.sh $WORKSPACE remove #exclude all unnecessary files
    $WORKSPACE/tools/gcov/lcov_generate.sh $WORKSPACE genhtml #generate html
    echo ">>>>>> Report Generation complete <<<<<<"
fi

#====================================================================================================
#==== Step : Doxygen PDF Generation =================================================================
#====================================================================================================


if [ "$BUILD_DOC" == "ON" ]; then
    echo ">>>>>> Doxygen PDF Generation <<<<<<"
    cd $BUILD_DIR

    for cmp in $COMPONENTS; do
        make doxygen_"$cmp"
        cd doc/"$cmp"/latex
        make
        cd ../../..
        mv -v doc/"$cmp"/latex/refman.pdf doc/"$cmp"_doxygen.pdf
    done
    echo ">>>>>> Doxygen PDF Generation complete <<<<<<"
fi
