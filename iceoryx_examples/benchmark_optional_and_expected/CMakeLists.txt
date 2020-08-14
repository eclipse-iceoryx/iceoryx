# Build icedelivery example
cmake_minimum_required(VERSION 3.5)
project(benchmark_optional_and_expected)

include(GNUInstallDirs)

find_package(iceoryx_utils CONFIG REQUIRED)
find_package(Threads REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_utils::iceoryx_utils CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iox-bm-optional-and-expected ./benchmark_optional_and_expected.cpp)
target_link_libraries(iox-bm-optional-and-expected
    iceoryx_utils::iceoryx_utils
    Threads::Threads
)
set_target_properties(iox-bm-optional-and-expected PROPERTIES 
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS iox-bm-optional-and-expected
    RUNTIME DESTINATION bin
)
