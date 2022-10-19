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
/// @note dtor, ctor and copy ctor are thread-safe but instance()
/// intentionally is not as it is supposed to be called in
/// the static initialization phase or in a thread-safe context (e.g. with a mutex).
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
        ++s_count;
    }

    StaticLifetimeGuard(const StaticLifetimeGuard&) noexcept
    {
        ++s_count;
    }

    // move and assignment have no purpose as the objects themselves
    // have no state and no side effects are required
    // (copy exists to support passing/returning a value object)
    StaticLifetimeGuard(StaticLifetimeGuard&&) = delete;
    StaticLifetimeGuard& operator=(const StaticLifetimeGuard&) = delete;
    StaticLifetimeGuard& operator=(StaticLifetimeGuard&&) = delete;

    ~StaticLifetimeGuard() noexcept
    {
        if (--s_count == 0)
        {
            destroy();
        }
    }

    /// @brief Construct the instance to be guarded with constructor arguments.
    /// @param args constructor arguments
    /// @return reference to the constructed instance or the existing instance
    /// if it already exists
    /// @note creates an implicit static StaticLifetimeGuard to ensure
    /// the instance is destroyed in the static destruction phase
    /// @note NOT thread-safe on its own on purpose as the first call should be used
    /// for a static or in a context that handles thread-safety on its own
    template <typename... Args>
    static T& instance(Args&&... args) noexcept
    {
        static StaticLifetimeGuard guard;
        if (s_count == 1)
        {
            // NOLINTJUSTIFICATION s_instance is managed by reference counting
            // NOLINTNEXTLINE (cppcoreguidelines-owning-memory)
            s_instance = new (&s_storage) T(std::forward<Args>(args)...);

            // synchronize s_instance
            // (store only is bad as concurrent construction of guards is allowed)
            ++s_count;
            --s_count;
        }
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

    // NOLINTJUSTIFICATION static variables are private and mutability is required
    // NOLINTBEGIN (cppcoreguidelines-avoid-non-const-global-variables)

    static storage_t s_storage;
    static std::atomic<uint64_t> s_count;
    // actually the pointer would not be needed (we can use the counter as indicator),
    // but simpler and more clear this way
    static T* s_instance;

    static void destroy()
    {
        if (s_instance)
        {
            s_instance->~T();
            s_instance = nullptr;
        }
    }
};

template <typename T>
typename StaticLifetimeGuard<T>::storage_t StaticLifetimeGuard<T>::s_storage;
template <typename T>
std::atomic<uint64_t> StaticLifetimeGuard<T>::s_count{0};
template <typename T>
T* StaticLifetimeGuard<T>::s_instance{nullptr};

// NOLINTEND (cppcoreguidelines-avoid-non-const-global-variables)

} // namespace design_pattern
} // namespace iox