// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_DUST_SINGLETON_INL
#define IOX_DUST_SINGLETON_INL

#include "iceoryx_dust/singleton.hpp"

namespace iox
{

template <typename T>
bool Singleton<T>::isInitialized()
{
    return ptr().load(std::memory_order_relaxed) != nullptr;
}

template <typename T>
template <typename... Args>
T& Singleton<T>::init(Args&&... args)
{
    std::lock_guard<std::mutex> g(lock());
    if (!isInitialized())
    {
        // initialized by this call
        return *initialize(std::forward<Args>(args)...);
    }
    // initialized before by some other call
    return *ptr().load(std::memory_order_acquire);
}

template <typename T>
void Singleton<T>::destroy()
{
    std::lock_guard<std::mutex> g(lock());
    auto p = ptr().load(std::memory_order_acquire);
    if (p)
    {
        p->~T();
        ptr().store(nullptr);
    }
}

template <typename T>
T& Singleton<T>::instance()
{
    // need to sync the memory at *p as well
    auto p = ptr().load(std::memory_order_acquire);
    if (!p)
    {
        std::lock_guard<std::mutex> g(lock());
        // could have been initialized in the meantime,
        // so we double check under lock
        p = ptr().load();
        if (p)
        {
            return *p;
        }

        p = initialize(); // lazy default initialization
        ptr().store(p, std::memory_order_release);

        // was initialized and stays initialized until destroy
        return *p;
    }
    return *p;
}

template <typename T>
Singleton<T>::~Singleton()
{
    destroy();
}

template <typename T>
auto& Singleton<T>::storage()
{
    static storage_t s;
    return s;
}

template <typename T>
auto& Singleton<T>::ptr()
{
    static std::atomic<T*> p;
    return p;
}

template <typename T>
auto& Singleton<T>::lock()
{
    static std::mutex m;
    return m;
}

template <typename T>
template <typename... Args>
T* Singleton<T>::initialize(Args&&... args)
{
    static Singleton singleton; // dtor will be called later and call destroy
    // NOLINTJUSTIFICATION implicit conversion from raw pointer is intentional in design of relocatable structures
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) T dtor will be called by singleton dtor
    auto p = new (&storage()) T(std::forward<Args>(args)...);
    ptr().store(p, std::memory_order_relaxed); // memory synced by lock
    return p;
}

} // namespace iox

#endif
