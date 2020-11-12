# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(example_icedelivery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iox-ex-publisher-typed-modern ./iox_publisher_typed_modern.cpp)
target_link_libraries(iox-ex-publisher-typed-modern
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-publisher-typed-modern PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-ex-publisher-untyped-modern ./iox_publisher_untyped_modern.cpp)
target_link_libraries(iox-ex-publisher-untyped-modern
    iceoryx_posh::iceoryx_posh
)
set_target_properties(iox-ex-publisher-untyped-modern PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS iox-ex-publisher-untyped-modern iox-ex-publisher-typed-modern
    RUNTIME DESTINATION bin
)
