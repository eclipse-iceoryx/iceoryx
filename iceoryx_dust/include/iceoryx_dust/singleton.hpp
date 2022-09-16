#pragma once

// TODO: decide location

#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>

namespace iox
{

template <typename T>
class Singleton
{
  public:
    template <typename... Args>
    static void init(Args&&... args)
    {
        std::lock_guard<std::mutex> g(lock());
        initialize(std::forward<Args>(args)...);
    }

    // must be called if it is clear no one uses the singleton anymore
    // could guard against it, but this slows down get()
    static void destroy()
    {
        std::lock_guard<std::mutex> g(lock());
        std::cout << "destroy" << std::endl;
        auto p = ptr().load();
        if (p)
        {
            std::cout << "dtor" << std::endl;
            p->~T();
            ptr().store(nullptr);
        }
    }

    // undefined behavior if called after destroy
    // - could check but this requires an additional atomic flag/enum and is not a
    // good design
    // - similar probelms exist with singletons that only auto destruct (design
    // must ensure they are not accessed after destruction)
    // - destroy allows better control of destruction
    static T& get()
    {
        // need to sync the memory at *p as well
        auto p = ptr().load(std::memory_order_acquire);
        if (!p)
        {
            std::lock_guard<std::mutex> g(lock());
            // could have been initialized in the meantime,
            // so we double check
            auto p = ptr().load();
            if (p)
            {
                return *p;
            }

            p = initialize(); // lazy default initialization
            ptr().store(std::memory_order_release);
            return *p;
        }
        return *p;
    }

    // destroy only calls the dtor of T if it is was not destroyed before
    ~Singleton()
    {
        destroy();
    }

  private:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;

    using storage_t = typename std::aligned_storage_t<sizeof(T), alignof(T)>::type;

    // avoid the external definitions of regular statics and use lazy init
    static auto& storage()
    {
        static storage_t s;
        return s;
    }

    static auto& ptr()
    {
        static std::atomic<T*> p;
        return p;
    }

    static auto& lock()
    {
        static std::mutex m;
        return m;
    }

    template <typename... Args>
    static T* initialize(Args&&... args)
    {
        std::cout << "init" << std::endl;
        static Singleton singleton; // dtor will be called later and call destroy
        std::cout << "ctor" << std::endl;
        auto p = new (&storage()) T(std::forward<Args>(args)...);
        ptr().store(p);
        return p;
    }
};

// concurrent get: exactly one wins,locks, initializes and syncs pointer
//
// concurrent get and init: one wins and syncs pointer
//
// concurrent init and destroy: no problem but undefined outcome (object alive
// or not)
//
// concurrent get and destroy: not allowed, same with regular singleton dtor and
// access in e.g. another static

} // namespace iox