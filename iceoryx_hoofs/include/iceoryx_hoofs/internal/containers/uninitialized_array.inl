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

#include "iceoryx_hoofs/internal/containers/uninitialized_array.hpp"

namespace iox
{
namespace containers
{

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr UninitializedArray<ElementType, Capacity, Buffer>::UninitializedArray(
    const UninitializedArray&) noexcept
{
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr UninitializedArray<ElementType, Capacity, Buffer>::UninitializedArray(UninitializedArray&&) noexcept
{
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr UninitializedArray<ElementType, Capacity, Buffer>&
UninitializedArray<ElementType, Capacity, Buffer>::operator=(const UninitializedArray&) noexcept
{
    return *this;
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr UninitializedArray<ElementType, Capacity, Buffer>&
UninitializedArray<ElementType, Capacity, Buffer>::operator=(UninitializedArray&&) noexcept
{
    return *this;
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr ElementType&
UninitializedArray<ElementType, Capacity, Buffer>::operator[](const uint64_t index) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const_cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<ElementType&>(const_cast<const UninitializedArray*>(this)->operator[](index));
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr const ElementType&
UninitializedArray<ElementType, Capacity, Buffer>::operator[](const uint64_t index) const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : type safety ensured by template parameter
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<const ElementType*>(&m_buffer.value[index]);
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline constexpr uint64_t UninitializedArray<ElementType, Capacity, Buffer>::capacity() noexcept
{
    return Capacity;
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline typename UninitializedArray<ElementType, Capacity, Buffer>::iterator
UninitializedArray<ElementType, Capacity, Buffer>::begin() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const_cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<iterator>(const_cast<const UninitializedArray*>(this)->begin());
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline typename UninitializedArray<ElementType, Capacity, Buffer>::const_iterator
UninitializedArray<ElementType, Capacity, Buffer>::begin() const noexcept
{
    return &operator[](0);
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline typename UninitializedArray<ElementType, Capacity, Buffer>::iterator
UninitializedArray<ElementType, Capacity, Buffer>::end() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const_cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<iterator>(const_cast<const UninitializedArray*>(this)->end());
}

template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer>
inline typename UninitializedArray<ElementType, Capacity, Buffer>::const_iterator
UninitializedArray<ElementType, Capacity, Buffer>::end() const noexcept
{
    return &operator[](0) + Capacity;
}
} // namespace containers
} // namespace iox

#endif // IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_INL
