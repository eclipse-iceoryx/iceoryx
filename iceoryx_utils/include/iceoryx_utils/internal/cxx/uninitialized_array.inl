// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_CXX_UNINITIALIZED_ARRAY_INL
#define IOX_UTILS_CXX_UNINITIALIZED_ARRAY_INL

#include "iceoryx_utils/cxx/uninitialized_array.hpp"
#include <iostream>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
inline uint64_t UninitializedArray<T, Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline uint64_t UninitializedArray<T, Capacity>::max_size() const noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline T* UninitializedArray<T, Capacity>::data() noexcept
{
    return reinterpret_cast<T*>(m_data);
}

template <typename T, uint64_t Capacity>
inline const T* UninitializedArray<T, Capacity>::data() const noexcept
{
    return reinterpret_cast<const T*>(m_data);
}


} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_UNINITIALIZED_ARRAY_INL
