// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_SYNC_SMART_LOCK_INL
#define IOX_HOOFS_CONCURRENT_SYNC_SMART_LOCK_INL

#include "iox/smart_lock.hpp"

namespace iox
{
namespace concurrent
{
template <typename T, typename MutexType, typename... Targs>
inline smart_lock<T, MutexType> make_smart_lock(Targs&&... args) noexcept
{
    return smart_lock<T, MutexType>(T(std::forward<Targs>(args)...));
}

template <typename T, typename MutexType>
template <typename... ArgTypes>
// NOLINTNEXTLINE(readability-named-parameter, hicpp-named-parameter) justification in Doxygen documentation
inline smart_lock<T, MutexType>::smart_lock(ForwardArgsToCTor_t, ArgTypes&&... args) noexcept
    : base(std::forward<ArgTypes>(args)...)
{
}

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>::smart_lock(const smart_lock& rhs) noexcept
    : base([&] {
        std::lock_guard<MutexType> guard(rhs.lock);
        return rhs.base;
    }())
{
}

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>::smart_lock(smart_lock&& rhs) noexcept
    : base([&] {
        std::lock_guard<MutexType> guard(rhs.lock);
        return std::move(rhs.base);
    }())
{
}

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>& smart_lock<T, MutexType>::operator=(const smart_lock& rhs) noexcept
{
    if (this != &rhs)
    {
        std::lock(lock, rhs.lock);
        std::lock_guard<MutexType> guard(lock, std::adopt_lock);
        std::lock_guard<MutexType> guardRhs(rhs.lock, std::adopt_lock);
        base = rhs.base;
    }

    return *this;
}

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>& smart_lock<T, MutexType>::operator=(smart_lock&& rhs) noexcept
{
    if (this != &rhs)
    {
        std::lock(lock, rhs.lock);
        std::lock_guard<MutexType> guard(lock, std::adopt_lock);
        std::lock_guard<MutexType> guardRhs(rhs.lock, std::adopt_lock);
        base = std::move(rhs.base);
    }
    return *this;
}

template <typename T, typename MutexType>
inline typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::operator->() noexcept
{
    return Proxy(base, lock);
}

template <typename T, typename MutexType>
// const return type improves const correctness, operator-> can be chained with underlying operator->
// NOLINTNEXTLINE(readability-const-return-type)
inline const typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::operator->() const noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) const_cast to avoid code duplication
    return const_cast<smart_lock<T, MutexType>*>(this)->operator->();
}

template <typename T, typename MutexType>
inline typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::get_scope_guard() noexcept
{
    return Proxy(base, lock);
}

template <typename T, typename MutexType>
// const return type improves const correctness, operator-> can be chained with underlying operator->
// NOLINTNEXTLINE(readability-const-return-type)
inline const typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::get_scope_guard() const noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) const_cast to avoid code duplication
    return const_cast<smart_lock<T, MutexType>*>(this)->get_scope_guard();
}

template <typename T, typename MutexType>
inline typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::getScopeGuard() noexcept
{
    return get_scope_guard();
}

template <typename T, typename MutexType>
// const return type improves const correctness, operator-> can be chained with underlying operator->
// NOLINTNEXTLINE(readability-const-return-type)
inline const typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::getScopeGuard() const noexcept
{
    return get_scope_guard();
}

template <typename T, typename MutexType>
inline T smart_lock<T, MutexType>::get_copy() const noexcept
{
    std::lock_guard<MutexType> guard(lock);
    return base;
}

template <typename T, typename MutexType>
inline T smart_lock<T, MutexType>::getCopy() const noexcept
{
    return get_copy();
}

// PROXY OBJECT

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>::Proxy::Proxy(T& base, MutexType& lock) noexcept
    : base(base)
    , lock(lock)
{
    lock.lock();
}

template <typename T, typename MutexType>
inline smart_lock<T, MutexType>::Proxy::~Proxy() noexcept
{
    lock.unlock();
}

template <typename T, typename MutexType>
inline T* smart_lock<T, MutexType>::Proxy::operator->() noexcept
{
    return &base;
}

template <typename T, typename MutexType>
inline const T* smart_lock<T, MutexType>::Proxy::operator->() const noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) const_cast to avoid code duplication
    return const_cast<smart_lock<T, MutexType>::Proxy*>(this)->operator->();
}

template <typename T, typename MutexType>
inline T& smart_lock<T, MutexType>::Proxy::operator*() noexcept
{
    return base;
}

template <typename T, typename MutexType>
inline const T& smart_lock<T, MutexType>::Proxy::operator*() const noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) const_cast to avoid code duplication
    return const_cast<smart_lock<T, MutexType>::Proxy*>(this)->operator*();
}
} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_SYNC_SMART_LOCK_INL
