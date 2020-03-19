cmake_minimum_required(VERSION 3.5)
set(iceoryx_posh_VERSION 0.16.1)
project(iceoryx_posh VERSION ${iceoryx_posh_VERSION})

find_package(Threads REQUIRED)
find_package(iceoryx_utils REQUIRED)

if(TOML_CONFIG)
    find_package(cpptoml REQUIRED)
    message("-- Build with TOML config file support.")
endif(TOML_CONFIG)

include(cmake/iceoryxversions.cmake)
include(cmake/install_help_and_config.cmake)
include(IceoryxPackageHelper)
include(IceoryxPlatformDetection)

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

#
########## set variables for library export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_posh
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

#
########## build posh lib for ##########
#
add_library(iceoryx_posh
    source/log/posh_logging.cpp
    source/capro/capro_message.cpp
    source/capro/service_description.cpp
    source/mepoo/chunk_header.cpp
    source/mepoo/mepoo_config.cpp
    source/mepoo/segment_config.cpp
    source/mepoo/memory_manager.cpp
    source/mepoo/mem_pool.cpp
    source/mepoo/shared_chunk.cpp
    source/mepoo/segment_manager.cpp
    source/mepoo/mepoo_segment.cpp
    source/popo/application_port.cpp
    source/popo/application_port_data.cpp
    source/popo/base_port_data.cpp
    source/popo/base_port.cpp
    source/popo/delivery_fifo.cpp
    source/popo/publisher.cpp
    source/popo/subscriber.cpp
    source/popo/gateway_generic.cpp
    source/popo/interface_port.cpp
    source/popo/interface_port_data.cpp
    source/popo/receiver_port.cpp
    source/popo/receiver_port_data.cpp
    source/popo/sender_port.cpp
    source/popo/sender_port_data.cpp
    source/popo/receiver_handler.cpp
    source/runtime/message_queue_interface.cpp
    source/runtime/message_queue_message.cpp
    source/runtime/posh_runtime.cpp
    source/runtime/runnable.cpp
    source/runtime/runnable_data.cpp
    source/runtime/runnable_property.cpp
    source/runtime/service_discovery_notifier.cpp
    source/runtime/shared_memory_user.cpp
)

add_library(${PROJECT_NAMESPACE}::iceoryx_posh ALIAS iceoryx_posh)

target_compile_options(iceoryx_posh PUBLIC -std=c++11)
target_compile_options(iceoryx_posh PUBLIC -fPIC)


