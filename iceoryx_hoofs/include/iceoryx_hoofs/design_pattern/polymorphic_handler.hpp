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

#ifndef IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_HPP
#define IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_HPP

#include <atomic>
#include <iostream>
#include <mutex>
#include <type_traits>

namespace iox
{
/// @todo: change namespace name for consistency?
namespace design_pattern
{

/// @brief Implements the Activatable concept to be used in the PolymorphicHandler
/// The concept implements a binary switch. By default is switched on (active).
/// Anyone defining another custom handler interface is supposed to derive from Activatable.
/// @note While this is public, it is also partially implementation detail and partially convenience
/// to use the PolymorphicHandler.
class Activatable
{
  public:
    Activatable() = default;

    /// @brief Switch on.
    void activate() noexcept;

    /// @brief Switch off.
    void deactivate() noexcept;

    /// @brief Query switch state.
    /// @return true if active (on), false otherwise (off).
    bool isActive() const noexcept;

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
    /// @param currentInstance the current instance of the handler singleton
    /// @param newInstance the instance of the handler singleton to be set
    static void onSetAfterFinalize(Interface& currentInstance, Interface& newInstance) noexcept;
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
    static Interface& get() noexcept;

    /// @brief set the current singleton instance
    /// @param handler the handler instance to be set
    /// @return pointer to the previous instance
    static Interface* set(Interface& handler) noexcept;

    /// @brief reset the current singleton instance to the default instance
    /// @return pointer to the previous instance
    static Interface* reset() noexcept;

    /// @brief finalizes the instance, afterwards Hooks::onSetAfterFinalize
    ///        will be called during the remaining program lifetime
    static void finalize() noexcept;

  private:
    std::atomic_bool m_isFinal{false};
    std::mutex m_mutex;
    std::atomic<Interface*> m_current;

    PolymorphicHandler() noexcept;

    static PolymorphicHandler& instance() noexcept;

    static Default& getDefault() noexcept;

    static Interface* getCurrent() noexcept;
};

} // namespace design_pattern
} // namespace iox

#include "iceoryx_hoofs/internal/design_pattern/polymorphic_handler.inl"

#endif // IOX_HOOFS_DESIGN_PATTERN_POLYMORPHIC_HANDLER_HPP