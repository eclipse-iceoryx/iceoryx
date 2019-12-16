# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(icedelivery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

add_executable(ice-publisher-bare-metal ./ice_publisher_bare_metal.cpp)
target_link_libraries(ice-publisher-bare-metal
    iceoryx_posh::iceoryx_posh
)

add_executable(ice-subscriber-bare-metal ./ice_subscriber_bare_metal.cpp)
target_link_libraries(ice-subscriber-bare-metal
    iceoryx_posh::iceoryx_posh
)

add_executable(ice-publisher-simple ./ice_publisher_simple.cpp)
target_link_libraries(ice-publisher-simple
    iceoryx_posh::iceoryx_posh
)

add_executable(ice-subscriber-simple ./ice_subscriber_simple.cpp)
target_link_libraries(ice-subscriber-simple
    iceoryx_posh::iceoryx_posh
)

install(
    TARGETS ice-publisher-bare-metal ice-subscriber-bare-metal ice-publisher-simple ice-subscriber-simple
    RUNTIME DESTINATION bin
)