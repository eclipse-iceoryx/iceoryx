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
#ifndef IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_HPP
#define IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/functional_interface.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/functional_interface.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
namespace internal
{
/// @deprecated use 'iox::internal::HasValueMethod' instead of 'iox::cxx::internal::HasValueMethod'
using iox::internal::HasValueMethod;

/// @deprecated use 'iox::internal::HasGetErrorMethod' instead of 'iox::cxx::internal::HasGetErrorMethod'
using iox::internal::HasGetErrorMethod;

/// @deprecated use 'iox::internal::Expect' instead of 'iox::cxx::internal::Expect'
using iox::internal::Expect;

/// @deprecated use 'iox::internal::ExpectWithValue' instead of 'iox::cxx::internal::ExpectWithValue'
using iox::internal::ExpectWithValue;

/// @deprecated use 'iox::internal::ValueOr' instead of 'iox::cxx::internal::ValueOr'
using iox::internal::ValueOr;

/// @deprecated use 'iox::internal::AndThenWithValue' instead of 'iox::cxx::internal::AndThenWithValue'
using iox::internal::AndThenWithValue;

/// @deprecated use 'iox::internal::AndThen' instead of 'iox::cxx::internal::AndThen'
using iox::internal::AndThen;

/// @deprecated use 'iox::internal::OrElseWithValue' instead of 'iox::cxx::internal::OrElseWithValue'
using iox::internal::OrElseWithValue;

/// @deprecated use 'iox::internal::OrElse' instead of 'iox::cxx::internal::OrElse'
using iox::internal::OrElse;

/// @deprecated use 'iox::internal::FunctionalInterfaceImpl' instead of 'iox::cxx::internal::FunctionalInterfaceImpl'
using iox::internal::FunctionalInterfaceImpl;
} // namespace internal

/// @deprecated use 'iox::FunctionalInterface' instead of 'iox::cxx::FunctionalInterface'
using iox::FunctionalInterface;

} // namespace cxx
} // namespace iox

// clang-format on

#endif
