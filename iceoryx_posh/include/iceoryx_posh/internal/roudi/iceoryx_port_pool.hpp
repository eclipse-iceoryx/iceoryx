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

#pragma once

#include "iceoryx_posh/iceoryx_posh_types.hpp"
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

    cxx::vector<SenderPortType::MemberType_t*, MAX_PORT_NUMBER> senderPortDataList() noexcept override;
    cxx::vector<ReceiverPortType::MemberType_t*, MAX_PORT_NUMBER> receiverPortDataList() noexcept override;

    cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
    addSenderPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const std::string& applicationName,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    cxx::expected<ReceiverPortType::MemberType_t*, PortPoolError>
    addReceiverPort(const capro::ServiceDescription& serviceDescription,
                    const std::string& applicationName,
                    const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept override;

    void removeSenderPort(SenderPortType::MemberType_t* const portData) noexcept override;
    void removeReceiverPort(ReceiverPortType::MemberType_t* const portData) noexcept override;

  private:
    PortPoolData* m_portPoolData;
};

} // namespace roudi
} // namespace iox
