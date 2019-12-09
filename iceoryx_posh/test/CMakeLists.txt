cmake_minimum_required(VERSION 3.5)
project(test_iceoryx_posh VERSION ${test_iceoryx_posh})

find_package(iceoryx_utils_testing REQUIRED)

if(test AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(test AND NOT GTest_FOUND)

set(PROJECT_PREFIX "posh")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")
file(GLOB_RECURSE COMPONENTTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/componenttests/*.cpp")
file(GLOB_RECURSE TESTUTILS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/testutils/*.cpp")
file(GLOB_RECURSE MOCKS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/mocks/*.cpp")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_utils::iceoryx_utils
    iceoryx_utils_testing::iceoryx_utils_testing
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh::iceoryx_posh_roudi_environment
)

if(NOT CMAKE_SYSTEM_NAME MATCHES QNX)
    set(TEST_LINK_LIBS ${TEST_LINK_LIBS} dl)
endif(NOT CMAKE_SYSTEM_NAME MATCHES QNX)

# unittests
add_executable(${PROJECT_PREFIX}_moduletests ${MODULETESTS_SRC} ${TESTUTILS_SRC} ${MOCKS_SRC})
target_include_directories(${PROJECT_PREFIX}_moduletests PRIVATE .)
# TODO: fix conversion warnings
target_compile_options(${PROJECT_PREFIX}_moduletests PUBLIC -std=c++11 -fPIC PRIVATE -Wno-pedantic -Wno-conversion) # -Wpedantic results in errors at the time mocks

target_link_libraries(${PROJECT_PREFIX}_moduletests ${TEST_LINK_LIBS})

# integration tests
add_executable( ${PROJECT_PREFIX}_integrationtests ${INTEGRATIONTESTS_SRC} ${TESTUTILS_SRC} ${MOCKS_SRC})
target_include_directories(${PROJECT_PREFIX}_integrationtests PUBLIC -std=c++11 -fPIC PRIVATE .)
target_link_libraries(${PROJECT_PREFIX}_integrationtests ${TEST_LINK_LIBS})

# component tests
add_executable( ${PROJECT_PREFIX}_componenttests ${COMPONENTTESTS_SRC} ${TESTUTILS_SRC} ${MOCKS_SRC})
target_compile_options(${PROJECT_PREFIX}_componenttests PUBLIC -std=c++11 -fPIC PRIVATE -Wno-pedantic -Wno-conversion) # -Wpedantic results in errors at the time mocks
target_include_directories(${PROJECT_PREFIX}_componenttests PRIVATE .)
target_link_libraries(${PROJECT_PREFIX}_componenttests ${TEST_LINK_LIBS})
