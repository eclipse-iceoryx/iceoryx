// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

// NOTE: This file is called '_string.cpp' due to issues with clang-tidy. When the file is called 'string.cpp',
// clang-tidy assumes it to be the translation unit for 'iox/string.hpp' and tries to check that header with the wrong
// translation unit which leads to an error not finding the includes from 'iox/string.hpp' and 'iox/detail/string.inl'

#include "iceoryx_platform/attributes.hpp"
#include "iceoryx_platform/string.hpp"
#include <cerrno>

namespace
{
IOX_MAYBE_UNUSED char* strerror_r_gnu_xsi_unificaton(const int returnCode IOX_MAYBE_UNUSED, char* buf)
{
    return buf;
}

IOX_MAYBE_UNUSED char* strerror_r_gnu_xsi_unificaton(char* msg, char* buf IOX_MAYBE_UNUSED)
{
    return msg;
}

} // namespace

char* iox_gnu_strerror_r(int errnum, char* buf, size_t buflen)
{
    return strerror_r_gnu_xsi_unificaton(strerror_r(errnum, buf, buflen), buf);
}
