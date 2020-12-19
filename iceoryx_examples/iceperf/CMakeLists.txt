# Build iceperf example
cmake_minimum_required(VERSION 3.5)
project(example_iceperf)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatform)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iceperf-laurel iceperf_laurel.cpp base.cpp iceoryx.cpp uds.cpp mq.cpp)
target_link_libraries(iceperf-laurel
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iceperf-laurel PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${OPTIMIZE_OPTIONS})

if(CMAKE_SYSTEM_NAME MATCHES QNX)
    target_link_libraries(iceperf-laurel socket)
endif(CMAKE_SYSTEM_NAME MATCHES QNX)

add_executable(iceperf-hardy iceperf_hardy.cpp base.cpp iceoryx.cpp uds.cpp mq.cpp)

set_target_properties(iceperf-laurel iceperf-hardy PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(iceperf-hardy
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iceperf-hardy PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${OPTIMIZE_OPTIONS})

if(CMAKE_SYSTEM_NAME MATCHES QNX)
    target_link_libraries(iceperf-hardy socket)
endif(CMAKE_SYSTEM_NAME MATCHES QNX)

install(
    TARGETS iceperf-laurel iceperf-hardy
    RUNTIME DESTINATION bin
)
