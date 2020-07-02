cmake_minimum_required(VERSION 3.5)
project(googletest-build CXX)

if(test)
    
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0 AND ((${CMAKE_VERSION} VERSION_GREATER "3.12.0") OR ${CMAKE_VERSION} VERSION_EQUAL "3.12.0"))
      set(CMAKE_BUILD_FLAGS -j${N})
    endif()

    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(CREATE_PATH_COMMAND mkdir)
    else()
        set(CREATE_PATH_COMMAND mkdir -p)
    endif(WIN32)

    # set download confi, source and build paths
    set(DOWNLOAD_CONFIG_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/download)
    set(SOURCE_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/src)
    set(BUILD_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/build)
    set(INSTALL_DIR ${CMAKE_BINARY_DIR}/dependencies/install)

    # Download and unpack googletest at configure time
    configure_file(googletest.cmake.in ${DOWNLOAD_CONFIG_DIR}/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "${DOWNLOAD_CONFIG_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure download] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build . ${CMAKE_BUILD_FLAGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [download] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CREATE_PATH_COMMAND} "${BUILD_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [create build dir] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}" "${SOURCE_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install ${CMAKE_BUILD_FLAGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [build and install] for googletest failed: ${result}")
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${INSTALL_DIR})
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE INTERNAL "" FORCE)

endif(test)
