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

#include "iceoryx_utils/cxx/serialization.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"

#include <cstdint>
#include <initializer_list>

namespace iox
{
namespace capro
{
static constexpr uint16_t InvalidID = 0;
static constexpr char InvalidIDString[]{"0"};

static constexpr uint16_t AnyService = 0xFFFF;
static constexpr uint16_t AnyInstance = 0xFFFF;
static constexpr uint16_t AnyEvent = 0xFFFF;
static constexpr char AnyServiceString[]{"65535"};
static constexpr char AnyInstanceString[]{"65535"};
static constexpr char AnyEventString[]{"65535"};
static constexpr int32_t MAX_NUMBER_OF_CHARS = 64;

/// @brief Scope of a service description
enum class Scope : unsigned int
{
    WORLDWIDE,
    INTERNAL,
    INVALID
};

constexpr char ScopeTypeString[][MAX_NUMBER_OF_CHARS] = {{"WORLDWIDE"}, {"INTERNAL"}, {"INVALID"}};

/// @brief class for the identification of a communication event including information on the service, the service
/// instance and the event id.
/// In order to support different communication protocols, two types of members exist: integer and string identifiers.
/// If string IDs are used, the integers are initialized to an invalid number. A class object can be
/// serialized/deserialized, so it is possible to send the information e.g. over a message queue.
class ServiceDescription
{
  public:
    using IdString = cxx::CString100;

    struct ClassHash
    {
        ClassHash(const std::initializer_list<uint32_t>& values) noexcept;
        uint32_t& operator[](const uint64_t index) noexcept;
        const uint32_t& operator[](const uint64_t index) const noexcept;
        bool operator==(const ClassHash& rhs) const noexcept;

      private:
        static constexpr size_t CLASS_HASH_ELEMENT_COUNT{4};
        uint32_t data[CLASS_HASH_ELEMENT_COUNT];
    };

    /// @brief construction of the capro service description using serialized strings
    ServiceDescription(const cxx::Serialization& f_serial) noexcept;

    /// @brief default C'tor
    ServiceDescription() noexcept;

    /// @brief construction of the capro service description using integers to create a service service description
    ServiceDescription(uint16_t f_serviceID, uint16_t f_instanceID) noexcept;
    /// @brief construction of the capro service description using fixed strings to create a service service description
    ServiceDescription(const IdString& f_service, const IdString& f_instance) noexcept;

    /// @brief construction of the capro service description using integers to create an event service description
    ServiceDescription(uint16_t f_serviceID, uint16_t f_eventID, uint16_t f_instanceID) noexcept;
    /// @brief construction of the capro service description using fixed strings to create an event service description
    ServiceDescription(const IdString& f_service,
                       const IdString& f_instance,
                       const IdString& f_event,
                       ClassHash m_classHash = {0, 0, 0, 0}) noexcept;

    /// @brief compare operator. If wildcards AnyService, AnyInstance or AnyEvent are used as integer IDs, the
    /// corresponding member comparisons are skipped. Otherwise, both the integer and the string members are compared.
    bool operator==(const ServiceDescription& rhs) const;

    /// @brief negation of compare operator.
    bool operator!=(const ServiceDescription& rhs) const;

    /// @brief Uses the underlying m_**String compare method to provide an order.
    ///         This is needed to use ServiceDescription in sorted containers like map or set.
    bool operator<(const ServiceDescription& rhs) const;

    /// @brief copy all member variables from the other class instance
    ServiceDescription& operator=(const ServiceDescription& other);

    /// @brief serialization of the capro description.
    /// @todo operator Serialization() replaces getServiceString()
    operator cxx::Serialization() const;

    /// @brief Generate a servicestring filled with the member variables for
    /// service communication
    /// @return stringstream filled with member vars ()
    std::string getServiceString() const noexcept;

    /// @brief Returns true if it contains a service description which does not have
    ///             events, otherwise it returns false
    bool hasServiceOnlyDescription() const noexcept;

    // @brief Returns if this service description is used for an RouDi-internal channel
    bool isInternal() const noexcept;
    // @brief Set this service description to be is used for an RouDi-internal channel
    void setInternal() noexcept;
    /// @brief Returns the scope of a ServiceDescription
    Scope getScope() noexcept;

    ///@{
    /// Getters for the integer and string IDs
    uint16_t getInstanceID() const noexcept;
    uint16_t getServiceID() const noexcept;
    uint16_t getEventID() const noexcept;
    IdString getServiceIDString() const noexcept;
    IdString getInstanceIDString() const noexcept;
    IdString getEventIDString() const noexcept;
    ///@}

    ///@{
    /// Getter for class hash
    ClassHash getClassHash() const noexcept;
    ///@}

  private:
    /// @brief 16-Bit service ID
    uint16_t m_serviceID;
    /// @brief 16-Bit event ID
    uint16_t m_eventID;
    /// @brief 16-Bit instance ID
    uint16_t m_instanceID;
    /// @brief string representation of the service
    IdString m_serviceString;
    /// @brief string representation of the instance
    IdString m_instanceString;
    /// @brief string representation of the event
    IdString m_eventString;

    bool m_hasServiceOnlyDescription = false;
    /// @brief 128-Bit class hash (32-Bit * 4)
    ClassHash m_classHash{0, 0, 0, 0};

    /// @brief How far this service should be propagated
    Scope m_scope{Scope::WORLDWIDE};
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
