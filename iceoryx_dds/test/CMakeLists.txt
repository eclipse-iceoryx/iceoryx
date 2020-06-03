cmake_minimum_required(VERSION 3.5)
project(test_iox_to_dds VERSION ${test_iox_to_dds})

find_package(iceoryx_utils_testing REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(PROJECT_PREFIX "ioxdds")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_utils::iceoryx_utils
    iceoryx_utils_testing::iceoryx_utils_testing
    iceoryx_posh::iceoryx_posh
    iceoryx_dds_gateway::ioxdds
)

if(NOT CMAKE_SYSTEM_NAME MATCHES QNX)
    set(TEST_LINK_LIBS ${TEST_LINK_LIBS} dl)
endif(NOT CMAKE_SYSTEM_NAME MATCHES QNX)

# unittests
add_executable(${PROJECT_PREFIX}_moduletests ${MODULETESTS_SRC})
set_property(TARGET ${PROJECT_PREFIX}_moduletests PROPERTY CXX_STANDARD 11)
target_include_directories(${PROJECT_PREFIX}_moduletests PRIVATE .)
target_compile_options(${PROJECT_PREFIX}_moduletests PUBLIC -fPIC PRIVATE -Wno-conversion)
target_link_libraries(${PROJECT_PREFIX}_moduletests ${TEST_LINK_LIBS})
