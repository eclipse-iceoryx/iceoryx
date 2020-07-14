cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx VERSION ${iceoryx_VERSION})

option(TOML_CONFIG "activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(test "Build tests" ON)
option(examples "build with iceoryx examples" ON)
option(introspection "builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(dds "enables dds support by compiling the iceoryx dds gateway" OFF)

if(TOML_CONFIG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/cpptoml/prebuild)
endif(TOML_CONFIG)

if(test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/dependencies/googletest/prebuild)
endif(test)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/posh)

if(introspection)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/introspection ${CMAKE_BINARY_DIR}/iceoryx_introspection)
endif(introspection)

if(examples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/iceoryx_examples/iceperf)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/singleprocess ${CMAKE_BINARY_DIR}/iceoryx_examples/singleprocess)
endif(examples)

if(dds)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_dds ${CMAKE_BINARY_DIR}/iceoryx_dds)
endif(dds)


message("")
message("       Configured Options")
message("          TOML_CONFIG...............: " ${TOML_CONFIG})
message("          BUILD_STRICT..............: " ${BUILD_STRICT})
message("          test......................: " ${test})
message("          examples..................: " ${examples})
message("          introspection.............: " ${introspection})
message("          dds.......................: " ${dds})

message("")
message("       Build Properties")
message("          project name..............: " ${CMAKE_PROJECT_NAME})
message("          c compiler................: " ${CMAKE_C_COMPILER})
message("          c++ compiler..............: " ${CMAKE_CXX_COMPILER})
message("")

