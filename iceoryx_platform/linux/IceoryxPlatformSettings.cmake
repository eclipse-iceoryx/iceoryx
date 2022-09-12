# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

set_global(VAR ICEORYX_CXX_STANDARD         VALUE 14)
set_global(VAR ICEORYX_PLATFORM_STRING      VALUE "Linux")

set_global(VAR ICEORYX_C_WARNINGS           VALUE -W -Wall -Wextra -Wuninitialized -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion)
set_global(VAR ICEORYX_CXX_WARNINGS         VALUE ${ICEORYX_C_WARNINGS} -Wno-noexcept-type)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set_global(VAR ICEORYX_CXX_WARNINGS     VALUE ${ICEORYX_CXX_WARNINGS} -Wuseless-cast)
endif()

if(BUILD_STRICT)
    set_global(VAR ICEORYX_C_WARNINGS       VALUE ${ICEORYX_C_WARNINGS} -Werror)
    set_global(VAR ICEORYX_CXX_WARNINGS     VALUE ${ICEORYX_CXX_WARNINGS} -Werror)
endif()


