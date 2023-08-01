// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_HOOFS_CXX_OPTIONAL_HPP
#define IOX_HOOFS_CXX_OPTIONAL_HPP

#include "iox/optional.hpp"

namespace iox
{
/// @todo iox-#1593 Deprecate include
/// [[deprecated("Deprecated in 3.0, removed in 4.0, please include 'iox/optional.hpp' instead")]]
namespace cxx
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

#endif // IOX_HOOFS_CXX_OPTIONAL_HPP
