// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_OPTIONAL_HPP
#define IOX_HOOFS_CXX_OPTIONAL_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/optional.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/optional.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::in_place' instead of 'iox::cxx::in_place'
using iox::in_place;
/// @deprecated use 'iox::in_place_t' instead of 'iox::cxx::in_place_t'
using iox::in_place_t;
/// @deprecated use 'iox::is_optional' instead of 'iox::cxx::is_optional'
using iox::is_optional;
/// @deprecated use 'iox::make_optional' instead of 'iox::cxx::make_optional'
using iox::make_optional;
/// @deprecated use 'iox::nullopt' instead of 'iox::cxx::nullopt'
using iox::nullopt;
/// @deprecated use 'iox::nullopt_t' instead of 'iox::cxx::nullopt_t'
using iox::nullopt_t;
/// @deprecated use 'iox::optional' instead of 'iox::cxx::optional'
using iox::optional;
} // namespace cxx
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_CXX_OPTIONAL_HPP
