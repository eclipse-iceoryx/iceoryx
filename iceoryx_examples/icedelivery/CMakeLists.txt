# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(icedelivery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(ice-publisher-bare-metal ./ice_publisher_bare_metal.cpp)
target_link_libraries(ice-publisher-bare-metal
    iceoryx_posh::iceoryx_posh
)
set_target_properties(ice-publisher-bare-metal PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(ice-subscriber-bare-metal ./ice_subscriber_bare_metal.cpp)
target_link_libraries(ice-subscriber-bare-metal
    iceoryx_posh::iceoryx_posh
)
set_target_properties(ice-subscriber-bare-metal PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(ice-publisher-simple ./ice_publisher_simple.cpp)
target_link_libraries(ice-publisher-simple
    iceoryx_posh::iceoryx_posh
)
set_target_properties(ice-publisher-simple PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(ice-subscriber-simple ./ice_subscriber_simple.cpp)
target_link_libraries(ice-subscriber-simple
    iceoryx_posh::iceoryx_posh
)
set_target_properties(ice-subscriber-simple PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS ice-publisher-bare-metal ice-subscriber-bare-metal ice-publisher-simple ice-subscriber-simple
    RUNTIME DESTINATION bin
)
