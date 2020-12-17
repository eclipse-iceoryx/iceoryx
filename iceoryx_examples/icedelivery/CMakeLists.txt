# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(example_icedelivery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatform)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iox-ex-publisher-typed ./iox_publisher_typed.cpp)
target_link_libraries(iox-ex-publisher-typed
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-publisher-typed PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-ex-publisher-untyped ./iox_publisher_untyped.cpp)
target_link_libraries(iox-ex-publisher-untyped
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-publisher-untyped PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-ex-subscriber-typed ./iox_subscriber_typed.cpp)
target_link_libraries(iox-ex-subscriber-typed
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-subscriber-typed PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-ex-subscriber-untyped ./iox_subscriber_untyped.cpp)
target_link_libraries(iox-ex-subscriber-untyped
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-subscriber-untyped PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

# ========================================================== //

install(TARGETS iox-ex-publisher-untyped iox-ex-publisher-typed
                iox-ex-subscriber-untyped iox-ex-subscriber-typed
        RUNTIME DESTINATION bin)
