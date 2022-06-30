#pragma once

#include <atomic>
#include <mutex>

namespace eh
{
// store some unique instance derived from (or equal to) Interface
// the default value is a default constructed Default which must inherit from Interface
// Invariant: always has a instance

// NB: we cannot derive from interface and specify default like this (is there a way
// that does not break other useful properties like well-defined default?)
// This is a generic construct independent of error handling.
template <typename Interface, typename Default>
class Unique
{
  public:
    using Self = Unique<Interface, Default>;

    static Interface& get()
    {
        return *s_current.load(std::memory_order_relaxed);
    }

    // we always require a valid handler to be set
    // enforce contract of valid handler
    static Interface* set(Interface& handler)
    {
        auto& u = instance();
        std::lock_guard<std::mutex> lock(u.m_mutex);
        if (u.m_isFinal)
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
        auto& u = instance();
        std::lock_guard<std::mutex> lock(u.m_mutex);
        u.m_isFinal.store(true);
    }

  private:
    static Default s_default; // ensure there is always at least a default
    // not a member to avoid indirection - we may have indirection in instance anyway
    static std::atomic<Interface*> s_current;

    // avoid lots of static variables, these are not accessed often (set and finalize are rare)
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex; // required for sync only

    // there can be only one but it cannot be accessed publicly
    static Unique& instance()
    {
        static Unique u;
        return u;
    }
};

// these are initialized before any static method (set, get etc.) is called(!)
template <typename I, typename D>
D Unique<I, D>::s_default{};

template <typename I, typename D>
std::atomic<I*> Unique<I, D>::s_current{&s_default};

} // namespace eh
