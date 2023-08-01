// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_POINTER_REPOSITORY_INL
#define IOX_HOOFS_MEMORY_POINTER_REPOSITORY_INL

#include "iox/detail/pointer_repository.hpp"

namespace iox
{
template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline PointerRepository<id_t, ptr_t, CAPACITY>::PointerRepository() noexcept
    : m_info(CAPACITY)
{
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::registerPtrWithId(const id_t id,
                                                                        const ptr_t ptr,
                                                                        const uint64_t size) noexcept
{
    if (id > MAX_ID)
    {
        return false;
    }
    return addPointerIfIdIsFree(id, ptr, size);
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline optional<id_t> PointerRepository<id_t, ptr_t, CAPACITY>::registerPtr(const ptr_t ptr,
                                                                            const uint64_t size) noexcept
{
    for (id_t id{1U}; id <= MAX_ID; ++id)
    {
        if (addPointerIfIdIsFree(id, ptr, size))
        {
            return id;
        }
    }

    return nullopt;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::unregisterPtr(const id_t id) noexcept
{
    if ((id <= MAX_ID) && (id >= MIN_ID))
    {
        if (m_info[id].basePtr != nullptr)
        {
            m_info[id].basePtr = nullptr;

            /// @note do not search for next lower registered index but we could do it here
            return true;
        }
    }

    return false;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline void PointerRepository<id_t, ptr_t, CAPACITY>::unregisterAll() noexcept
{
    for (auto& info : m_info)
    {
        info.basePtr = nullptr;
    }
    m_maxRegistered = 0U;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline ptr_t PointerRepository<id_t, ptr_t, CAPACITY>::getBasePtr(const id_t id) const noexcept
{
    if ((id <= MAX_ID) && (id >= MIN_ID))
    {
        return m_info[id].basePtr;
    }

    /// @note for id 0 nullptr is returned, meaning we will later interpret a relative pointer by casting the offset
    /// into a pointer (i.e. we measure relative to 0)

    /// @note we cannot distinguish between not registered and nullptr registered, but we do not need to
    return nullptr;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline id_t PointerRepository<id_t, ptr_t, CAPACITY>::searchId(const ptr_t ptr) const noexcept
{
    for (id_t id{1U}; id <= m_maxRegistered; ++id)
    {
        // return first id where the ptr is in the corresponding interval
        // AXIVION Next Construct AutosarC++19_03-M5.14.1 : False positive. vector::operator[](index) has no side-effect when index is less than vector size which is guaranteed by PointerRepository design
        if ((ptr >= m_info[id].basePtr) && (ptr <= m_info[id].endPtr))
        {
            return id;
        }
    }
    /// @note treat the pointer as a regular pointer if not found
    /// by setting id to RAW_POINTER_BEHAVIOUR_ID
    return RAW_POINTER_BEHAVIOUR_ID;
}
template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::addPointerIfIdIsFree(const id_t id,
                                                                           const ptr_t ptr,
                                                                           const uint64_t size) noexcept
{
    if (m_info[id].basePtr == nullptr)
    {
        m_info[id].basePtr = ptr;
        // AXIVION Next Construct AutosarC++19_03-M5.2.9 : Used for pointer arithmetic with void pointer, uintptr_t is capable of holding a void ptr
        // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Cast is needed for pointer arithmetic and casted back
        // to the original type
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        m_info[id].endPtr = reinterpret_cast<ptr_t>(reinterpret_cast<uintptr_t>(ptr) + (size - 1U));

        if (id > m_maxRegistered)
        {
            m_maxRegistered = id;
        }
        return true;
    }
    return false;
}

} // namespace iox

#endif // IOX_HOOFS_MEMORY_POINTER_REPOSITORY_INL
