cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_utils_VERSION)
project(iceoryx_utils VERSION ${iceoryx_utils_VERSION})

include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxPackageHelper.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxPlatformDetection.cmake")

if( clang-tidy )
    find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
    )

    if(CLANG_TIDY_EXE)
        set(PERFORM_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else(CLANG_TIDY_EXE)
        message(WARNING "clang-tidy activated but unable to find clang-tidy executable")
    endif(CLANG_TIDY_EXE)
endif( clang-tidy )

########## find_package in source tree ##########
#
set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake
    CACHE FILEPATH
    "${PROJECT_NAME}Config.cmake to make find_package(${PROJECT_NAME}) work in source tree!"
    FORCE
)

#
########## set variables for library export ##########
#


#
########## set util internal target, needed by tests in components ##########
#
# TODO: we need something like internal public for internal build dependencies
# instead of making the whole source folder public for internal dependency resolution

if(test AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(test AND NOT GTest_FOUND)

if(GTest_FOUND) # only GTest_FOUND, just in case someone want's to use iceoryx_utils_testing without also building the tests

    setup_package_name_and_create_files(
        NAME ${PROJECT_NAME}_testing
        NAMESPACE iceoryx_utils_testing
        PROJECT_PREFIX ${PREFIX}
    )

    add_library(iceoryx_utils_testing
        testutils/mocks/mqueue_mock.cpp
        testutils/mocks/time_mock.cpp
        testutils/timing_test.cpp
        testutils/compile_test.cpp
    )

    add_library(iceoryx_utils_testing::iceoryx_utils_testing ALIAS iceoryx_utils_testing)

    set_target_properties(iceoryx_utils_testing PROPERTIES VERSION ${PROJECT_VERSION})
    if(PERFORM_CLANG_TIDY)
        set_target_properties(
            iceoryx_utils_testing PROPERTIES CXX_CLANG_TIDY "${PERFORM_CLANG_TIDY}"
        )
    endif(PERFORM_CLANG_TIDY)
    set_target_properties(iceoryx_utils_testing PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_STANDARD ${ICEORYX_CXX_STANDARD}
        POSITION_INDEPENDENT_CODE ON
    )

    if(test_with_additional_user)
        target_compile_definitions(iceoryx_utils_testing PUBLIC -DTEST_WITH_ADDITIONAL_USER)
    endif(test_with_additional_user)

    target_include_directories(iceoryx_utils_testing
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/testutils>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/testutils/mocks>
        $<INSTALL_INTERFACE:include/${PREFIX}>
    )

    target_link_libraries(iceoryx_utils_testing PRIVATE
        ${CODE_COVERAGE_LIBS}
        iceoryx_utils
        GTest::gtest
        GTest::gmock
        ${CMAKE_DL_LIBS}
    )

    target_compile_options(iceoryx_utils_testing PRIVATE ${ICEORYX_WARNINGS})

    if(LINUX)
        target_link_libraries(iceoryx_utils_testing PRIVATE rt)
    endif()

    # install_target_directories_and_header(
    # TARGETS iceoryx_utils_testing
    # INCLUDE_DIRECTORY testutils/
    # )

    setup_install_directories_and_export_package(
    TARGETS iceoryx_utils_testing
    INCLUDE_DIRECTORY testutils/
    )

    #
    ########## find_package in source tree ##########
    #
    set(${PROJECT_NAME}_testing_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake
    CACHE FILEPATH
    "${PROJECT_NAME}_testingConfig.cmake to make find_package(${PROJECT_NAME}_testing) work in source tree!"
    FORCE
    )

endif(GTest_FOUND)

setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_utils
    PROJECT_PREFIX ${PREFIX}
)

#
########## build iceoryx util lib ##########
#

add_library(iceoryx_utils
    source/concurrent/active_object.cpp
    source/concurrent/loffli.cpp
    source/concurrent/locked_loffli.cpp
    source/cxx/helplets.cpp
    source/cxx/generic_raii.cpp
    source/error_handling/error_handling.cpp
    source/file_reader/file_reader.cpp
    source/log/logcommon.cpp
    source/log/logger.cpp
    source/log/logging.cpp
    source/log/logging_internal.cpp
    source/log/logmanager.cpp
    source/log/logstream.cpp
    source/log/ac3log_shim/simplelogger.cpp
    source/posix_wrapper/access_control.cpp
    source/posix_wrapper/mutex.cpp
    source/posix_wrapper/semaphore.cpp
    source/posix_wrapper/timer.cpp
    source/posix_wrapper/timespec.cpp
    source/posix_wrapper/shared_memory_object.cpp
    source/posix_wrapper/message_queue.cpp
    source/posix_wrapper/unix_domain_socket.cpp
    source/posix_wrapper/shared_memory_object/allocator.cpp
    source/posix_wrapper/shared_memory_object/memory_map.cpp
    source/posix_wrapper/shared_memory_object/shared_memory.cpp
    source/posix_wrapper/system_configuration.cpp
    source/posix_wrapper/posix_access_rights.cpp
    source/units/duration.cpp
    source/relocatable_pointer/relative_ptr.cpp
)

add_library(iceoryx_utils::iceoryx_utils ALIAS iceoryx_utils)
set_target_properties(iceoryx_utils PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if(LINUX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/platform/linux/)
    target_link_libraries(iceoryx_utils PRIVATE rt acl atomic ${CODE_COVERAGE_LIBS})
elseif(QNX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/platform/qnx/)
elseif(APPLE)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/platform/mac/)
elseif(WIN32)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/platform/win/)
else()
    message(WARNING "Could not detect supported platform, but I'm feeling lucky today." )
endif()

# this must be recurse since every platform has potentially different cpp files
file ( GLOB_RECURSE ICEORYX_PLATFORM_FILES
    ${ICEORYX_PLATFORM}/source/*.cpp
)
add_library(iceoryx_platform ${ICEORYX_PLATFORM_FILES})
target_include_directories(iceoryx_platform
    PUBLIC
    $<BUILD_INTERFACE:${ICEORYX_PLATFORM}/include/>
    $<INSTALL_INTERFACE:include/${PREFIX}>
)
add_library(iceoryx_utils::iceoryx_platform ALIAS iceoryx_platform)
set_target_properties(iceoryx_platform PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

target_compile_options(iceoryx_platform PRIVATE ${ICEORYX_WARNINGS})

if(PERFORM_CLANG_TIDY)
    set_target_properties(
        iceoryx_utils PROPERTIES CXX_CLANG_TIDY "${PERFORM_CLANG_TIDY}"
    )
endif(PERFORM_CLANG_TIDY)


target_link_libraries(iceoryx_utils PUBLIC iceoryx_utils::iceoryx_platform)

target_compile_options(iceoryx_utils PRIVATE ${ICEORYX_WARNINGS})

# TODO: Make ICEORYX::UTILS private???
target_include_directories(iceoryx_utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
)



#
########## exporting library ##########
#

#TODO add support for multiple include directories to setup_install_directories_and_export_package
install(
    DIRECTORY ${ICEORYX_PLATFORM}/include/
    DESTINATION include/${PREFIX}
    COMPONENT dev
)
setup_install_directories_and_export_package(
    TARGETS iceoryx_utils iceoryx_platform
    INCLUDE_DIRECTORY include/
)

# header
install(
    FILES
        cmake/IceoryxPackageHelper.cmake
        cmake/IceoryxPlatformDetection.cmake
    DESTINATION ${DESTINATION_CONFIGDIR}
)

if(test)
    add_subdirectory(test)
endif(test)
