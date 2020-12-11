# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(LINUX true)
    elseif(CMAKE_SYSTEM_NAME MATCHES QNX)
        set(QNX true)
    endif()
endif(UNIX AND NOT APPLE)

if(LINUX)
    set(ICEORYX_CXX_STANDARD 14)
elseif(QNX)
    set(ICEORYX_CXX_STANDARD 14)
elseif(WIN32)
    set(ICEORYX_CXX_STANDARD 14)
elseif(APPLE)
    set(ICEORYX_CXX_STANDARD 17)
endif(LINUX)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(ICEORYX_WARNINGS PRIVATE ${ICEORYX_WARNINGS} /W1)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(ICEORYX_WARNINGS PRIVATE ${ICEORYX_WARNINGS} -W -Wall -Wextra -Wuninitialized -Wpedantic -Wstrict-aliasing -Wcast-align -Wno-noexcept-type -Wconversion)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(ICEORYX_WARNINGS PRIVATE ${ICEORYX_WARNINGS} -W -Wall -Wextra -Wuninitialized -Wpedantic -Wstrict-aliasing -Wcast-align -Wno-noexcept-type)
endif()

if(BUILD_STRICT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(ICEORYX_WARNINGS ${ICEORYX_WARNINGS} /WX)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(ICEORYX_WARNINGS ${ICEORYX_WARNINGS} -Werror)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(ICEORYX_WARNINGS ${ICEORYX_WARNINGS} -Werror)
    endif (  )
endif(BUILD_STRICT)

if(sanitize)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        # NOTE : This works only when iceoryx is built standalone , in which case CMAKE_SOURCE_DIR point to iceoryx_meta
        set(ICEORYX_SANITIZER_BLACKLIST -fsanitize-blacklist=${CMAKE_SOURCE_DIR}/sanitizer_blacklist/asan_compile_time.txt)
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(ICEORYX_SANITIZER_COMMON_FLAGS -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O1)

        # For using LeakSanitizer in standalone mode
        # https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer#stand-alone-mode
        # Using this mode was a bit unstable
        set(ICEORYX_LEAK_SANITIZER_FLAGS -fsanitize=leak)

        set(ICEORYX_ADDRESS_SANITIZER_FLAGS -fsanitize=address -fsanitize-address-use-after-scope ${ICEORYX_SANITIZER_BLACKLIST})

        # Combine different sanitizer flags to define overall sanitization
        set(ICEORYX_SANITIZER_FLAGS ${ICEORYX_SANITIZER_COMMON_FLAGS} ${ICEORYX_ADDRESS_SANITIZER_FLAGS} CACHE INTERNAL "")

        # unset local variables , to avoid polluting global space
        unset(ICEORYX_SANITIZER_BLACKLIST )
        unset(ICEORYX_SANITIZER_COMMON_FLAGS)
        unset(ICEORYX_ADDRESS_SANITIZER_FLAGS)
    else(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message( FATAL_ERROR "You need to run sanitize with gcc/clang compiler." )
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
endif(sanitize)

