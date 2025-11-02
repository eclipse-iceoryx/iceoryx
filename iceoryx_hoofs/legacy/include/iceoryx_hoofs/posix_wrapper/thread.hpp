// Copyright (c) 2020, 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
#define IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/thread.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/thread.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") posix
{
IOX_DEPRECATED_SINCE(3, "Please use 'iox::MAX_THREAD_NAME_LENGTH' from 'iox/thread.hpp' instead.")
constexpr uint64_t MAX_THREAD_NAME_LENGTH{iox::MAX_THREAD_NAME_LENGTH};

using ThreadName_t IOX_DEPRECATED_SINCE(3,
                                        "Please use 'iox::ThreadName_t' from 'iox/thread.hpp' instead.") = ThreadName_t;

IOX_DEPRECATED_SINCE(3, "Please use 'iox::setThreadName' from 'iox/thread.hpp' instead.")
inline void setThreadName(const ThreadName_t& name) noexcept
{
    iox::setThreadName(name);
}

IOX_DEPRECATED_SINCE(3, "Please use 'iox::getThreadName' from 'iox/thread.hpp' instead.")
inline ThreadName_t getThreadName() noexcept
{
    return iox::getThreadName();
}

} // namespace posix
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
