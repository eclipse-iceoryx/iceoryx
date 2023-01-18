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

#ifndef IOX_HOOFS_DESIGN_STATIC_LIFETIME_GUARD_INL
#define IOX_HOOFS_DESIGN_STATIC_LIFETIME_GUARD_INL

#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"

#include <atomic>
#include <thread>

namespace iox
{
namespace design_pattern
{

// NOLINTJUSTIFICATION these static variables are private and mutability is required
// NOLINTBEGIN (cppcoreguidelines-avoid-non-const-global-variables)
template <typename T>
typename StaticLifetimeGuard<T>::storage_t StaticLifetimeGuard<T>::s_storage;
template <typename T>
std::atomic<uint64_t> StaticLifetimeGuard<T>::s_count{0};
template <typename T>
std::atomic<uint32_t> StaticLifetimeGuard<T>::s_instanceState{UNINITIALIZED};
template <typename T>
T* StaticLifetimeGuard<T>::s_instance{nullptr};
// NOLINTEND (cppcoreguidelines-avoid-non-const-global-variables)

template <typename T>
StaticLifetimeGuard<T>::StaticLifetimeGuard() noexcept
{
    s_count.fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
StaticLifetimeGuard<T>::StaticLifetimeGuard(const StaticLifetimeGuard&) noexcept
{
    s_count.fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
StaticLifetimeGuard<T>::StaticLifetimeGuard(StaticLifetimeGuard&&) noexcept
{
    // we have to increment the counter here as well as it is only
    // decremented in the dtor (which was not yet called for the moved object)
    s_count.fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
StaticLifetimeGuard<T>::~StaticLifetimeGuard() noexcept
{
    if (s_count.fetch_sub(1, std::memory_order_relaxed) == 1)
    {
        destroy();
    }
}

template <typename T>
template <typename... Args>
T& StaticLifetimeGuard<T>::instance(Args&&... args) noexcept
{
    static StaticLifetimeGuard<T> guard;

    // we determine wether this call has to initialize the instance
    // via CAS (without mutex!)
    // NB: this shows how CAS acts as consensus primitive to determine the initializing call
    uint32_t exp{UNINITIALIZED};
    if (s_instanceState.compare_exchange_strong(exp, INITALIZING, std::memory_order_acq_rel, std::memory_order_acquire))
    {
        // NOLINTJUSTIFICATION s_instance is managed by reference counting
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        s_instance = new (&s_storage) T(std::forward<Args>(args)...);

        // synchronize s_instance
        s_instanceState.store(INITALIZED, std::memory_order_release);
        return *s_instance;
    }
    // design constraint: no mutex - makes it more intricate
    // (otherwise we could simply use double checked locking)

    // either this call initialized the instance and we already returned
    // or a concurrent call is doing so and we have to wait until it completes ...

    while (s_instanceState.load(std::memory_order_acquire) != INITALIZED)
    {
        // wait, guaranteed to complete with fair scheduling
        std::this_thread::yield();
    }
    // guaranteed to be non-null after initialization
    return *s_instance;
}

template <typename T>
uint64_t StaticLifetimeGuard<T>::setCount(uint64_t count)
{
    return s_count.exchange(count, std::memory_order_relaxed);
}

template <typename T>
uint64_t StaticLifetimeGuard<T>::count()
{
    return s_count.load(std::memory_order_relaxed);
}

template <typename T>
void StaticLifetimeGuard<T>::destroy()
{
    if (s_instance)
    {
        s_instance->~T();
        s_instance = nullptr;
        s_instanceState = UNINITIALIZED;
    }
}


} // namespace design_pattern
} // namespace iox

#endif // IOX_HOOFS_DESIGN_STATIC_LIFETIME_GUARD_INL
