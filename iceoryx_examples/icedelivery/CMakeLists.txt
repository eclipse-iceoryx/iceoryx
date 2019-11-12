# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(icedelivery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

add_executable(ice_sender ./ice_sender.cpp)
target_link_libraries(ice_sender
    iceoryx_posh::iceoryx_posh
)

add_executable(ice_receiver ./ice_receiver.cpp)
target_link_libraries(ice_receiver
    iceoryx_posh::iceoryx_posh
)

add_executable(ice_sender_simple ./ice_sender_simple.cpp)
target_link_libraries(ice_sender_simple
    iceoryx_posh::iceoryx_posh
)

add_executable(ice_receiver_simple ./ice_receiver_simple.cpp)
target_link_libraries(ice_receiver_simple
    iceoryx_posh::iceoryx_posh
)

set_target_properties(ice_sender ice_receiver ice_sender_simple ice_receiver_simple
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/icedelivery")

install(
    TARGETS ice_sender ice_receiver ice_sender_simple ice_receiver_simple
    RUNTIME DESTINATION bin
)
