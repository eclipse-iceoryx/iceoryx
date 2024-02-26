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

#ifndef IOX_HOOFS_CXX_FUNCTION_REF_HPP
#define IOX_HOOFS_CXX_FUNCTION_REF_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/function_ref.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/function_ref.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::function_ref' instead of 'iox::cxx::function_ref'
using iox::function_ref;

/// @deprecated use 'iox::has_same_decayed_type' instead of 'iox::cxx::has_same_decayed_type'
using iox::has_same_decayed_type;

} // namespace cxx
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_CXX_FUNCTION_REF_HPP
