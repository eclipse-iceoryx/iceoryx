cmake_minimum_required(VERSION 3.14)
set(iceoryx_VERSION 0.16.0.1)

project(iceoryx VERSION ${iceoryx_VERSION})

#uncomment this as soon the ExternalProject install path issue is solved
#if(test)
#    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/gtest)
#endif(test)


include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if(test)
FetchContent_Declare(
    GOOGLETEST
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.10.0
)
FetchContent_GetProperties(GOOGLETEST)
if (NOT googletest_POPULATED)
    message(STATUS "updating: googletest" )
    FetchContent_Populate(GOOGLETEST)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    set(GTest_FOUND true)
    add_library(GTest::gtest ALIAS gtest)
    add_library(GTest::gmock ALIAS gmock)
endif(NOT googletest_POPULATED)
endif(test)

# must be additionally defined here, otherwise TOML_CONFIG is on default off
# and the module is loaded only after the second cmake -Bbuild -Hiceoryx_meta command call
option(TOML_CONFIG "activates or deactivates TOML support - without TOML RouDi will not be build" ON)

if (TOML_CONFIG )
    FetchContent_Declare(
        CPPTOML
        GIT_REPOSITORY https://github.com/skystrife/cpptoml
        GIT_TAG v0.1.1
    )
    FetchContent_GetProperties(CPPTOML)
    if (NOT cpptoml_POPULATED)
        message(STATUS "updating: cpptoml" )
        FetchContent_Populate(CPPTOML)
        add_subdirectory(${cpptoml_SOURCE_DIR} ${cpptoml_BINARY_DIR})
        set(cpptoml_FOUND true)
    endif(NOT cpptoml_POPULATED)
endif(TOML_CONFIG)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/iceoryx_utils )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/iceoryx_posh )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/examples/icedelivery )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/examples/iceperf )