set_target_properties(iceoryx_posh
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
if(PERFORM_CLANG_TIDY)
    set_target_properties(
        iceoryx_posh PROPERTIES CXX_CLANG_TIDY "${PERFORM_CLANG_TIDY}"
    )
endif(PERFORM_CLANG_TIDY)

target_include_directories(iceoryx_posh
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated/iceoryx/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
    PRIVATE
    source/capro
    source/log
    source/mepoo
    source/popo
    source/runtime
)
target_link_libraries(iceoryx_posh
    PUBLIC
    iceoryx_utils::iceoryx_utils
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
)

if(LINUX)
    target_link_libraries(iceoryx_posh PRIVATE rt )
elseif(QNX)
elseif(APPLE)
elseif(WIN32)
else()
    message(WARNING "Could not detect supported platform, but I'm feeling lucky today." )
endif()



#
########## build roudi lib ##########
#
add_library(iceoryx_posh_roudi
    source/roudi/application/iceoryx_roudi_app.cpp
    source/roudi/application/roudi_app.cpp
    source/roudi/memory/memory_block.cpp
    source/roudi/memory/memory_provider.cpp
    source/roudi/memory/posix_shm_memory_provider.cpp
    source/roudi/roudi_config.cpp
    source/roudi/roudi_cmd_line_parser.cpp
    source/roudi/roudi_cmd_line_parser_config_file_option.cpp
    source/roudi/roudi_multi_process.cpp
    source/roudi/roudi_lock.cpp
    source/roudi/roudi_process.cpp
    source/roudi/shared_memory_manager.cpp
    source/roudi/service_registry.cpp
)

add_library(${PROJECT_NAMESPACE}::iceoryx_posh_roudi ALIAS iceoryx_posh_roudi)
set_target_properties(iceoryx_posh_roudi PROPERTIES OUTPUT_NAME "iceoryx_posh_roudi")
if(PERFORM_CLANG_TIDY)
    set_target_properties(
        iceoryx_posh_roudi PROPERTIES CXX_CLANG_TIDY "${PERFORM_CLANG_TIDY}"
    )
endif(PERFORM_CLANG_TIDY)

target_compile_options(iceoryx_posh_roudi PUBLIC -std=c++11)
target_compile_options(iceoryx_posh_roudi PUBLIC -fPIC)

target_include_directories(iceoryx_posh_roudi PRIVATE
    source/runtime
    source/popo
    source/capro
    source/roudi
)

set_target_properties(iceoryx_posh_roudi
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(iceoryx_posh_roudi
    PUBLIC
    ${PROJECT_NAMESPACE}::iceoryx_posh     # because of roudi.hpp
    PRIVATE
    iceoryx_utils::iceoryx_utils
)

if(TOML_CONFIG)
    add_library(iceoryx_posh_roudi_toml
        source/roudi/roudi_config_toml_file_provider.cpp
    )
    target_link_libraries(iceoryx_posh_roudi_toml PRIVATE cpptoml iceoryx_posh_roudi)
endif(TOML_CONFIG)

if(CMAKE_SYSTEM_NAME MATCHES QNX)
    target_link_libraries(iceoryx_posh_roudi PRIVATE socket)
endif(CMAKE_SYSTEM_NAME MATCHES QNX)

#
########## build roudi ##########
if(TOML_CONFIG)

    add_executable(RouDi
        source/roudi/application/roudi_main.cpp
    )

    target_compile_options(RouDi PUBLIC -std=c++11)
    target_compile_options(RouDi PUBLIC -fPIC)

    set_target_properties(RouDi
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    if(PERFORM_CLANG_TIDY)
        set_target_properties(
            RouDi PROPERTIES CXX_CLANG_TIDY "${PERFORM_CLANG_TIDY}"
        )
    endif(PERFORM_CLANG_TIDY)
    target_link_libraries(RouDi
        PRIVATE
        ${PROJECT_NAMESPACE}::iceoryx_posh_roudi
        iceoryx_utils::iceoryx_utils
    )

    target_link_libraries(RouDi PRIVATE cpptoml iceoryx_posh_roudi_toml)

endif(TOML_CONFIG)

if(roudi_environment OR test)
    add_library(iceoryx_posh_roudi_environment
        source/roudi_environment/runtime_test_interface.cpp
        source/roudi_environment/roudi_environment.cpp
    )

    add_library(${PROJECT_NAMESPACE}::iceoryx_posh_roudi_environment ALIAS iceoryx_posh_roudi_environment)

    target_compile_options(iceoryx_posh_roudi_environment PUBLIC -std=c++11)
    target_compile_options(iceoryx_posh_roudi_environment PUBLIC -fPIC)

    target_include_directories(iceoryx_posh_roudi_environment PUBLIC
        source/roudi_environment
    )

    target_link_libraries(iceoryx_posh_roudi_environment
        PUBLIC
        ${PROJECT_NAMESPACE}::iceoryx_posh
        PRIVATE
        iceoryx_utils::iceoryx_utils
        ${PROJECT_NAMESPACE}::iceoryx_posh_roudi
    )
endif(roudi_environment OR test)

#
########## exporting library ##########
#
setup_install_directories_and_export_package(
    TARGETS iceoryx_posh iceoryx_posh_roudi
    INCLUDE_DIRECTORY include/
)
if(TOML_CONFIG)
    setup_install_directories_and_export_package(
        TARGETS RouDi
        INCLUDE_DIRECTORY include/
    )
endif(TOML_CONFIG)

if(test)
    add_subdirectory(test)
endif(test)
