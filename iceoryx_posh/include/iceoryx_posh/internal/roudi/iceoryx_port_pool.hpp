// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ICEORYX_PORT_POOL_HPP
#define IOX_POSH_ROUDI_ICEORYX_PORT_POOL_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolData;

class IceOryxPortPool : public PortPool
{
  public:
    IceOryxPortPool(PortPoolData& portPool) noexcept;

    /// @deprecated #25
    cxx::vector<SenderPortType::MemberType_t*, MAX_PUBLISHERS> senderPortDataList() noexcept override;
    /// @deprecated #25
    cxx::vector<ReceiverPortType::MemberType_t*, MAX_SUBSCRIBERS> receiverPortDataList() noexcept override;

    /// @deprecated #25
    cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
    addSenderPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const std::string& applicationName,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    /// @deprecated #25
    cxx::expected<ReceiverPortType::MemberType_t*, PortPoolError>
    addReceiverPort(const capro::ServiceDescription& serviceDescription,
                    const std::string& applicationName,
                    const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    /// @deprecated #25
    void removeSenderPort(SenderPortType::MemberType_t* const portData) noexcept override;
    /// @deprecated #25
    void removeReceiverPort(ReceiverPortType::MemberType_t* const portData) noexcept override;

    cxx::vector<PublisherPortRouDiType::MemberType_t*, MAX_PUBLISHERS> getPublisherPortDataList() noexcept override;
    cxx::vector<SubscriberPortProducerType::MemberType_t*, MAX_SUBSCRIBERS>
    getSubscriberPortDataList() noexcept override;

    cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    addPublisherPort(const capro::ServiceDescription& serviceDescription,
                     const uint64_t& historyCapacity,
                     mepoo::MemoryManager* const memoryManager,
                     const ProcessName_t& applicationName,
                     const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    cxx::expected<SubscriberPortProducerType::MemberType_t*, PortPoolError>
    addSubscriberPort(const capro::ServiceDescription& serviceDescription,
                      const uint64_t& historyRequest,
                      const ProcessName_t& applicationName,
                      const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    void removePublisherPort(PublisherPortRouDiType::MemberType_t* const portData) noexcept override;
    void removeSubscriberPort(SubscriberPortProducerType::MemberType_t* const portData) noexcept override;

  private:
    PortPoolData* m_portPoolData;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ICEORYX_PORT_POOL_HPP
