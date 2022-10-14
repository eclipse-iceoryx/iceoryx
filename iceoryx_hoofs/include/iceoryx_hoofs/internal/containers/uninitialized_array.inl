// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_INL
#define IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_INL

#include "iceoryx_hoofs/containers/uninitialized_array.hpp"

namespace iox
{
namespace containers
{
// UninitializedArray shall behave like a c-array; we explicitly do not want to initialize m_buffer. The constexpr
// default c'tor is needed to enable constexpr c'tors in classes that have an UnitializedArray member.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr UnitializedArray<ElementType, Capacity, index_t>::UnitializedArray() noexcept
{
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr ElementType& UnitializedArray<ElementType, Capacity, index_t>::operator[](const index_t index) noexcept
{
    return *toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr const ElementType&
UnitializedArray<ElementType, Capacity, index_t>::operator[](const index_t index) const noexcept
{
    return *toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr ElementType* UnitializedArray<ElementType, Capacity, index_t>::ptr(const index_t index) noexcept
{
    return toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr const ElementType*
UnitializedArray<ElementType, Capacity, index_t>::ptr(const index_t index) const noexcept
{
    return toPtr(index);
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr uint64_t UnitializedArray<ElementType, Capacity, index_t>::capacity() noexcept
{
    return Capacity;
}

template <typename ElementType, uint64_t Capacity, typename index_t>
inline constexpr ElementType* UnitializedArray<ElementType, Capacity, index_t>::toPtr(index_t index) const noexcept
{
    auto ptr = &(m_buffer[index * sizeof(ElementType)]);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-pro-type-const-cast) type erasure
    return reinterpret_cast<ElementType*>(const_cast<cxx::byte_t*>(ptr));
}

} // namespace containers
} // namespace iox

#endif // IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_INL
