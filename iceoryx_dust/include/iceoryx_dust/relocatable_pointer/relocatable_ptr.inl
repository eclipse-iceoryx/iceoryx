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

#ifndef IOX_DUST_RELOCATABLE_POINTER_RELOCATABLE_PTR_INL
#define IOX_DUST_RELOCATABLE_POINTER_RELOCATABLE_PTR_INL

#include "iceoryx_dust/relocatable_pointer/relocatable_ptr.hpp"

namespace iox
{
namespace memory
{
template <typename T>
inline relocatable_ptr<T>::relocatable_ptr(T* ptr) noexcept
    : m_offset(to_offset(ptr))
{
}

template <typename T>
inline relocatable_ptr<T>::relocatable_ptr(const relocatable_ptr& other) noexcept
    : m_offset(to_offset(other.get()))
{
}

template <typename T>
inline relocatable_ptr<T>::relocatable_ptr(relocatable_ptr&& other) noexcept
    : m_offset(to_offset(other.get()))
{
    other.m_offset = NULL_POINTER_OFFSET;
}

template <typename T>
inline relocatable_ptr<T>& relocatable_ptr<T>::operator=(const relocatable_ptr& rhs) noexcept
{
    if (this != &rhs)
    {
        m_offset = to_offset(rhs.get());
    }
    return *this;
}

template <typename T>
inline relocatable_ptr<T>& relocatable_ptr<T>::operator=(relocatable_ptr&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_offset = to_offset(rhs.get());
        rhs.m_offset = NULL_POINTER_OFFSET;
    }
    return *this;
}

template <typename T>
inline T* relocatable_ptr<T>::get() noexcept
{
    return from_offset(m_offset);
}

template <typename T>
inline const T* relocatable_ptr<T>::get() const noexcept
{
    return from_offset(m_offset);
}

template <typename T>
template <typename S>
inline S& relocatable_ptr<T>::operator*() noexcept
{
    // not actually evaluated in the error case (compiler fails earlier since S = void leads to void&)
    static_assert(!std::is_same<S, void>::value, "relocatable_ptr<void> does not support operator*");
    return *get();
}

template <typename T>
template <typename S>
inline const S& relocatable_ptr<T>::operator*() const noexcept
{
    // not actually evaluated in the error case (compiler fails earlier since S = void leads to void&)
    static_assert(!std::is_same<S, void>::value, "relocatable_ptr<void> does not support operator* const");
    return *get();
}

template <typename T>
inline T* relocatable_ptr<T>::operator->() noexcept
{
    return get();
}

template <typename T>
inline const T* relocatable_ptr<T>::operator->() const noexcept
{
    return get();
}

template <typename T>
inline relocatable_ptr<T>::operator T*() noexcept
{
    return get();
}

template <typename T>
inline relocatable_ptr<T>::operator const T*() const noexcept
{
    return get();
}

template <typename T>
inline typename relocatable_ptr<T>::offset_t relocatable_ptr<T>::self() const noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) cast required to obtain object address as offset
    return reinterpret_cast<offset_t>(this);
}

template <typename T>
inline typename relocatable_ptr<T>::offset_t relocatable_ptr<T>::to_offset(const void* ptr) const noexcept
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_OFFSET;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) cast required to obtain offset from ptr
    auto p = reinterpret_cast<offset_t>(ptr);
    return p - self();
}

template <typename T>
inline T* relocatable_ptr<T>::from_offset(offset_t offset) const noexcept
{
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) cast required to ptr from offset
    return reinterpret_cast<T*>(offset + self());
}

template <typename T>
inline bool operator==(const relocatable_ptr<T>& lhs, const relocatable_ptr<T>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

/// @brief Compare relocatable_ptr with respect to logical inequality.
/// @return true if rhs and lhs point to a different location, false otherwise
template <typename T>
inline bool operator!=(const relocatable_ptr<T>& lhs, const relocatable_ptr<T>& rhs) noexcept
{
    return !operator==(lhs, rhs);
}

} // namespace memory
} // namespace iox

#endif // IOX_DUST_RELOCATABLE_POINTER_RELOCATABLE_PTR_INL
