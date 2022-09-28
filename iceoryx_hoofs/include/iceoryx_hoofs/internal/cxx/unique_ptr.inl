// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_UNIQUE_PTR_INL
#define IOX_HOOFS_CXX_UNIQUE_PTR_INL

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"

namespace iox
{
namespace cxx
{
template <typename T, typename D>
inline unique_ptr<T, D>::unique_ptr(T* const object, const function<D>& deleter) noexcept
    : m_ptr(object)
    , m_deleter(deleter)
{
    Ensures(object != nullptr);
}

template <typename T, typename D>
inline unique_ptr<T, D>& unique_ptr<T, D>::operator=(unique_ptr&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();
        m_ptr = rhs.m_ptr;
        m_deleter = std::move(rhs.m_deleter);
        rhs.m_ptr = nullptr;
    }
    return *this;
}

template <typename T, typename D>
inline unique_ptr<T, D>::unique_ptr(unique_ptr&& rhs) noexcept
    : m_ptr{rhs.m_ptr}
    , m_deleter{std::move(rhs.m_deleter)}
{
    rhs.m_ptr = nullptr;
}

template <typename T, typename D>
inline unique_ptr<T, D>::~unique_ptr() noexcept
{
    destroy();
}

template <typename T, typename D>
inline T* unique_ptr<T, D>::operator->() noexcept
{
    cxx::Expects(m_ptr != nullptr);
    return get();
}

template <typename T, typename D>
inline const T* unique_ptr<T, D>::operator->() const noexcept
{
    // Avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<unique_ptr<T, D>*>(this)->operator->();
}

template <typename T, typename D>
inline T* unique_ptr<T, D>::get() noexcept
{
    return m_ptr;
}

template <typename T, typename D>
inline const T* unique_ptr<T, D>::get() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : Avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<unique_ptr<T, D>*>(this)->get();
}

template <typename T, typename D>
inline T* unique_ptr<T, D>::release(unique_ptr&& ptrToBeReleased) noexcept
{
    auto ptr = ptrToBeReleased.m_ptr;
    ptrToBeReleased.m_ptr = nullptr;
    return ptr;
}

template <typename T, typename D>
inline void unique_ptr<T, D>::reset(T* const ptr) noexcept
{
    Ensures(ptr != nullptr);
    destroy();
    m_ptr = ptr;
}

template <typename T, typename D>
inline void unique_ptr<T, D>::destroy() noexcept
{
    if (m_ptr)
    {
        m_deleter(m_ptr);
    }
    m_ptr = nullptr;
}

template <typename T, typename D>
inline void unique_ptr<T, D>::swap(unique_ptr<T, D>& other) noexcept
{
    std::swap(m_ptr, other.m_ptr);
    std::swap(m_deleter, other.m_deleter);
}

template <typename T, typename U, typename D>
inline bool operator==(const unique_ptr<T, D>& lhs, const unique_ptr<U, D>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

template <typename T, typename U, typename D>
inline bool operator!=(const unique_ptr<T, D>& lhs, const unique_ptr<U, D>& rhs) noexcept
{
    return !(lhs == rhs);
}
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_UNIQUE_PTR_INL
