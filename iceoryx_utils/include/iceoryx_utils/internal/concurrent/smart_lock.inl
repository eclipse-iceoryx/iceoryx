// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"

namespace iox
{
namespace concurrent
{
template <typename T, typename MutexType, typename... Targs>
smart_lock<T, MutexType> make_smart_lock(Targs&&... args)
{
    return smart_lock<T, MutexType>(T(std::forward<Targs>(args)...));
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::smart_lock()
{
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::smart_lock(const T& t)
    : base(t)
{
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::smart_lock(const smart_lock& rhs)
{
    std::lock_guard<std::mutex> guard(rhs.lock);
    base = rhs.base;
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::smart_lock(smart_lock&& rhs)
{
    std::lock_guard<std::mutex> guard(rhs.lock);
    base = std::move(rhs.base);
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::~smart_lock()
{
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>& smart_lock<T, MutexType>::operator=(const smart_lock& rhs)
{
    std::lock(lock, rhs.lock);
    std::lock_guard<std::mutex> guard(lock, std::adopt_lock);
    std::lock_guard<std::mutex> guardRhs(rhs.lock, std::adopt_lock);
    base = rhs.base;
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>& smart_lock<T, MutexType>::operator=(smart_lock&& rhs)
{
    std::lock(lock, rhs.lock);
    std::lock_guard<std::mutex> guard(lock, std::adopt_lock);
    std::lock_guard<std::mutex> guardRhs(rhs.lock, std::adopt_lock);
    base = std::move(rhs.base);
}

// PROXY OBJECT

template <typename T, typename MutexType>
smart_lock<T, MutexType>::Proxy::Proxy(T* base, MutexType* lock)
    : base(base)
    , lock(lock)
{
    lock->lock();
}

template <typename T, typename MutexType>
smart_lock<T, MutexType>::Proxy::~Proxy()
{
    lock->unlock();
}

template <typename T, typename MutexType>
T* smart_lock<T, MutexType>::Proxy::operator->()
{
    return base;
}

template <typename T, typename MutexType>
typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::operator->()
{
    return Proxy(&base, &lock);
}

template <typename T, typename MutexType>
typename smart_lock<T, MutexType>::Proxy smart_lock<T, MutexType>::GetScopeGuard()
{
    return Proxy(&base, &lock);
}


} // namespace concurrent
} // namespace iox
