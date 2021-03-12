// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_INL
#define IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_INL

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
template <typename T>
inline relative_ptr<T>::relative_ptr(ptr_t ptr, id_t id) noexcept
    : RelativePointer(ptr, id)
{
}

template <typename T>
inline relative_ptr<T>::relative_ptr(offset_t offset, id_t id) noexcept
    : RelativePointer(offset, id)
{
}

template <typename T>
inline relative_ptr<T>::relative_ptr(ptr_t ptr) noexcept
    : RelativePointer(ptr)
{
}


template <typename T>
inline relative_ptr<T>::relative_ptr(const RelativePointer& other) noexcept
    : RelativePointer(other)
{
}

template <typename T>
inline relative_ptr<T>& relative_ptr<T>::operator=(const RelativePointer& other) noexcept
{
    RelativePointer::operator=(other);

    return *this;
}

template <typename T>
inline relative_ptr<T>& relative_ptr<T>::operator=(ptr_t ptr) noexcept
{
    m_id = searchId(ptr);
    m_offset = computeOffset(ptr);

    return *this;
}

template <typename T>
template <typename U>
inline typename std::enable_if<!std::is_void<U>::value, U&>::type relative_ptr<T>::operator*() noexcept
{
    return *get();
}

template <typename T>
inline T* relative_ptr<T>::operator->() noexcept
{
    return get();
}

template <typename T>
template <typename U>
inline typename std::enable_if<!std::is_void<U>::value, const U&>::type relative_ptr<T>::operator*() const noexcept
{
    return *get();
}

template <typename T>
inline T* relative_ptr<T>::operator->() const noexcept
{
    return get();
}

template <typename T>
inline T* relative_ptr<T>::get() const noexcept
{
    return static_cast<T*>(computeRawPtr());
}

template <typename T>
inline relative_ptr<T>::operator T*() const noexcept
{
    return get();
}

template <typename T>
inline bool relative_ptr<T>::operator==(T* const ptr) const noexcept
{
    return ptr == get();
}

template <typename T>
inline bool relative_ptr<T>::operator!=(T* const ptr) const noexcept
{
    return ptr != get();
}
} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_INL

