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
#ifndef IOX_HOOFS_CXX_SPAN_INL
#define IOX_HOOFS_CXX_SPAN_INL

#include "iceoryx_hoofs/cxx/span.hpp"

namespace iox
{
namespace cxx
{
template <typename T>
inline span<T>::span(T* const data, const uint64_t size) noexcept
    : m_data{data}
    , m_size{size}
{
}

template <typename T>
inline T& span<T>::operator[](const uint64_t index) noexcept
{
    // NOLINTJUSTIFICATION low level construct which abstracts pointer arithmetic so that it does
    //                     not occur in the user code
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return m_data[index];
}

template <typename T>
inline const T& span<T>::operator[](const uint64_t index) const noexcept
{
    // NOLINTJUSTIFICATION low level construct which abstracts pointer arithmetic so that it does
    //                     not occur in the user code
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return m_data[index];
}

template <typename T>
inline uint64_t span<T>::size() const noexcept
{
    return m_size;
}
} // namespace cxx
} // namespace iox

#endif
