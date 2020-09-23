cmake_minimum_required(VERSION 3.5)
set(test_iceoryx_utils_VERSION 0)
project(test_iceoryx_utils VERSION ${test_iceoryx_utils_VERSION})

find_package(Threads REQUIRED)

if(test AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(test AND NOT GTest_FOUND)

set(PROJECT_PREFIX "utils")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")
file(GLOB_RECURSE COMPONENTTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/componenttests/*.cpp")
file(GLOB_RECURSE TESTUTILS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/testutils/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_utils::iceoryx_utils
    iceoryx_utils_testing::iceoryx_utils_testing
)

if(LINUX)
    set(TEST_LINK_LIBS ${TEST_LINK_LIBS} acl dl pthread rt)
    set(TEST_CXX_FLAGS PRIVATE ${ICEORYX_WARNINGS} -Wno-pedantic -Wno-conversion)  # -Wpedantic results in errors at the time mocks
elseif(QNX)
    set(TEST_CXX_FLAGS PRIVATE ${ICEORYX_WARNINGS} -Wno-pedantic -Wno-conversion)
elseif(APPLE)
    set(TEST_CXX_FLAGS PRIVATE ${ICEORYX_WARNINGS} -Wno-pedantic -Wno-conversion)
elseif(WIN32)
    set(TEST_CXX_FLAGS PRIVATE /bigobj)
endif()


# set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR}/test)

# unittests
add_executable(${PROJECT_PREFIX}_moduletests ${MODULETESTS_SRC} ${TESTUTILS_SRC})
target_include_directories(${PROJECT_PREFIX}_moduletests PRIVATE .)
# TODO: fix conversion warnings
target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${TEST_CXX_FLAGS})
target_link_libraries(${PROJECT_PREFIX}_moduletests ${TEST_LINK_LIBS})
set_target_properties(${PROJECT_PREFIX}_moduletests PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

# integration tests
add_executable( ${PROJECT_PREFIX}_integrationtests ${INTEGRATIONTESTS_SRC} ${TESTUTILS_SRC})
target_compile_options(${PROJECT_PREFIX}_integrationtests PRIVATE ${TEST_CXX_FLAGS})
target_include_directories(${PROJECT_PREFIX}_integrationtests PRIVATE .)
target_link_libraries(${PROJECT_PREFIX}_integrationtests ${TEST_LINK_LIBS})
set_target_properties(${PROJECT_PREFIX}_integrationtests PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

# component tests
add_executable( ${PROJECT_PREFIX}_componenttests ${COMPONENTTESTS_SRC} ${TESTUTILS_SRC})
target_compile_options(${PROJECT_PREFIX}_componenttests PRIVATE ${TEST_CXX_FLAGS})
target_include_directories(${PROJECT_PREFIX}_componenttests PRIVATE .)
target_link_libraries(${PROJECT_PREFIX}_componenttests ${TEST_LINK_LIBS})
set_target_properties(${PROJECT_PREFIX}_componenttests PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)
