#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>

// TODO: change namespace? but DesignPattern is a bad namespace name ...
namespace iox
{
namespace design
{

// TODO: namespace of this concept? should not depend on templated class (e.g. PolymorphicHandler)
// it is not internal as it is supposed to be used to design new Handlers

/// @brief Implements the Activatable concept to be used in the PolymorphicHandler
///        The concept implements a binary switch. By default is switched on (active).
class Activatable
{
  public:
    Activatable() = default;

    /// @brief Switch on.
    void activate()
    {
        m_active = true;
    }

    /// @brief Switch off.
    void deactivate()
    {
        m_active = false;
    }

    /// @brief Query switch state.
    /// @return true if active (on), false otherwise (off).
    bool isActive() const
    {
        return m_active;
    }

  private:
    bool m_active{true};
};

namespace detail
{

///@brief default hooks for the PolymorphicHandler
///@note template hooks to avoid forced virtual inheritance (STL approach),
template <typename Interface>
struct DefaultHooks
{
    /// @brief called after if the polymorphic handler is set or reset after finalize
    static void onSetAfterFinalize(Interface& /*currentInstance*/, Interface& /*newInstance*/)
    {
        std::cerr << "setting the polymorphic handler after finalize is not allowed" << std::endl;
        std::terminate();
    }
};

} // namespace detail

/// @brief Implements a singleton handler that has a default instance and can be changed
///        to another instance at runtime. All instances have to derive from the same interface.
///        The singleton handler owns the default instance but all other instances are created externally.
/// @tparam Interface The interface of the handler instances. Must inherit from Activatable.
/// @tparam Default The type of the default instance. Must be equal to or derive from Interface.
///
/// @note In the special case where Default equals Interface, no polymorphism is required.
///       It is then possible to e.g. switch between multiple instances of Default type.
/// @note The lifetime of external non-default instances must exceed the lifetime of the PolymorphicHandler.
/// @note The PolymorphicHandler is guaranteed to provide a valid handler during the whole program lifetime (static).
///       It is hence not advisable to have other static variables depend on the PolymorphicHandler.
///       It must be ensured that the are destroyed before the PolymorphicHandler.
/// @note Hooks must implement
///       static void onSetAfterFinalize(Interface& /*currentInstance*/, Interface& /*newInstance*/).
template <typename Interface, typename Default, typename Hooks = detail::DefaultHooks<Interface>>
class PolymorphicHandler
{
    static_assert(std::is_base_of<Interface, Default>::value, "Default must inherit from Interface");

    // actually it suffices to provide the methods activate, deactivate, isActive
    // but they need to behave correctly and inheritance enforces this
    static_assert(std::is_base_of<Activatable, Interface>::value, "Interface must inherit from Activatable");

  public:
    /// @brief get the current singleton instance
    /// @return the current instance
    static Interface& get();

    /// @brief set the current singleton instance
    /// @param handler the handler instance to be set
    /// @return pointer to the previous instance
    static Interface* set(Interface& handler);

    /// @brief reset the current singleton instance to the default instance
    /// @return pointer to the previous instance
    static Interface* reset();

    /// @brief finalizes the instance, afterwards Hooks::onSetAfterFinalize
    ///        will be called during the remaining program lifetime
    static void finalize();

  private:
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex;
    std::atomic<Interface*> m_current;

    PolymorphicHandler();

    static PolymorphicHandler& instance();

    static Default& getDefault();

    static Interface* getCurrent();
};

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

template <typename Interface, typename Default, typename Hooks>
Interface& PolymorphicHandler<Interface, Default, Hooks>::get()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) false positive, thread_local
    thread_local Interface* localHandler = getCurrent(); // initialized once per thread on first call

    if (!localHandler->isActive())
    {
        std::lock_guard<std::mutex> lock(instance().m_mutex);
        localHandler = getCurrent();
    }
    return *localHandler;
}

template <typename Interface, typename Default, typename Hooks>
Interface* PolymorphicHandler<Interface, Default, Hooks>::set(Interface& handler)
{
    auto& ins = instance();

    // setting is rare and we lock to synchronize and update the active flags properly
    std::lock_guard<std::mutex> lock(ins.m_mutex);
    if (ins.m_isFinal)
    {
        Hooks::onSetAfterFinalize(*getCurrent(), handler);
        return nullptr;
    }

    handler.activate(); // it may have been deactivated before, so always reactivate it
    auto prev = ins.m_current.exchange(&handler, std::memory_order_relaxed);

    // anyone still using it will eventually see that it is inactive
    // and switch to the new handler
    prev->deactivate();
    return prev;
}

template <typename Interface, typename Default, typename Hooks>
Interface* PolymorphicHandler<Interface, Default, Hooks>::reset()
{
    return set(getDefault());
}

template <typename Interface, typename Default, typename Hooks>
void PolymorphicHandler<Interface, Default, Hooks>::finalize()
{
    auto& ins = instance();
    std::lock_guard<std::mutex> lock(ins.m_mutex);
    ins.m_isFinal.store(true);
}

template <typename Interface, typename Default, typename Hooks>
PolymorphicHandler<Interface, Default, Hooks>::PolymorphicHandler()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    //  this is set the first time we call instance()
    m_current.store(&getDefault(), std::memory_order_release);
}

template <typename Interface, typename Default, typename Hooks>
PolymorphicHandler<Interface, Default, Hooks>& PolymorphicHandler<Interface, Default, Hooks>::instance()
{
    static PolymorphicHandler handler;
    return handler;
}

template <typename Interface, typename Default, typename Hooks>
Default& PolymorphicHandler<Interface, Default, Hooks>::getDefault()
{
    static Default defaultHandler;
    return defaultHandler;
}

template <typename Interface, typename Default, typename Hooks>
Interface* PolymorphicHandler<Interface, Default, Hooks>::getCurrent()
{
    auto& ins = instance();
    return ins.m_current.load(
        std::memory_order_acquire); // must be strong enough to sync memory of the object pointed to
}

} // namespace design
} // namespace iox