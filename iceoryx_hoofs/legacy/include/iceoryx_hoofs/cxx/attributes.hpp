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

#ifndef IOX_HOOFS_CXX_ATTRIBUTES_HPP
#define IOX_HOOFS_CXX_ATTRIBUTES_HPP

#include "iox/attributes.hpp"
#include "iox/detail/deprecation_marker.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/attributes.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
namespace internal
{
/// @deprecated use 'iox::internal::IOX_DISCARD_RESULT_IMPL' instead of 'iox::cxx::internal::IOX_DISCARD_RESULT_IMPL'
using iox::internal::IOX_DISCARD_RESULT_IMPL;
} // namespace internal
} // namespace cxx
} // namespace iox

// clang-format on

/// @deprecated use '[[nodiscard]]' instead of 'IOX_NO_DISCARD'
#define IOX_NO_DISCARD [[nodiscard, IOX_NO_DISCARD_is_deprecated_use__nodiscard__attribute]]

/// @deprecated use '[[fallthrough]]' instead of 'IOX_FALLTHROUGH'
#define IOX_FALLTHROUGH [[fallthrough, IOX_FALLTHROUGH_is_deprecated_use__fallthrough__attribute]]

/// @deprecated use '[[maybe_unused]]' instead of 'IOX_MAYBE_UNUSED'
#define IOX_MAYBE_UNUSED [[maybe_unused, IOX_MAYBE_UNUSED_is_deprecated_use__maybe_unused__attribute]]

#endif
