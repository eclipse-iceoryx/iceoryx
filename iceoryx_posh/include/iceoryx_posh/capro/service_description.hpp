// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_CAPRO_SERVICE_DESCRIPTION_HPP
#define IOX_POSH_CAPRO_SERVICE_DESCRIPTION_HPP

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/serialization.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include <cstdint>
#include <initializer_list>

namespace iox
{
namespace capro
{
/// @todo remove
static const IdString_t InvalidString{"INVALID"};
static constexpr char AnyServiceString[]{"65535"};
static constexpr char AnyInstanceString[]{"65535"};
static constexpr char AnyEventString[]{"65535"};
static constexpr int32_t MAX_NUMBER_OF_CHARS = 64;
static constexpr size_t CLASS_HASH_ELEMENT_COUNT{4U};

/// @brief Describes from which interface the service is coming from
enum class Interfaces : uint16_t
{
    /// @brief Used for services, which are locally on this system and not coming over a gateway
    INTERNAL = 0,
    /// @brief Canonical protocol for CAN Bus
    ESOC,
    /// @brief SOME/IP
    SOMEIP,
    /// @brief Advanced Message Queuing Protocol
    AMQP,
    /// @brief Message Queuing Telemetry Transport
    MQTT,
    /// @brief Data Distribution Service
    DDS,
    /// @brief Universal Measurement and Calibration Protocol (XCP)
    SIGNAL,
    /// @brief Measurement technology adapter
    MTA,
    /// @brief Robot Operating System 1
    ROS1,
    /// @brief End of enum
    INTERFACE_END
};

constexpr const char* INTERFACE_NAMES[] = {"INTERNAL", "ESOC", "SOMEIP", "AMQP", "DDS", "SIGNAL", "MTA", "ROS1", "END"};

/// @brief Scope of a service description
enum class Scope : uint16_t
{
    WORLDWIDE,
    INTERNAL,
    INVALID
};

constexpr char ScopeTypeString[][MAX_NUMBER_OF_CHARS] = {"WORLDWIDE", "INTERNAL", "INVALID"};

/// @brief class for the identification of a communication event including information on the service, the service
/// instance and the event id.
/// A class object can be serialized/deserialized, so it is possible to send the information e.g. over a IPC channel.
class ServiceDescription
{
  public:
    struct ClassHash
    {
        ClassHash() noexcept;
        ClassHash(const std::initializer_list<uint32_t>& values) noexcept;
        uint32_t& operator[](iox::cxx::range<uint64_t, 0, CLASS_HASH_ELEMENT_COUNT - 1> index) noexcept;
        const uint32_t& operator[](iox::cxx::range<uint64_t, 0, CLASS_HASH_ELEMENT_COUNT - 1> index) const noexcept;
        bool operator==(const ClassHash& rhs) const noexcept;
        bool operator!=(const ClassHash& rhs) const noexcept;

      private:
        uint32_t data[CLASS_HASH_ELEMENT_COUNT];
    };

    /// @brief construction of the capro service description using serialized strings
    ServiceDescription(const cxx::Serialization& serial) noexcept;

    /// @brief default C'tor
    ServiceDescription() noexcept;
    ServiceDescription(const ServiceDescription&) noexcept = default;
    ServiceDescription(ServiceDescription&&) noexcept = default;
    ~ServiceDescription() noexcept = default;

    /// @brief construction of the capro service description using fixed strings to create an event service description
    ServiceDescription(const IdString_t& service,
                       const IdString_t& instance,
                       const IdString_t& event,
                       ClassHash m_classHash = {0u, 0u, 0u, 0u},
                       Interfaces interfaceSource = Interfaces::INTERNAL) noexcept;

    /// @brief compare operator. If wildcards AnyServiceString, AnyInstanceString or AnyEventString are used, the
    /// corresponding member comparisons are skipped.
    bool operator==(const ServiceDescription& rhs) const;

    /// @brief negation of compare operator.
    bool operator!=(const ServiceDescription& rhs) const;

    /// @brief Uses the underlying m_**String compare method to provide an order.
    ///         This is needed to use ServiceDescription in sorted containers like map or set.
    bool operator<(const ServiceDescription& rhs) const;

    ServiceDescription& operator=(const ServiceDescription&) = default;
    ServiceDescription& operator=(ServiceDescription&&) = default;

    /// @brief serialization of the capro description.
    operator cxx::Serialization() const;

    // @brief Returns if this service description is used for an RouDi-internal channel
    bool isInternal() const noexcept;
    // @brief Set this service description to be is used for an RouDi-internal channel
    void setInternal() noexcept;
    /// @brief Returns the scope of a ServiceDescription
    Scope getScope() noexcept;

    /// @todo remove any*string from doxygen?
    ///@brief Returns true for valid ServiceDescription
    /// false for ServiceDescription that contains either of InvalidID/InvalidIDString  AnyService/AnyServiceString,
    /// AnyInstance/AnyInstanceString, AnyEvent/AnyEventString.
    bool isValid() const noexcept;

    ///@{
    /// Getters for the integer and string IDs
    IdString_t getServiceIDString() const noexcept;
    IdString_t getInstanceIDString() const noexcept;
    IdString_t getEventIDString() const noexcept;
    ///@}

    ///@{
    /// Getter for class hash
    ClassHash getClassHash() const noexcept;
    ///@}

    /// @brief Returns the interface form where the service is coming from.
    Interfaces getSourceInterface() const noexcept;

  private:
    /// @brief string representation of the service
    IdString_t m_serviceString{InvalidString};
    /// @brief string representation of the instance
    IdString_t m_instanceString{InvalidString};
    /// @brief string representation of the event
    IdString_t m_eventString{InvalidString};

    /// @brief 128-Bit class hash (32-Bit * 4)
    ClassHash m_classHash{0, 0, 0, 0};

    /// @brief How far this service should be propagated
    Scope m_scope{Scope::WORLDWIDE};

    /// @brief If StopOffer or Offer message, this is set from which interface its coming
    Interfaces m_interfaceSource{Interfaces::INTERNAL};
};

/// @brief Compare two service descriptions via their values in member
/// variables
/// and return bool if match
/// @param ServiceDescription &first       Servicedescription to compare
/// @param ServiceDescription &second      Servicedescription to compare
/// @return                                 Bool if comparison match or not
bool serviceMatch(const ServiceDescription& first, const ServiceDescription& second) noexcept;

} // namespace capro
} // namespace iox

#endif // IOX_POSH_CAPRO_SERVICE_DESCRIPTION_HPP
