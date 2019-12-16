# Build iceperf example
cmake_minimum_required(VERSION 3.5)
project(iceperf)

find_package(iceoryx_posh CONFIG REQUIRED)

add_executable(iceperf-laurel iceperf_laurel.cpp)
target_link_libraries(iceperf-laurel
    iceoryx_posh::iceoryx_posh
)

add_executable(iceperf-hardy iceperf_hardy.cpp)
target_link_libraries(iceperf-hardy
    iceoryx_posh::iceoryx_posh
)

install(
    TARGETS iceperf-laurel iceperf-hardy
    RUNTIME DESTINATION bin
)
