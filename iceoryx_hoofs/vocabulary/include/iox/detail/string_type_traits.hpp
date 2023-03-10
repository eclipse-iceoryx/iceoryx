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
#ifndef IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP
#define IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP

#include <cstdint>

#include "iox/type_traits.hpp"

namespace iox
{
template <uint64_t Capacity>
class string;

/// @brief struct to check whether an argument is a iox::string
template <typename T>
struct is_iox_string : std::false_type
{
};

template <uint64_t N>
struct is_iox_string<::iox::string<N>> : std::true_type
{
};

} // namespace iox

#endif // IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP
