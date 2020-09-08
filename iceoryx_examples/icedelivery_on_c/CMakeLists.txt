# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(example_icedelivery_on_c)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iox-c-publisher ./ice_c_publisher.c)
set_source_files_properties(./ice_c_publisher.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-publisher
    iceoryx_posh::iceoryx_posh
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-publisher PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-c-subscriber ./ice_c_subscriber.c)
set_source_files_properties(./ice_c_subscriber.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-subscriber
    iceoryx_posh::iceoryx_posh
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-subscriber PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS iox-c-publisher iox-c-subscriber
    RUNTIME DESTINATION bin
)
