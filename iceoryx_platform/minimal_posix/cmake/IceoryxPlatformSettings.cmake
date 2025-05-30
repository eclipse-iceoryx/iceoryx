# Copyright (c) 2025 by ekxide IO GmbH. All rights reserved.
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
# SPDX-License-Identifier: Apache-2.0

set_global(VAR ICEORYX_CXX_STANDARD         VALUE 17)
set_global(VAR ICEORYX_PLATFORM_STRING      VALUE "MINIMAL_POSIX")

set_global(VAR ICEORYX_C_FLAGS              VALUE )
set_global(VAR ICEORYX_CXX_FLAGS            VALUE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
set_global(VAR ICEORYX_TEST_CXX_FLAGS       VALUE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)


set_global(VAR ICEORYX_C_WARNINGS           VALUE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall -Wextra -Wuninitialized -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion>
)
set_global(VAR ICEORYX_CXX_WARNINGS         VALUE
    $<$<CXX_COMPILER_ID:MSVC>:${ICEORYX_C_WARNINGS}>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${ICEORYX_C_WARNINGS} -Wno-noexcept-type>
)

if(BUILD_STRICT)
    set_global(VAR ICEORYX_C_WARNINGS       VALUE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${ICEORYX_C_WARNINGS} -Werror>
    )
    set_global(VAR ICEORYX_CXX_WARNINGS     VALUE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${ICEORYX_CXX_WARNINGS} -Werror>
    )
endif()
