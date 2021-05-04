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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_CAPRO_CAPRO_MESSAGE_HPP
#define IOX_POSH_CAPRO_CAPRO_MESSAGE_HPP

#include "iceoryx_posh/capro/service_description.hpp"

namespace iox
{
namespace capro
{
/// @brief Enum for service message types which are used in CaPro for
/// service-oriented communication
enum class CaproMessageType : uint8_t
{
    NOTYPE = 0,
    FIND,
    OFFER,
    STOP_OFFER,
    SUB,
    UNSUB,
    ACK,
    NACK,
    PUB,
    REQ,
    RES,
    PING,
    PONG,
    MESSGAGE_TYPE_END
};

constexpr int32_t MAX_ENUM_STRING_SIZE = 64;
constexpr char CaproMessageTypeString[][MAX_ENUM_STRING_SIZE] = {
    "NOTYPE", "FIND", "OFFER", "STOP_OFFER", "SUB", "UNSUB", "ACK", "NACK", "PUB", "REQ", "RES", "PING", "PONG"};


enum class CaproMessageSubType : uint8_t
{
    NOSUBTYPE = 0,
    SERVICE,
    EVENT,
    FIELD
};

/// @brief C'tors for CaPro messages
class CaproMessage
{
  public:
    /// @brief default constructor
    CaproMessage() = default;

    /// @brief C'tor for CaPro Message with type, service description
    /// @param type                    Message type
    /// @param serviceDescription      Service Description
    /// @param subType                 Message sub type
    /// @param chunkQueueData(0)       No port
    /// @return                        Nothing
    CaproMessage(CaproMessageType type,
                 const ServiceDescription& serviceDescription,
                 CaproMessageSubType subType = CaproMessageSubType::NOSUBTYPE,
                 void* chunkQueueData = nullptr) noexcept;

    CaproMessageType m_type{CaproMessageType::NOTYPE};
    CaproMessageSubType m_subType{CaproMessageSubType::NOSUBTYPE};
    ServiceDescription m_serviceDescription;
    void* m_chunkQueueData{nullptr};
    uint64_t m_historyCapacity{0u};
};

} // namespace capro
} // namespace iox

#endif // IOX_POSH_CAPRO_CAPRO_MESSAGE_HPP
