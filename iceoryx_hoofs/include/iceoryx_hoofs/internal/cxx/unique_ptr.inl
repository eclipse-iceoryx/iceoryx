// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_UNIQUE_PTR_INL
#define IOX_HOOFS_CXX_UNIQUE_PTR_INL

namespace iox
{
namespace cxx
{
template <typename T>
unique_ptr<T>::unique_ptr(T* const ptr, function_ref<void(T*)>&& deleter) noexcept
    : m_ptr(ptr)
    , m_deleter(std::move(deleter))
{
}

template <typename T>
unique_ptr<T>::unique_ptr(function_ref<void(T*)>&& deleter) noexcept
    : unique_ptr(nullptr, std::move(deleter))
{
}

template <typename T>
unique_ptr<T>& unique_ptr<T>::operator=(std::nullptr_t) noexcept
{
    reset();
    return *this;
}

template <typename T>
unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr&& rhs) noexcept
{
    if (this != &rhs)
    {
        reset(rhs.release());
        m_deleter = std::move(rhs.m_deleter);
    }
    return *this;
}

template <typename T>
unique_ptr<T>::unique_ptr(unique_ptr&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename T>
unique_ptr<T>::~unique_ptr() noexcept
{
    reset();
}

template <typename T>
T* unique_ptr<T>::operator->() noexcept
{
    return get();
}

template <typename T>
const T* unique_ptr<T>::operator->() const noexcept
{
    return const_cast<const T*>(get());
}

template <typename T>
unique_ptr<T>::operator bool() const noexcept
{
    return get() != nullptr ? true : false;
}

template <typename T>
T* unique_ptr<T>::get() noexcept
{
    return m_ptr;
}

template <typename T>
const T* unique_ptr<T>::get() const noexcept
{
    return const_cast<const T*>(m_ptr);
}

template <typename T>
T* unique_ptr<T>::release() noexcept
{
    auto ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}

template <typename T>
void unique_ptr<T>::reset(T* const ptr) noexcept
{
    if (m_ptr && m_deleter)
    {
        m_deleter(m_ptr);
    }
    m_ptr = ptr;
}

template <typename T>
void unique_ptr<T>::swap(unique_ptr<T>& other) noexcept
{
    // release object pointers from both instances
    auto thisPtr = release();
    auto otherPtr = other.release();

    // set new object pointers on both instances
    reset(otherPtr);
    other.reset(thisPtr);

    // move deleters
    auto thisDeleter = m_deleter;
    m_deleter = other.m_deleter;
    other.m_deleter = thisDeleter;
}

template <typename T, typename U>
bool operator==(const unique_ptr<T>& x, const unique_ptr<U>& y) noexcept
{
    return x.get() == y.get();
}

template <typename T>
bool operator==(const unique_ptr<T>& x, std::nullptr_t) noexcept
{
    return !x;
}

template <typename T>
bool operator==(std::nullptr_t, const unique_ptr<T>& x) noexcept
{
    return !x;
}

template <typename T, typename U>
bool operator!=(const unique_ptr<T>& x, const unique_ptr<U>& y) noexcept
{
    return x.get() != y.get();
}

template <typename T>
bool operator!=(const unique_ptr<T>& x, std::nullptr_t) noexcept
{
    return static_cast<bool>(x);
}

template <typename T>
bool operator!=(std::nullptr_t, const unique_ptr<T>& x) noexcept
{
    return static_cast<bool>(x);
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_UNIQUE_PTR_INL
