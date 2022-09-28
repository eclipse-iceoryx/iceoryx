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

#ifndef IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_INL
#define IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_INL

#include "iceoryx_hoofs/internal/relocatable_pointer/relative_pointer.hpp"

namespace iox
{
namespace rp
{
template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(ptr_t ptr, segment_id_t id) noexcept
    : m_id(id)
    , m_offset(computeOffset(ptr))
{
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(offset_t offset, segment_id_t id) noexcept
    : m_id(id)
    , m_offset(offset)
{
}

template <typename T>
inline RelativePointer<T>::RelativePointer(ptr_t ptr) noexcept
    : m_id(searchId(ptr))
    , m_offset(computeOffset(ptr))
{
}

template <typename T>
inline RelativePointer<T>& RelativePointer<T>::operator=(const RelativePointer& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
    }
    return *this;
}

template <typename T>
RelativePointer<T>::RelativePointer(RelativePointer&& other) noexcept
{
    *this = std::move(other);
}

template <typename T>
RelativePointer<T>& RelativePointer<T>::operator=(RelativePointer&& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
        other.m_id = NULL_POINTER_ID;
        other.m_offset = NULL_POINTER_OFFSET;
    }
    return *this;
}

template <typename T>
inline RelativePointer<T>& RelativePointer<T>::operator=(ptr_t ptr) noexcept
{
    m_id = searchId(ptr);
    m_offset = computeOffset(ptr);

    return *this;
}

template <typename T>
template <typename U>
inline typename std::enable_if<!std::is_void<U>::value, const U&>::type RelativePointer<T>::operator*() const noexcept
{
    return *get();
}

template <typename T>
inline T* RelativePointer<T>::operator->() const noexcept
{
    auto* ptr = get();
    cxx::Ensures(ptr != nullptr);
    return ptr;
}

template <typename T>
inline T* RelativePointer<T>::get() const noexcept
{
    return static_cast<T*>(computeRawPtr());
}

template <typename T>
inline RelativePointer<T>::operator bool() const noexcept
{
    return computeRawPtr() != nullptr;
}

template <typename T>
inline bool RelativePointer<T>::operator==(T* const ptr) const noexcept
{
    return ptr == get();
}

template <typename T>
inline bool RelativePointer<T>::operator!=(T* const ptr) const noexcept
{
    return ptr != get();
}

template <typename T>
inline segment_id_underlying_t RelativePointer<T>::getId() const noexcept
{
    return m_id;
}

template <typename T>
inline typename RelativePointer<T>::offset_t RelativePointer<T>::getOffset() const noexcept
{
    return m_offset;
}

template <typename T>
inline typename RelativePointer<T>::ptr_t RelativePointer<T>::getBasePtr() const noexcept
{
    return getBasePtr(segment_id_t{m_id});
}

template <typename T>
inline cxx::optional<segment_id_underlying_t> RelativePointer<T>::registerPtr(const ptr_t ptr,
                                                                              const uint64_t size) noexcept
{
    return getRepository().registerPtr(ptr, size);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline bool RelativePointer<T>::registerPtrWithId(const segment_id_t id, const ptr_t ptr, const uint64_t size) noexcept
{
    return getRepository().registerPtrWithId(static_cast<segment_id_underlying_t>(id), ptr, size);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline bool RelativePointer<T>::unregisterPtr(const segment_id_t id) noexcept
{
    return getRepository().unregisterPtr(static_cast<segment_id_underlying_t>(id));
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline typename RelativePointer<T>::ptr_t RelativePointer<T>::getBasePtr(const segment_id_t id) noexcept
{
    return static_cast<RelativePointer<T>::ptr_t>(getRepository().getBasePtr(static_cast<segment_id_underlying_t>(id)));
}

template <typename T>
inline void RelativePointer<T>::unregisterAll() noexcept
{
    getRepository().unregisterAll();
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline typename RelativePointer<T>::offset_t RelativePointer<T>::getOffset(const segment_id_t id,
                                                                           const_ptr_t ptr) noexcept
{
    if (static_cast<segment_id_underlying_t>(id) == NULL_POINTER_ID)
    {
        return NULL_POINTER_OFFSET;
    }
    auto* basePtr = getBasePtr(id);
    // NOLINTJUSTIFICATION Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline typename RelativePointer<T>::ptr_t RelativePointer<T>::getPtr(const segment_id_t id,
                                                                     const offset_t offset) noexcept
{
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }
    auto* basePtr = getBasePtr(id);
    // NOLINTJUSTIFICATION Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
}

template <typename T>
inline segment_id_underlying_t RelativePointer<T>::searchId(ptr_t ptr) noexcept
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_ID;
    }
    return getRepository().searchId(ptr);
}

template <typename T>
inline typename RelativePointer<T>::offset_t RelativePointer<T>::computeOffset(ptr_t ptr) const noexcept
{
    return getOffset(segment_id_t{m_id}, ptr);
}

template <typename T>
inline typename RelativePointer<T>::ptr_t RelativePointer<T>::computeRawPtr() const noexcept
{
    return getPtr(segment_id_t{m_id}, m_offset);
}

inline PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t>& getRepository() noexcept
{
    static PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t> repository;
    return repository;
}
} // namespace rp
} // namespace iox

#endif // IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_INL
