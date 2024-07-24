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

#ifndef IOX_HOOFS_MEMORY_RELATIVE_POINTER_INL
#define IOX_HOOFS_MEMORY_RELATIVE_POINTER_INL

#include "iox/assertions.hpp"
#include "iox/relative_pointer.hpp"

namespace iox
{
template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(ptr_t const ptr, const segment_id_t id) noexcept
    : RelativePointer(getOffset(id, ptr), id)
{
}

template <typename T>
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This is the main c'tor which the other c'tors use
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(const offset_t offset, const segment_id_t id) noexcept
    : m_id(id)
    , m_offset(offset)
{
}

template <typename T>
inline RelativePointer<T>::RelativePointer(ptr_t const ptr) noexcept
    : RelativePointer([this, ptr]() noexcept -> RelativePointer {
        const segment_id_t id{this->searchId(ptr)};
        const offset_t offset{this->getOffset(id, ptr)};
        return RelativePointer{offset, id};
    }())
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
    : m_id(std::move(other.m_id))
    , m_offset(std::move(other.m_offset))
{
    other.m_id = NULL_POINTER_ID;
    other.m_offset = NULL_POINTER_OFFSET;
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
inline RelativePointer<T>& RelativePointer<T>::operator=(ptr_t const ptr) noexcept
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
    auto* const ptr{get()};
    IOX_ENFORCE(ptr != nullptr, "should not happen unless src is incorrectly used after move");
    return ptr;
}

template <typename T>
inline T* RelativePointer<T>::get() const noexcept
{
    return static_cast<ptr_t>(computeRawPtr());
}

template <typename T>
inline RelativePointer<T>::operator bool() const noexcept
{
    return computeRawPtr() != nullptr;
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
inline T* RelativePointer<T>::getBasePtr() const noexcept
{
    return getBasePtr(segment_id_t{m_id});
}

template <typename T>
inline optional<segment_id_underlying_t> RelativePointer<T>::registerPtr(ptr_t const ptr, const uint64_t size) noexcept
{
    return getRepository().registerPtr(ptr, size);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline bool RelativePointer<T>::registerPtrWithId(const segment_id_t id, ptr_t const ptr, const uint64_t size) noexcept
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
inline T* RelativePointer<T>::getBasePtr(const segment_id_t id) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-M5.2.8 : Cast to the underyling pointer type is safe as this is
    // encapsulated in the RelativePointer class and type safety is ensured by using templates
    return static_cast<ptr_t>(getRepository().getBasePtr(static_cast<segment_id_underlying_t>(id)));
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
                                                                           ptr_t const ptr) noexcept
{
    if (static_cast<segment_id_underlying_t>(id) == NULL_POINTER_ID)
    {
        return NULL_POINTER_OFFSET;
    }
    const auto* const basePtr = getBasePtr(id);
    // AXIVION Next Construct AutosarC++19_03-A5.2.4, AutosarC++19_03-M5.2.9 : Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<std::uintptr_t>(ptr) - reinterpret_cast<std::uintptr_t>(basePtr);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline T* RelativePointer<T>::getPtr(const segment_id_t id, const offset_t offset) noexcept
{
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }
    const auto* const basePtr = getBasePtr(id);
    // AXIVION DISABLE STYLE AutosarC++19_03-A5.2.4 : Cast needed for pointer arithmetic
    // AXIVION DISABLE STYLE AutosarC++19_03-M5.2.8 : Cast needed for pointer arithmetic
    // AXIVION DISABLE STYLE AutosarC++19_03-M5.2.9 : Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    return reinterpret_cast<ptr_t>(offset + reinterpret_cast<std::uintptr_t>(basePtr));
    // AXIVION ENABLE STYLE AutosarC++19_03-M5.2.9
    // AXIVION ENABLE STYLE AutosarC++19_03-M5.2.8
    // AXIVION ENABLE STYLE AutosarC++19_03-A5.2.4
}

template <typename T>
inline segment_id_underlying_t RelativePointer<T>::searchId(ptr_t const ptr) noexcept
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_ID;
    }
    return getRepository().searchId(ptr);
}

template <typename T>
inline typename RelativePointer<T>::offset_t RelativePointer<T>::computeOffset(ptr_t const ptr) const noexcept
{
    return getOffset(segment_id_t{m_id}, ptr);
}

template <typename T>
inline T* RelativePointer<T>::computeRawPtr() const noexcept
{
    return getPtr(segment_id_t{m_id}, m_offset);
}

// AXIVION Next Construct AutosarC++19_03-A15.5.3, AutosarC++19_03-A15.4.2, FaultDetection-NoexceptViolations : False
// positive, std::terminate is not called in the c'tor of PointerRepository and noexcept-specification is not violated
inline PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t>& getRepository() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A3.3.2 : PointerRepository can't be constexpr, usage of the static
    // object is encapsulated in the RelativePointer class
    static PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t> repository;
    return repository;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
inline bool operator==(const RelativePointer<T> lhs, const T* const rhs) noexcept
{
    return lhs.get() == rhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
inline bool operator==(const T* const lhs, const RelativePointer<T> rhs) noexcept
{
    return rhs == lhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
inline bool operator==(std::nullptr_t, const RelativePointer<T> rhs) noexcept
{
    return rhs.get() == nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
inline bool operator==(const RelativePointer<T> lhs, std::nullptr_t) noexcept
{
    return lhs.get() == nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
inline bool operator!=(const RelativePointer<T> lhs, const T* const rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
inline bool operator!=(const T* const lhs, const RelativePointer<T> rhs) noexcept
{
    return rhs != lhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
inline bool operator!=(std::nullptr_t, const RelativePointer<T> rhs) noexcept
{
    return rhs.get() != nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
inline bool operator!=(const RelativePointer<T> lhs, std::nullptr_t) noexcept
{
    return lhs.get() != nullptr;
}
} // namespace iox

#endif // IOX_HOOFS_MEMORY_RELATIVE_POINTER_INL
