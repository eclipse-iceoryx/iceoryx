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

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"

namespace iox
{
namespace cxx
{
template <typename T>
inline unique_ptr<T>::unique_ptr(T& object, const function<void(T*)>& deleter) noexcept
    : m_ptr(&object)
    , m_deleter(std::move(deleter))
{
}

template <typename T>
inline unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();
        m_ptr = std::move(rhs.m_ptr);
        m_deleter = std::move(rhs.m_deleter);

        rhs.m_ptr = nullptr;
    }
    return *this;
}

template <typename T>
inline unique_ptr<T>::unique_ptr(unique_ptr&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename T>
inline unique_ptr<T>::~unique_ptr() noexcept
{
    destroy();
}

template <typename T>
inline T* unique_ptr<T>::operator->() noexcept
{
    return get();
}

template <typename T>
inline const T* unique_ptr<T>::operator->() const noexcept
{
    // Avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<const T*>(get());
}

template <typename T>
inline T* unique_ptr<T>::get() noexcept
{
    cxx::Expects(m_ptr != nullptr);
    return m_ptr;
}

template <typename T>
inline const T* unique_ptr<T>::get() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : Avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<unique_ptr<T>*>(this)->get();
}

template <typename T>
inline T* unique_ptr<T>::release(unique_ptr<T>&& releasedPtr) noexcept
{
    auto ptr = releasedPtr.m_ptr;
    releasedPtr.m_ptr = nullptr;
    return ptr;
}

template <typename T>
inline void unique_ptr<T>::reset(T& object) noexcept
{
    destroy();
    m_ptr = &object;
}

template <typename T>
inline void unique_ptr<T>::destroy() noexcept
{
    if (m_deleter)
    {
        m_deleter(m_ptr);
    }

    m_ptr = nullptr;
}

template <typename T>
inline void unique_ptr<T>::swap(unique_ptr<T>& other) noexcept
{
    std::swap(this->m_ptr, other.m_ptr);
    std::swap(this->m_deleter, other.m_deleter);
}

template <typename T, typename U>
inline bool operator==(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

template <typename T, typename U>
inline bool operator!=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept
{
    return !(lhs == rhs);
}
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_UNIQUE_PTR_INL
