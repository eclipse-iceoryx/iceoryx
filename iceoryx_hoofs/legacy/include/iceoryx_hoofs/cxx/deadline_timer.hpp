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

#ifndef IOX_HOOFS_CXX_DEADLINE_TIMER_HPP
#define IOX_HOOFS_CXX_DEADLINE_TIMER_HPP

#include "iox/deadline_timer.hpp"
#include "iox/detail/deprecation_marker.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/deadline_timer.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::deadline_timer' instead of 'iox::cxx::DeadlineTimer'
using DeadlineTimer IOX_DEPRECATED_SINCE(3, "Please use 'iox::deadline_timer' instead.") = iox::deadline_timer;
} // namespace cxx
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_CXX_DEADLINE_TIMER_HPP
