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
#ifndef IOX_DUST_STD_STRING_SUPPORT_INL
#define IOX_DUST_STD_STRING_SUPPORT_INL

#include "iceoryx_dust/cxx/std_string_support.hpp"

namespace iox
{
template <uint64_t N>
inline std::string FromImpl<string<N>, std::string>::fromImpl(const string<N>& value)
{
    return std::string(value.c_str(), value.size());
}

template <uint64_t N>
inline string<N> FromImpl<std::string, string<N>>::fromImpl(const std::string& value) noexcept
{
    return string<N>(TruncateToCapacity, value.c_str(), value.size());
}
} // namespace iox

#endif // IOX_DUST_STD_STRING_SUPPORT_INL
