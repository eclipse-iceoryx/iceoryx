# This script builds iceoryx_utils und iceoryx_posh and executes all tests

set -e

#====================================================================================================
#==== Step 0 : Setup ================================================================================
#====================================================================================================

# The absolute path of the directory assigned to the build
WORKSPACE=$(git rev-parse --show-toplevel)
BUILD_DIR=$WORKSPACE/build

ICEORYX_INSTALL_PREFIX=$WORKSPACE/build/install/prefix/

CLEAN_BUILD=false
BUILD_TYPE=""
STRICT_FLAG="off"
TEST_FLAG="off"
RUN_TEST=false
INTROSPECTION_FLAG="on"

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
        "strict")
            STRICT_FLAG="on"
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
            INTROSPECTION_FLAG="off"
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
            echo "    strict                Build is performed with '-Werror'"
            echo "    test                  Builds and runs the tests"
            echo "    build-test            Builds the tests (doesn't tun)"
            echo "    skip-introspection    Skips building iceoryx introspection"
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

echo " [i] Building in $BUILD_DIR"

#====================================================================================================
#==== Step 1 : Build  ===============================================================================
#====================================================================================================

# Clean build folder
if [ $CLEAN_BUILD == true ]
then
    echo " [i] Cleaning build directory"
    cd $WORKSPACE
    rm -rf build/*
    rm -rf $BUILD_DIR/*
fi

# create a new build directory and change the current working directory
echo " [i] Create a new build directory and change the current working directory"
cd $WORKSPACE
mkdir -p build
cd build
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo " [i] Current working directory:"
pwd
echo " [i] Current working directory: $(pwd)"

echo ">>>>>> Start building iceoryx package <<<<<<"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_STRICT=$STRICT_FLAG -DCMAKE_INSTALL_PREFIX=$ICEORYX_INSTALL_PREFIX -DTOML_CONFIG=on -Dtest=$TEST_FLAG -Droudi_environment=on -Dexamples=OFF -Dintrospection=$INTROSPECTION_FLAG $WORKSPACE/iceoryx_meta
cmake --build . --target install
echo ">>>>>> finished building iceoryx package <<<<<<"

echo ">>>>>> Start building iceoryx examples <<<<<<"
cd $WORKSPACE/build
cd $BUILD_DIR
mkdir -p iceoryx_examples
echo ">>>>>>>> icedelivery"
cd $WORKSPACE/build/iceoryx_examples
cd $BUILD_DIR/iceoryx_examples
mkdir -p icedelivery
cd icedelivery
cmake -DCMAKE_PREFIX_PATH=$ICEORYX_INSTALL_PREFIX $WORKSPACE/iceoryx_examples/icedelivery
cmake --build .
echo ">>>>>>>> iceperf"
cd $WORKSPACE/build/iceoryx_examples
cd $BUILD_DIR/iceoryx_examples
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
cd $BUILD_DIR

# change the current working directory
mkdir -p tools
cp $WORKSPACE/tools/run_all_tests.sh $WORKSPACE/build/tools/run_all_tests.sh

echo " [i] Run all Tests:"
# call runAllTest shell script to run all tests for Iceoryx
$WORKSPACE/build/tools/run_all_tests.sh

for folder in $component_folder; do