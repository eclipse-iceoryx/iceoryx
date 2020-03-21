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
#include "iceoryx_posh/internal/popo/application_port.hpp"
#include "iceoryx_posh/internal/popo/interface_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/runtime/runnable_data.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolDataBase;

enum class PortPoolError : uint8_t
{
    UNIQUE_SENDER_PORT_ALREADY_EXISTS,
    SENDER_PORT_LIST_FULL,
    RECEIVER_PORT_LIST_FULL,
    INTERFACE_PORT_LIST_FULL,
    APPLICATION_PORT_LIST_FULL,
    RUNNABLE_DATA_LIST_FULL,
};

class PortPool
{
  public:
    PortPool(PortPoolDataBase& portPoolDataBase) noexcept;

    virtual ~PortPool() noexcept = default;

    /// @todo don't create the vector with each call but only when the data really change
    /// there could be a member "cxx::vector<popo::SenderPortData* m_senderPorts;" and senderPorts() would just update
    /// this member if the sender ports actually changed
    virtual cxx::vector<SenderPortType::MemberType_t*, MAX_PORT_NUMBER> senderPortDataList() noexcept = 0;
    virtual cxx::vector<ReceiverPortType::MemberType_t*, MAX_PORT_NUMBER> receiverPortDataList() noexcept = 0;
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> interfacePortDataList() noexcept;
    cxx::vector<popo::ApplicationPortData*, MAX_PROCESS_NUMBER> appliactionPortDataList() noexcept;
    cxx::vector<runtime::RunnableData*, MAX_RUNNABLE_NUMBER> runnableDataList() noexcept;

    virtual cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
    addSenderPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const std::string& applicationName,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept = 0;

    virtual cxx::expected<ReceiverPortType::MemberType_t*, PortPoolError>
    addReceiverPort(const capro::ServiceDescription& serviceDescription,
                    const std::string& applicationName,
                    const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept = 0;

    cxx::expected<popo::InterfacePortData*, PortPoolError> addInterfacePort(const std::string& applicationName,
                                                                            const capro::Interfaces interface) noexcept;

    cxx::expected<popo::ApplicationPortData*, PortPoolError>
    addApplicationPort(const std::string& applicationName) noexcept;

    cxx::expected<runtime::RunnableData*, PortPoolError>
    addRunnableData(const iox::cxx::CString100& process,
                    const iox::cxx::CString100& runnable,
                    const uint64_t runnableDeviceIdentifier) noexcept;

    virtual void removeSenderPort(SenderPortType::MemberType_t* const portData) noexcept = 0;
    virtual void removeReceiverPort(ReceiverPortType::MemberType_t* const portData) noexcept = 0;
    void removeInterfacePort(popo::InterfacePortData* const portData) noexcept;
    void removeApplicationPort(popo::ApplicationPortData* const portData) noexcept;
    void removeRunnableData(runtime::RunnableData* const runnableData) noexcept;

    std::atomic<uint64_t>* serviceRegistryChangeCounter() noexcept;

  private:
    PortPoolDataBase* m_portPoolDataBase;
};

} // namespace roudi
} // namespace iox
