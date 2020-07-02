cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx VERSION ${iceoryx_VERSION})

option(TOML_CONFIG "activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(test "Build tests" ON)
option(examples "build with iceoryx examples" ON)
option(introspection "builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(diagnostic "enables allmost all compiler warnings" OFF)
option(dds "builds the iceoryx dds gateway - enables internode communication via dds" OFF)

# ===== Dependencies
if(TOML_CONFIG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/cpptoml/prebuild)
endif(TOML_CONFIG)

if(test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/dependencies/googletest/prebuild)
endif(test)

# ===== Core
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/posh)

# ===== Extras
if(introspection)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/introspection ${CMAKE_BINARY_DIR}/iceoryx_introspection)
endif(introspection)

if(examples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/iceoryx_examples/iceperf)
endif(examples)

# ===== Gateways
if(cyclonedds)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cyclonedds ${CMAKE_BINARY_DIR}/dependencies/cyclonedds/prebuild)
endif(cyclonedds)

if(dds_gateway)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_dds ${CMAKE_BINARY_DIR}/dds_gateway)
endif(dds_gateway)
