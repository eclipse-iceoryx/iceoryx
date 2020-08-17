# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(example_singleprocess)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(single_process ./single_process.cpp)
target_link_libraries(single_process
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_roudi
)
set_target_properties(single_process PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS single_process
    RUNTIME DESTINATION bin
)
