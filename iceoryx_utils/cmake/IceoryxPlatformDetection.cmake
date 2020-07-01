if(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(LINUX true)
    elseif(CMAKE_SYSTEM_NAME MATCHES QNX)
        set(QNX true)
    endif()
endif(UNIX AND NOT APPLE)

if(LINUX)
    set(ICEORYX_CXX_STANDARD 11)
elseif(QNX)
    set(ICEORYX_CXX_STANDARD 11)
elseif(WIN32)
    set(ICEORYX_CXX_STANDARD 11)
elseif(APPLE)
    set(ICEORYX_CXX_STANDARD 17)
endif(LINUX)

set (ICEORYX_WARNINGS -W -Wall -Wextra -Wconversion -Wuninitialized -Wpedantic -Wstrict-aliasing -Wcast-align -Wno-noexcept-type)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # temporary disable conversion warnings for clang
    # this needs to be fixed with iox-#163
    set(ICEORYX_WARNINGS ${ICEORYX_WARNINGS} -Wno-conversion)
endif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

if(BUILD_STRICT)
    set(ICEORYX_WARNINGS ${ICEORYX_WARNINGS} -Werror)
endif(BUILD_STRICT)
