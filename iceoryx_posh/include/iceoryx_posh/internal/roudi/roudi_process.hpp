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

#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/roudi/introspection/process_introspection.hpp"
#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"

#include <csignal>
#include <cstdint>
#include <ctime>
#include <list>
#include <string>

namespace iox
{
namespace roudi
{
class RouDiProcess
{
  public:
    RouDiProcess(std::string f_name,
                 int f_pid,
                 mepoo::MemoryManager* f_payloadMemoryManager,
                 bool isMonitored,
                 const uint64_t payloadSegmentId);

    RouDiProcess(const RouDiProcess& other) = delete;
    RouDiProcess& operator=(const RouDiProcess& other) = delete;
    RouDiProcess(RouDiProcess&& other) = default;
    RouDiProcess& operator=(RouDiProcess&& other) = default;
    ~RouDiProcess() = default;

    int getPid() const;

    const std::string& getName() const;

    void sendToMQ(const runtime::MqMessage& f_data);

    void setTimestamp(const mepoo::TimePointNs f_timestamp);

    mepoo::TimePointNs getTimestamp();

    mepoo::MemoryManager* getPayloadMemoryManager() const;
    uint64_t getPayloadSegmentId() const;

    bool isMonitored() const;

  private:
    int m_pid;
    runtime::MqInterfaceUser m_mq;
    mepoo::TimePointNs m_timestamp;
    mepoo::MemoryManager* m_payloadMemoryManager{nullptr};
    bool m_isMonitored{true};
    uint64_t m_payloadSegmentId;
};

class ProcessManagerInterface
{
  public:
    virtual bool sendMessageToProcess(const std::string& f_name, const iox::runtime::MqMessage& f_message) = 0;

    // port handling
    virtual ReceiverPortType addInternalReceiverPort(const capro::ServiceDescription& f_service,
                                                     const std::string& f_process_name) = 0;
    virtual SenderPortType addInternalSenderPort(const capro::ServiceDescription& f_service,
                                                 const std::string& f_process_name) = 0;
    virtual void removeInternalPorts(const std::string& f_process_name) = 0;
    virtual void sendServiceRegistryChangeCounterToProcess(const std::string& f_process_name) = 0;
    virtual bool areAllReceiverPortsSubscribed(const std::string& f_process_name) = 0;
    virtual void discoveryUpdate() = 0;

    // enable data-triggering -> based on receiver port
    virtual ~ProcessManagerInterface()
    {
    }
};

class ProcessManager : public ProcessManagerInterface
{
  public:
    ProcessManager(SharedMemoryManager& f_shmMgr);
    virtual ~ProcessManager() override
    {
    }

    ProcessManager(const ProcessManager& other) = delete;
    ProcessManager& operator=(const ProcessManager& other) = delete;

    bool registerProcess(const std::string& f_name,
                         int f_pid,
                         posix::PosixUser f_user,
                         bool f_isMonitored,
                         int64_t transmissionTimestamp);

    void killAllProcesses();

    void updateLivlinessOfProcess(const std::string& f_name);

    void findServiceForProcess(const std::string& f_name, const capro::ServiceDescription& f_service);

    void addInterfaceForProcess(const std::string& f_name, Interfaces f_interface, const std::string& f_runnable);

    void addApplicationForProcess(const std::string& f_name, Interfaces f_interface);

    void addRunnableForProcess(const std::string& f_process, const std::string& f_runnable);

    void removeRunnableForProcess(const std::string& f_process, const std::string& f_runnable);

    void addReceiverForProcess(const std::string& f_name,
                               const capro::ServiceDescription& f_service,
                               Interfaces f_interface,
                               const std::string& f_runnable);

    void addSenderForProcess(const std::string& f_name,
                             const capro::ServiceDescription& f_service,
                             Interfaces f_interface,
                             const std::string& f_runnable);

    void initIntrospection(ProcessIntrospectionType* f_processIntrospection);

    void run();

    SenderPortType addIntrospectionSenderPort(const capro::ServiceDescription& f_service,
                                              const std::string& f_process_name);

    /// @brief Notify the application that it sent an unsupported message
    void sendMessageNotSupportedToRuntime(const std::string& f_name);

    bool sendMessageToProcess(const std::string& f_name, const iox::runtime::MqMessage& f_message) override;
    // PortHandling interface
    ReceiverPortType addInternalReceiverPort(const capro::ServiceDescription& f_service,
                                             const std::string& f_process_name) override;
    SenderPortType addInternalSenderPort(const capro::ServiceDescription& f_service,
                                         const std::string& f_process_name) override;
    void removeInternalPorts(const std::string& f_process_name) override;
    void sendServiceRegistryChangeCounterToProcess(const std::string& f_process_name) override;

    bool areAllReceiverPortsSubscribed(const std::string& f_process_name) override;


  private:
    RouDiProcess* getProcessFromList(const std::string& f_name);
    void monitorProcesses();
    void discoveryUpdate() override;

    bool addProcess(const std::string& f_name,
                    int f_pid,
                    mepoo::MemoryManager* f_payloadMemoryManager,
                    bool f_isMonitored,
                    int64_t transmissionTimestamp,
                    const uint64_t segmentId);

    bool removeProcess(const std::string& f_name);

    SharedMemoryManager& m_shmMgr;
    mutable std::mutex m_mutex;
    /// @todo use a fixed, stack based list once available
    // using ProcessList_t = cxx::list<RouDiProcess, MAX_PROCESS_NUMBER>;
    using ProcessList_t = std::list<RouDiProcess>;
    ProcessList_t m_processList;

    ProcessIntrospectionType* m_processIntrospection{nullptr};

    // this is currently used for the internal sender/receiver ports
    mepoo::MemoryManager* m_memoryManagerOfCurrentProcess{nullptr};
    uint64_t m_segmentIdOfCurrentProcess{0};
};

} // namespace roudi
} // namespace iox
