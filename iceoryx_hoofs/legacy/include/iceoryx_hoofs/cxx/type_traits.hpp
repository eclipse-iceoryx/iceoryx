// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_TYPE_TRAITS_HPP
#define IOX_HOOFS_CXX_TYPE_TRAITS_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/type_traits.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/type_traits.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::add_const_conditionally' instead of 'iox::cxx::add_const_conditionally'
using iox::add_const_conditionally;

/// @deprecated use 'iox::add_const_conditionally_t' instead of 'iox::cxx::add_const_conditionally_t'
using iox::add_const_conditionally_t;

/// @deprecated use 'iox::always_false_v' instead of 'iox::cxx::always_false_v'
using iox::always_false_v;

/// @deprecated use 'iox::is_invocable' instead of 'iox::cxx::is_invocable'
using iox::is_invocable;

/// @deprecated use 'iox::is_invocable_r' instead of 'iox::cxx::is_invocable_r'
using iox::is_invocable_r;

/// @deprecated use 'iox::is_function_pointer' instead of 'iox::cxx::is_function_pointer'
using iox::is_function_pointer;

/// @deprecated use 'iox::is_char_array' instead of 'iox::cxx::is_char_array'
using iox::is_char_array;

/// @deprecated use 'iox::void_t' instead of 'iox::cxx::void_t'
using iox::void_t;

/// @deprecated use 'iox::TypeInfo' instead of 'iox::cxx::TypeInfo'
using iox::TypeInfo;
} // namespace cxx
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_CXX_TYPE_TRAITS_HPP
