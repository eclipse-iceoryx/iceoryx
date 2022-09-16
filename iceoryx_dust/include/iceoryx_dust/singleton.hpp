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

#ifndef IOX_DUST_SINGLETON_HPP
#define IOX_DUST_SINGLETON_HPP

// TODO: decide file location

#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>

namespace iox
{

/// @brief Generic class that allows accessing another class like a singleton.
///        The singleton instance is either initialized explictly or lazily default initalized on first access.
///        It is possible to destroy the singleton instance explictly (call the dtor), but it must be ensured
///        that it is not accessed anymore. Reinitialization is only possible after destruction and should be used
///        carefully (ensure instance is not used during reinitialization).
///        Initialization and destruction is thread-safe.
/// @tparam T type of the singleton instance
///
/// @note While Singleton<T> allows using T as a singleton instance, it cannot prevent other T instances to be
/// constructed explictly.
template <typename T>
class Singleton
{
  public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    ~Singleton();

    /// @brief check whether the singleton instance is initialized
    /// @return true if the singleton instance is initialized, false otherwise
    /// @note thread-safe
    static bool isInitialized();

    /// @brief construct the singleton instance if not already initialized
    /// @param args consctructor arguments of the singleton instance
    /// @return reference to the constructed or previously existing singleton instance
    /// @note thread-safe
    template <typename... Args>
    static T& init(Args&&... args);

    /// @brief explicitly destroy the singleton instance if it is initialized
    /// @note thread-safe
    /// @note must ONLY be called if the instance is no longer accessed
    ///       (this is not a severe restriction, as it is true for standard static singletons as well)
    /// @note benefit: better control of the life-time of a singleton
    /// @note if destroy is not explictly called, it will be called implictly after main,
    ///       the usual rules (and problems) of static destruction order apply
    static void destroy();

    /// @brief explicitly destroy the singleton instance if it is initialized
    /// @return reference to the default constructed or previously existing singleton instance
    /// @note thread-safe wrt. all functions except for destroy
    /// @note accessing the returned instance is undefined behavior if called concurrently with destroy
    static T& instance();

  private:
    using storage_t = typename std::aligned_storage_t<sizeof(T), alignof(T)>::type;

    Singleton() = default;

    // avoid the external definitions of regular statics (especially with templates)
    // and use lazy initialization
    static auto& storage();

    static auto& ptr();

    static auto& lock();

    template <typename... Args>
    static T* initialize(Args&&... args);
};

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
        auto p = ptr().load();
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

#endif // IOX_DUST_SINGLETON_HPP