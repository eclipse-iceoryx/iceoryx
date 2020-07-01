cmake_minimum_required(VERSION 3.5)
project(cyclonedds-build CXX)

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_FLAGS -j${N})
endif()

if(WIN32)
    set(CREATE_PATH_COMMAND mkdir)
else()
    set(CREATE_PATH_COMMAND mkdir -p)
endif(WIN32)

# ===== Helpers
function(fetch_and_install name)
    set(DOWNLOAD_CONFIG_DIR ${CMAKE_BINARY_DIR}/dependencies/${name}/download)
    set(SOURCE_DIR ${CMAKE_BINARY_DIR}/dependencies/${name}/src)
    set(BUILD_DIR ${CMAKE_BINARY_DIR}/dependencies/${name}/build)
    set(INSTALL_DIR ${CMAKE_BINARY_DIR}/dependencies/install)

    # Fetch source
    configure_file(${name}.cmake.in ${DOWNLOAD_CONFIG_DIR}/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "${DOWNLOAD_CONFIG_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure download] for ${name} failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [download] for ${name} failed: ${result}")
    endif()

    # Build
    execute_process(COMMAND ${CREATE_PATH_COMMAND} "${BUILD_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [create build dir] for ${name} failed: ${result}")
    endif()

    # Parse additional CMake flags
    set(ADDITIONAL_CMAKE_FLAGS "")
    foreach(flag IN LISTS ARGN)
        list(APPEND ADDITIONAL_CMAKE_FLAGS ${flag})
    endforeach()
    string( REPLACE ";" " " ADDITIONAL_CMAKE_FLAGS "${ADDITIONAL_CMAKE_FLAGS}")

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}" "${ADDITIONAL_CMAKE_FLAGS}" "${SOURCE_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure] for ${name} failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install ${CMAKE_BUILD_FLAGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [build and install] for ${name} failed: ${result}")
    endif()
endfunction()

fetch_and_install(idlpp-cxx)
fetch_and_install(cyclonedds)
fetch_and_install(cdds-cxx -DBUILD_TESTING=OFF)