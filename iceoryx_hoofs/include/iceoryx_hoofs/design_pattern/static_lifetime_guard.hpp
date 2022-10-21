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

#pragma once

#include <atomic>
#include <iostream>
#include <type_traits>

namespace iox
{
namespace design_pattern
{

/// @todo move to inl if we want to keep the construction

/// @brief Manages a static instance of type T in a way so that each
/// existing StaticLifetimeGuard prevents the destruction of
/// the instance.
/// 1. instance() creates a static guard itself and hence has static lifetime
/// 2. any static StaticLifetimeGuard G created before that prolongs the lifetime
/// of the instance at least until G is destroyed
/// @tparam T the type of the instance to be guarded
///
/// @note dtor, ctor and copy ctor and instance are thread-safe
///
/// @code
/// // instance will be destroyed after guard
/// // (or later if there are guards preceding guard in construction order)
///
/// static StaticLifetimeGuard<T> guard;
/// static T& instance = StaticLifetimeGuard<T>::instance();
/// @endcode
template <typename T>
class StaticLifetimeGuard
{
  public:
    StaticLifetimeGuard() noexcept
    {
        std::cerr << "GUARD " << typeid(T).hash_code() << std::endl;
        ++s_count;
    }

    StaticLifetimeGuard(const StaticLifetimeGuard&) noexcept
    {
        ++s_count;
    }

    // move and assignment have no purpose,
    // copy exists to support passing/returning a value object
    StaticLifetimeGuard(StaticLifetimeGuard&&) = delete;
    StaticLifetimeGuard& operator=(const StaticLifetimeGuard&) = delete;
    StaticLifetimeGuard& operator=(StaticLifetimeGuard&&) = delete;

    ~StaticLifetimeGuard() noexcept
    {
        if (s_count.fetch_sub(1) == 1)
        {
            std::cerr << "UNGUARD " << typeid(T).hash_code() << std::endl;
            destroy();
        }
    }

    /// @brief Construct the instance to be guarded with constructor arguments.
    /// @param args constructor arguments
    /// @return reference to the constructed instance or the existing instance
    /// if it already exists
    /// @note creates an implicit static StaticLifetimeGuard to ensure
    /// the instance is destroyed in the static destruction phase
    template <typename... Args>
    static T& instance(Args&&... args) noexcept
    {
        static StaticLifetimeGuard guard;

        // we determine wether this call has to initialize the instance
        // via CAS (without mutex!)
        // NB: this shows how CAS acts as consensus primitive
        // will only be initialized once
        uint32_t exp{UNINITIALIZED};
        if (s_instanceState.compare_exchange_strong(
                exp, INITALIZING, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            std::cerr << "CREATE INSTANCE " << typeid(T).hash_code() << std::endl;
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
            // wait
        }
        // guaranteed to be non-null after initialization
        return *s_instance;
    }

    /// @brief Set the instance life time count.
    /// @param count value to be set
    /// @return previous count value
    /// @note This can be used to additionally extend or shorten the instance lifetime,
    /// This has to be done carefully, to ensure destruction or prevent early destruction.
    /// It is useful for testing purposes.
    static uint64_t setCount(uint64_t count)
    {
        return s_count.exchange(count);
    }

    /// @brief Get the current count value.
    /// @return current count value
    static uint64_t count()
    {
        return s_count.load(std::memory_order_relaxed);
    }

  private:
    using storage_t = typename std::aligned_storage_t<sizeof(T), alignof(T)>;

    static constexpr uint32_t UNINITIALIZED{0};
    static constexpr uint32_t INITALIZING{1};
    static constexpr uint32_t INITALIZED{2};

    // NOLINTJUSTIFICATION static variables are private and mutability is required
    // NOLINTBEGIN (cppcoreguidelines-avoid-non-const-global-variables)
    static storage_t s_storage;
    static std::atomic<uint64_t> s_count;
    static std::atomic<uint32_t> s_instanceState;
    static T* s_instance;

    std::atomic<uint64_t> m_value{1};

    static void destroy()
    {
        if (s_instance)
        {
            s_instance->~T();
            s_instance = nullptr;
            s_instanceState = 0;
        }
    }
};

template <typename T>
typename StaticLifetimeGuard<T>::storage_t StaticLifetimeGuard<T>::s_storage;
template <typename T>
std::atomic<uint64_t> StaticLifetimeGuard<T>::s_count{0};
template <typename T>
std::atomic<uint32_t> StaticLifetimeGuard<T>::s_instanceState{UNINITIALIZED};
template <typename T>
T* StaticLifetimeGuard<T>::s_instance{nullptr};

// NOLINTEND (cppcoreguidelines-avoid-non-const-global-variables)

} // namespace design_pattern
} // namespace iox