#pragma once

#include <atomic>
#include <mutex>
#include <type_traits>

namespace eh
{
// store some unique instance derived from (or equal to) Interface
//
// the default value is a default constructed Default which must inherit from Interface
// Invariant: always has an instance (statically initialized)

// NB: we cannot derive from interface and specify default like this (is there a way
// that does not break other useful properties like well-defined default?)
// This is a generic construct independent of error handling.
template <typename Interface, typename Default>
class PolymorphicSingleton
{
    static_assert(std::is_base_of<Interface, Default>::value);

  public:
    using Self = PolymorphicSingleton<Interface, Default>;

    static Interface& get()
    {
        return *s_current.load(std::memory_order_relaxed);
    }

    // we always require a valid handler to be set
    // enforce contract of valid handler
    static Interface* set(Interface& handler)
    {
        auto& s = instance();
        std::lock_guard<std::mutex> lock(s.m_mutex);
        if (s.m_isFinal)
        {
            std::cerr << "SETTING AFTER FINALIZE IS NOT ALLOWED!" << std::endl;
            std::terminate();
            return nullptr;
        }

        return s_current.exchange(&handler, std::memory_order_relaxed);
    }

    static Interface* reset()
    {
        return set(s_default);
    }

    static void finalize()
    {
        auto& s = instance();
        std::lock_guard<std::mutex> lock(s.m_mutex);
        s.m_isFinal.store(true);
    }

  private:
    static Default s_default; // ensure there is always at least a default
    // not a member to avoid indirection - we may have indirection in instance anyway
    static std::atomic<Interface*> s_current;

    // avoid lots of static variables, these are not accessed often (set and finalize are rare)
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex; // required for sync only

    // there can be only one but it cannot be accessed publicly
    static PolymorphicSingleton& instance()
    {
        static PolymorphicSingleton s;
        return s;
    }
};

// these are initialized before any static method (set, get etc.) is called(!)
template <typename I, typename D>
D PolymorphicSingleton<I, D>::s_default{};

template <typename I, typename D>
std::atomic<I*> PolymorphicSingleton<I, D>::s_current{&s_default};

// this special case will require a non-abstract Base that may or may not have virtual interface
// if it does, it will be able to store any derived type of Base,
// otherwise it can only store instances of Base
template <typename Base>
using MultimorphicSingleton = PolymorphicSingleton<Base, Base>;

} // namespace eh
