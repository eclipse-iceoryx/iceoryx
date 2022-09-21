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

#ifndef IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_INL
#define IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_INL

#include "iceoryx_hoofs/design_pattern/polymorphic_handler.hpp"

namespace iox
{
namespace design_pattern
{

namespace detail
{

template <typename Interface>
void DefaultHooks<Interface>::onSetAfterFinalize(Interface&, Interface&) noexcept
{
    // we should not use an error handling construct (e.g. some IOX_ASSERT) here for dependency reasons
    // we could in principle do nothing by default as well, but the misuse failure should have visible consequences
    std::terminate();
}

} // namespace detail

void Activatable::activate() noexcept
{
    m_active = true;
}

void Activatable::deactivate() noexcept
{
    m_active = false;
}

bool Activatable::isActive() const noexcept
{
    return m_active;
}

// The get method is considered to be on the hot path and works as follows:
//
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
Interface& PolymorphicHandler<Interface, Default, Hooks>::get() noexcept
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

// can throw
template <typename Interface, typename Default, typename Hooks>
Interface* PolymorphicHandler<Interface, Default, Hooks>::set(Interface& handler) noexcept
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
Interface* PolymorphicHandler<Interface, Default, Hooks>::reset() noexcept
{
    return set(getDefault());
}

template <typename Interface, typename Default, typename Hooks>
void PolymorphicHandler<Interface, Default, Hooks>::finalize() noexcept
{
    auto& ins = instance();
    std::lock_guard<std::mutex> lock(ins.m_mutex);
    ins.m_isFinal.store(true);
}

template <typename Interface, typename Default, typename Hooks>
PolymorphicHandler<Interface, Default, Hooks>::PolymorphicHandler() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    //  this is set the first time we call instance()
    m_current.store(&getDefault(), std::memory_order_release);
}

template <typename Interface, typename Default, typename Hooks>
PolymorphicHandler<Interface, Default, Hooks>& PolymorphicHandler<Interface, Default, Hooks>::instance() noexcept
{
    static PolymorphicHandler handler;
    return handler;
}

template <typename Interface, typename Default, typename Hooks>
Default& PolymorphicHandler<Interface, Default, Hooks>::getDefault() noexcept
{
    static Default defaultHandler;
    return defaultHandler;
}

template <typename Interface, typename Default, typename Hooks>
Interface* PolymorphicHandler<Interface, Default, Hooks>::getCurrent() noexcept
{
    auto& ins = instance();
    return ins.m_current.load(
        std::memory_order_acquire); // must be strong enough to sync memory of the object pointed to
}

} // namespace design_pattern
} // namespace iox

#endif // IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_INL