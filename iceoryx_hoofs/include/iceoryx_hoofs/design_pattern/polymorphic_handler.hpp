#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>

// TODO: change namespace? but DesignPattern is a bad namespace name ...
namespace iox
{

struct Activatable
{
    Activatable() = default;

    void activate()
    {
        m_active = true;
    }

    void deactivate()
    {
        m_active = false;
    }

    bool isActive() const
    {
        return m_active;
    }

  private:
    bool m_active = true;
};

// we lose generality now wrt. the interface (require activate, deactivate etc. in the interface)
template <typename Interface, typename Default>
class PolymorphicHandler
{
    static_assert(std::is_base_of<Interface, Default>::value, "Default must inherit from Interface");

    // actually it suffices to provide the methods activate, deactivate, isActive
    static_assert(std::is_base_of<Activatable, Interface>::value, "Interface must inherit from Activatable");

  public:
    using Self = PolymorphicHandler<Interface, Default>;

    // on first call (in a thread):
    // 1. localHandler is initialized
    //    - getCurrent is called
    //    - instantiates singleton instance()
    //    - instantiates default
    //    - sets m_current of instance to default instance (release)
    //    - default is active

    // if any thread changes the active handler with set (or reset)
    // under mutex protection, it will:
    //    - set the new handler to active
    //    - set current handler to the new handler
    //    - deactivate the old handler (can still be used as it still needs to exist)
    //

    // on any call after the handler was changes in another thread
    // 1. we check whether the handler is active
    // (can be outdated information but will eventually be false once the atomic value is updated)
    // 2. if it was changed it is now inactive and we wait to obtain the mutex lock
    // under lock, we update the local handler to the new one (note that it cannot change
    // while this happens as we hold the lock)

    static Interface& get()
    {
        thread_local Interface* localHandler = getCurrent(); // initialized once per thread on first call

        if (!localHandler->isActive())
        {
            std::lock_guard<std::mutex> lock(instance().m_mutex);
            localHandler = getCurrent();
        }
        return *localHandler;
    }

    static Interface* set(Interface& handler)
    {
        auto& ins = instance();

        // setting is rare and we lock to synchronize and update the active flags properly
        std::lock_guard<std::mutex> lock(ins.m_mutex);
        if (ins.m_isFinal)
        {
            std::cerr << "setting the polymorphic handler after finalize is not allowed" << std::endl;
            std::terminate();
            return nullptr;
        }

        handler.activate(); // it may have been deactivated before, so always reactivate it
        auto prev = ins.m_current.exchange(&handler, std::memory_order_relaxed);

        // anyone still using it will eventually see that it is inactive
        // and switch to the new handler
        prev->deactivate();
        return prev;
    }

    static Interface* reset()
    {
        return set(getDefault());
    }

    static void finalize()
    {
        auto& ins = instance();
        std::lock_guard<std::mutex> lock(ins.m_mutex);
        ins.m_isFinal.store(true);
    }

  private:
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex;
    std::atomic<Interface*> m_current;

    PolymorphicHandler()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        //  this is set the first time we call instance()
        m_current.store(&getDefault(), std::memory_order_release);
    }

    static PolymorphicHandler& instance()
    {
        static PolymorphicHandler handler;
        return handler;
    }

    // the assumption is that this class manages the default but not any
    // other that could be set (which must be created statically before being set)
    static Default& getDefault()
    {
        static Default defaultHandler;
        return defaultHandler;
    }

    static Interface* getCurrent()
    {
        auto& ins = instance();
        return ins.m_current.load(
            std::memory_order_acquire); // must be strong enough to sync memory of the object pointed to
    }
};

} // namespace eh