cmake_minimum_required(VERSION 3.7)

set(iceoryx_dds_gateway_VERSION 0.1.0)
project(iceoryx_dds_gateway VERSION ${iceoryx_dds_gateway_VERSION})

option(BUILD_TESTS "build test binaries")

find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_utils REQUIRED)

include(IceoryxPackageHelper)

#
########## set variables for library export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_dds_gateway
    PROJECT_PREFIX ${PREFIX}
)

#
########## feature flags ##########
#
option(USE_CYCLONE_DDS "Bind to CycloneDDS implementation" on)

if(USE_CYCLONE_DDS)
    message(INFO " Using CycloneDDS stack")
    find_package(CycloneDDS CONFIG REQUIRED)
    find_package(CycloneDDS_CXX_API CONFIG REQUIRED)
    find_package(Idlpp-cxx CONFIG REQUIRED)
endif()

#
########## build building-block library ##########
#
add_library(ioxdds
    STATIC
    source/ioxdds/log/logging.cpp
)
add_library(${PROJECT_NAMESPACE}::ioxdds ALIAS ioxdds)

target_compile_options(ioxdds PUBLIC -std=c++11)

target_include_directories(ioxdds
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
)
target_link_libraries(ioxdds
    PRIVATE
    iceoryx_posh::iceoryx_posh
    iceoryx_utils::iceoryx_utils
)

if(USE_CYCLONE_DDS)
    target_sources(ioxdds
        PRIVATE
        source/ioxdds/dds/cyclone_data_writer.cpp
    )
    target_compile_definitions(ioxdds PUBLIC -DUSE_CYCLONE_DDS)
    idl_ddscxx_generate(mempoolMessages_lib "msg/Mempool.idl")
    target_link_libraries(ioxdds
        PUBLIC
        CycloneDDS_CXX_API::ddscxx
        mempoolMessages_lib
    )
endif()

#
########## build gateway apps ##########
#
add_executable(gateway_iox2dds
    source/iox2dds_app/main.cpp
)
target_link_libraries(gateway_iox2dds
    iceoryx_posh::iceoryx_posh
    ${PROJECT_NAMESPACE}::ioxdds
)

add_executable(gateway_dds2iox
    source/dds2iox_app/main.cpp
)
target_link_libraries(gateway_dds2iox
    iceoryx_posh::iceoryx_posh
    ${PROJECT_NAMESPACE}::ioxdds
)

#
########## build test executables ##########
#
if(BUILD_TESTS)
    add_subdirectory(test)
endif(BUILD_TESTS)

#
########## export library ##########
#
setup_install_directories_and_export_package(
    TARGETS gateway_iox2dds gateway_dds2iox
    INCLUDE_DIRECTORY include/
)

