cmake_minimum_required(VERSION 3.5)
set(iceoryx_introspection_VERSION 0.16.0)
project(iceoryx_introspection VERSION ${iceoryx_introspection_VERSION})

find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_utils REQUIRED)

include(IceoryxPackageHelper)

# cmake doesn't find ncurses under QNX; let the compiler complain if the header is missing
if(NOT CMAKE_SYSTEM_NAME MATCHES QNX)
    SET(CURSES_USE_NCURSES TRUE)
    find_package(Curses REQUIRED)
    if(${CURSES_HAVE_NCURSES_H} MATCHES ".*-NOTFOUND")
        message(SEND_ERROR "ncurses header not found! Please install libncurses5-dev!")
    endif(${CURSES_HAVE_NCURSES_H} MATCHES ".*-NOTFOUND")
endif()

#
########## set variables for library export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_introspection
    PROJECT_PREFIX ${PREFIX}
)

#
########## find_package in source tree ##########
#
set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake
    CACHE FILEPATH
    "${PROJECT_NAME}Config.cmake to make find_package(${PROJECT_NAME}) work in source tree!"
    FORCE
)

add_library(iceoryx_introspection
            source/iceoryx_introspection_app.cpp
            source/introspection_app.cpp
    )

add_library(${PROJECT_NAMESPACE}::iceoryx_introspection ALIAS iceoryx_introspection)

target_include_directories(iceoryx_introspection
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
    PRIVATE
    ${CURSES_INCLUDE_DIR}
)

target_link_libraries(iceoryx_introspection
    iceoryx_utils::iceoryx_utils
    iceoryx_posh::iceoryx_posh
    ncurses
)

set_target_properties(iceoryx_introspection
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_executable(iceoryx_introspection_client source/introspection_main.cpp)

target_link_libraries(iceoryx_introspection_client
    PRIVATE
    ${PROJECT_NAMESPACE}::iceoryx_introspection
)


#
########## exporting library ##########
#
setup_install_directories_and_export_package(
    TARGETS iceoryx_introspection
    INCLUDE_DIRECTORY include/
)