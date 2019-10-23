// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_posh/internal/popo/base_port_data.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace popo
{
/// @brief this class is the base for all ports. it is constructed from a member
///         pointer and is only movable.
///        only movable rational: a port has only one member, a pointer to its data. if a port
///                               is copied then both ports would work on the same data even
///                               though these are two independent copies. this would case
///                               a weird shared state, race conditions and so on
///
///        before using a port it is, depending on the use case, important to verify that
///        the port member pointers are set
/// @code
///     auto port = std::move(GetPortFromSomewhereElse());
///     if ( port ) {
///         // do stuff
///     }
/// @endcode
class BasePort
{
  public:
    using MemberType_t = BasePortData;

    BasePort(MemberType_t* const f_basePortDataPtr) noexcept;

    BasePort(const BasePort& other) = delete;
    BasePort& operator=(const BasePort&) = delete;
    BasePort(BasePort&&) noexcept;
    BasePort& operator=(BasePort&&) noexcept;

    virtual ~BasePort() noexcept;

    /// @brief a port can be constructed from a nullptr, additionally it also can be moved
    ///         and in these cases the member methods would work on a nullptr.
    ///         to circumvent this problem
    /// @return if the memberpointer is not null it returns true, otherwise false
    operator bool() const;

    /// @brief Reads Type of actual CaPro Port (sender/receiver...)
    /// @param              Nothing
    /// @return m_portType  Type of Port in struct BasePortType
    BasePortType getPortType() const noexcept;

    /// @brief Reads Type of actual CaPro Port (sender/receiver...)
    /// @param              Nothing
    /// @return m_portType  Type of Port in struct BasePortType
    capro::ServiceDescription getCaProServiceDescription() const noexcept;

    /// @brief Gets Application Name for the active port
    /// @param              Nothing
    /// @return             Application name as String
    cxx::CString100 getApplicationName() const noexcept;


    /// @brief Gets Interface Name for the active port
    /// @param              Nothing
    /// @return             Interface name as enum value
    Interfaces getInterface() const noexcept;

    /// @brief Gets Id of thethe active port
    /// @param              Nothing
    /// @return             UniqueId name as Integer
    uint64_t getUniqueID() const noexcept;

  protected:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

  private:
    MemberType_t* m_basePortDataPtr{nullptr};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_port.inl"
