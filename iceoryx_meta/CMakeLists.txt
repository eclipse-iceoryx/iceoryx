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
endif()   
endif(test)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/iceoryx_utils )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/iceoryx_posh )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/examples/icedelivery )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/examples/iceperf )
