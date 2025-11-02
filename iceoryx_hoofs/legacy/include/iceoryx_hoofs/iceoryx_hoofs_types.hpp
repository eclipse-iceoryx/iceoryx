// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP
#define IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/iceoryx_hoofs_types.hpp"

#include <cstddef>

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/iceoryx_hoofs_types.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'std::byte' instead of 'iox::cxx::byte_t'
using byte_t IOX_DEPRECATED_SINCE(3, "Please use 'std::byte' instead.") = std::byte;

} // namespace cxx
namespace log
{
/// @deprecated use 'iox::log::LogLevel' instead of 'iox::cxx::log::LogLevel'
using iox::log::LogLevel;
} // namespace log
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP
