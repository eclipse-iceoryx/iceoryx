# Copyright (c) 2025 by ekxide IO GmbH. All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Apache Software License 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
# which is available at https://opensource.org/licenses/MIT.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT

set_global(VAR ICEORYX_CXX_STANDARD         VALUE 14)
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
