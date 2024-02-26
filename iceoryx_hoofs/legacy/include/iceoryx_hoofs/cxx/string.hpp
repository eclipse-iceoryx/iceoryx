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

#ifndef IOX_HOOFS_CXX_STRING_HPP
#define IOX_HOOFS_CXX_STRING_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/string.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/string.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") cxx
{
/// @deprecated use 'iox::concatenate' instead of 'iox::cxx::concatenate'
using iox::concatenate;
/// @deprecated use 'iox::is_iox_string' instead of 'iox::cxx::is_cxx_string'
template <typename T>
using is_cxx_string = iox::is_iox_string<T>;
/// @deprecated use 'iox::IsIoxStringAndIoxStringOrCharArrayOrChar' instead of
/// 'iox::cxx::IsCxxStringAndCxxStringOrCharArrayOrChar'
template <typename T1, typename T2, typename ReturnType>
using IsCxxStringAndCxxStringOrCharArrayOrChar = iox::IsIoxStringAndIoxStringOrCharArrayOrChar<T1, T2, ReturnType>;
/// @deprecated use 'iox::IsIoxStringOrCharArray' instead of 'iox::cxx::IsCxxStringOrCharArray'
template <typename T, typename ReturnType>
using IsCxxStringOrCharArray = iox::IsIoxStringOrCharArray<T, ReturnType>;
/// @deprecated use 'iox::IsIoxStringOrCharArrayOrChar' instead of 'iox::cxx::IsCxxStringOrCharArrayOrChar'
template <typename T1, typename T2, typename ReturnType>
using IsCxxStringOrCharArrayOrChar = iox::IsIoxStringOrCharArrayOrChar<T1, T2, ReturnType>;
/// @deprecated use 'iox::IsStringOrCharArray' instead of 'iox::cxx::IsStringOrCharArray'
using iox::IsStringOrCharArray;
/// @deprecated use 'iox::IsStringOrCharArrayOrChar' instead of 'iox::cxx::IsStringOrCharArrayOrChar'
using iox::IsStringOrCharArrayOrChar;
/// @deprecated use 'iox::string' instead of 'iox::string'
using iox::string;
/// @deprecated use 'iox::TruncateToCapacity' instead of 'iox::cxx::TruncateToCapacity'
using iox::TruncateToCapacity;
/// @deprecated use 'iox::TruncateToCapacity_t' instead of 'iox::cxx::TruncateToCapacity_t'
using iox::TruncateToCapacity_t;
} // namespace cxx
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_CXX_STRING_HPP
