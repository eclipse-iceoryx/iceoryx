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
    set(ICEORYX_CXX_STANDARD 11)
elseif(QNX)
    set(ICEORYX_CXX_STANDARD 11)
elseif(WIN32)
    set(ICEORYX_CXX_STANDARD 11)
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
