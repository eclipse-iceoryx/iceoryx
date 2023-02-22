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

#include "iox/string.hpp"

namespace iox
{
/// @todo iox-#1593 Deprecate include
/// [[deprecated("Deprecated in 3.0, removed in 4.0, please include 'iox/string.hpp' instead")]]
namespace cxx
{
/// @deprecated use 'iox::concatenate' instead of 'iox::cxx::concatenate'
using iox::concatenate;
/// @deprecated use 'iox::is_cxx_string' instead of 'iox::cxx::is_cxx_string'
using iox::is_cxx_string;
/// @deprecated use 'iox::IsCxxStringAndCxxStringOrCharArrayOrChar' instead of
/// 'iox::cxx::IsCxxStringAndCxxStringOrCharArrayOrChar'
using iox::IsCxxStringAndCxxStringOrCharArrayOrChar;
/// @deprecated use 'iox::IsCxxStringOrCharArray' instead of 'iox::cxx::IsCxxStringOrCharArray'
using iox::IsCxxStringOrCharArray;
/// @deprecated use 'iox::IsCxxStringOrCharArrayOrChar' instead of 'iox::cxx::IsCxxStringOrCharArrayOrChar'
using iox::IsCxxStringOrCharArrayOrChar;
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

#endif // IOX_HOOFS_CXX_STRING_HPP
