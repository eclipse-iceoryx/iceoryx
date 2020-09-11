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

#include "iceoryx_posh/internal/roudi/roudi_process.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/popo/receiver_port_data.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <chrono>
#include <thread>

namespace iox
{
namespace roudi
{
RouDiProcess::RouDiProcess(const ProcessName_t& name,
                           int32_t pid,
                           mepoo::MemoryManager* payloadMemoryManager,
                           bool isMonitored,
                           const uint64_t payloadSegmentId,
                           const uint64_t sessionId) noexcept
    : m_pid(pid)
    , m_mq(name)
    , m_timestamp(mepoo::BaseClock::now())
    , m_payloadMemoryManager(payloadMemoryManager)
    , m_isMonitored(isMonitored)
    , m_payloadSegmentId(payloadSegmentId)
    , m_sessionId(sessionId)
{
}

int32_t RouDiProcess::getPid() const noexcept
{
    return m_pid;
}

const ProcessName_t RouDiProcess::getName() const noexcept
{
    return ProcessName_t(cxx::TruncateToCapacity, m_mq.getInterfaceName());
}

void RouDiProcess::sendToMQ(const runtime::MqMessage& data) noexcept
{
    m_mq.send(data);
}

uint64_t RouDiProcess::getSessionId() noexcept
{
    return m_sessionId.load(std::memory_order_relaxed);
}

void RouDiProcess::setTimestamp(const mepoo::TimePointNs timestamp) noexcept
{
    m_timestamp = timestamp;
}

mepoo::TimePointNs RouDiProcess::getTimestamp() noexcept
{
    return m_timestamp;
}

mepoo::MemoryManager* RouDiProcess::getPayloadMemoryManager() const noexcept
{
    return m_payloadMemoryManager;
}

uint64_t RouDiProcess::getPayloadSegmentId() const noexcept
{
    return m_payloadSegmentId;
}

bool RouDiProcess::isMonitored() const noexcept
{
    return m_isMonitored;
}

//--------------------------------------------------------------------------------------------------

ProcessManager::ProcessManager(RouDiMemoryInterface& roudiMemoryInterface,
                               PortManager& portManager,
                               const version::CompatibilityCheckLevel compatibilityCheckLevel) noexcept
    : m_roudiMemoryInterface(roudiMemoryInterface)
    , m_portManager(portManager)
    , m_compatibilityCheckLevel(compatibilityCheckLevel)
{
    auto maybeSegmentManager = m_roudiMemoryInterface.segmentManager();
    if (!maybeSegmentManager.has_value())
    {
        LogFatal() << "Invalid state! Could not obtain SegmentManager!";
        std::terminate();
    }
    m_segmentManager = maybeSegmentManager.value();

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface.introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        LogFatal() << "Invalid state! Could not obtain MemoryManager for instrospection!";
        std::terminate();
    }
    m_introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

    auto maybeMgmtSegmentId = m_roudiMemoryInterface.mgmtMemoryProvider()->segmentId();
    if (!maybeMgmtSegmentId.has_value())
    {
        LogFatal() << "Invalid state! Could not obtain SegmentId for iceoryx management segment!";
        std::terminate();
    }
    m_mgmtSegmentId = maybeMgmtSegmentId.value();

    auto currentUser = posix::PosixUser::getUserOfCurrentProcess();
    auto m_segmentInfo = m_segmentManager->getSegmentInformationForUser(currentUser);
    m_memoryManagerOfCurrentProcess = m_segmentInfo.m_memoryManager;
}

void ProcessManager::killAllProcesses() noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    // send SIGTERM to all running applications
    typename ProcessList_t::iterator it = m_processList.begin();
    const typename ProcessList_t::iterator itEnd = m_processList.end();

    for (; itEnd != it; ++it)
    {
        if (-1 == kill(static_cast<pid_t>(it->getPid()), SIGTERM))
        {
            LogWarn() << "Process " << it->getPid() << " could not be killed";
        }
    }
}

bool ProcessManager::registerProcess(const ProcessName_t& name,
                                     int32_t pid,
                                     posix::PosixUser user,
                                     bool isMonitored,
                                     int64_t transmissionTimestamp,
                                     const uint64_t sessionId,
                                     const version::VersionInfo& versionInfo) noexcept
{
    bool wasPreviouslyMonitored = false; // must be in outer scope but is only initialized before use
    bool processExists = false;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        auto process = getProcessFromList(name); // process existence check
        if (process)
        {
            processExists = true;
            wasPreviouslyMonitored = process->isMonitored(); // needs to be read here under lock
        }
        else
        {
            wasPreviouslyMonitored = false; // does not really matter (only for static analysis - avoid unitialized var)
        }
    }
    // lock is not required anymore


    auto segmentInfo = m_segmentManager->getSegmentInformationForUser(user);

    if (!processExists)
    {
        // process does not exist in list and can be added
        return addProcess(name,
                          pid,
                          segmentInfo.m_memoryManager,
                          isMonitored,
                          transmissionTimestamp,
                          segmentInfo.m_segmentID,
                          sessionId,
                          versionInfo);
    }

    // process is already in list (i.e. registered)
    // depending on the mode we clean up the process resources and register it again
    // if it is monitored, we reject the registration and wait for automatic cleanup
    // otherwise we remove the process ourselves and register it again

    if (wasPreviouslyMonitored)
    {
        // process exists and is monitored - we rely on monitoring for removal
        LogWarn() << "Received REG from " << name << ", but another application with this name is already registered";
    }
    else
    {
        // process exists and is not monitored - remove it and add the new process afterwards
        LogDebug() << "Registering already existing application " << name;

        // remove existing process
        if (!removeProcess(name)) // call will acquire lock
        {
            LogWarn() << "Received REG from " << name
                      << ", but another application with this name is already registered and could not be removed";
            return false;
        }

        LogDebug() << "Registering already existing application " << name << " - removed existing application";

        // try registration again, should succeed since removal was successful
        return addProcess(name,
                          pid,
                          segmentInfo.m_memoryManager,
                          isMonitored,
                          transmissionTimestamp,
                          segmentInfo.m_segmentID,
                          sessionId,
                          versionInfo); // call will acquire lock
    }

    return false;
}

bool ProcessManager::addProcess(const ProcessName_t& name,
                                int32_t pid,
                                mepoo::MemoryManager* payloadMemoryManager,
                                bool isMonitored,
                                int64_t transmissionTimestamp,
                                const uint64_t payloadSegmentId,
                                const uint64_t sessionId,
                                const version::VersionInfo& versionInfo) noexcept
{
    if (!version::VersionInfo::getCurrentVersion().checkCompatibility(versionInfo, m_compatibilityCheckLevel))
    {
        LogError()
            << "Version mismatch from '" << name
            << "'! Please build your app and RouDi against the same iceoryx version (version & commitID). RouDi: "
            << version::VersionInfo::getCurrentVersion().operator iox::cxx::Serialization().toString()
            << " App: " << versionInfo.operator iox::cxx::Serialization().toString();
        return false;
    }
    std::lock_guard<std::mutex> g(m_mutex);
    // overflow check
    if (m_processList.size() >= MAX_PROCESS_NUMBER)
    {
        LogError() << "Could not register process '" << name << "' - too many processes";
        return false;
    }

    m_processList.emplace_back(name, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);

    // send REG_ACK and BaseAddrString
    runtime::MqMessage sendBuffer;

    auto offset = RelativePointer::getOffset(m_mgmtSegmentId, m_segmentManager);
    sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::REG_ACK)
               << m_roudiMemoryInterface.mgmtMemoryProvider()->size() << offset << transmissionTimestamp
               << m_mgmtSegmentId;

    m_processList.back().sendToMQ(sendBuffer);

    // set current timestamp again (already done in RouDiProcess's constructor
    m_processList.back().setTimestamp(mepoo::BaseClock::now());

    m_processIntrospection->addProcess(pid, ProcessName_t(cxx::TruncateToCapacity, name.c_str()));

    LogDebug() << "Registered new application " << name;
    return true;
}

bool ProcessManager::removeProcess(const ProcessName_t& name) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);
    // we need to search for the process (currently linear search)

    auto it = m_processList.begin();
    while (it != m_processList.end())
    {
        if (it->getName() == name)
        {
            m_portManager.deletePortsOfProcess(name);

            m_processIntrospection->removeProcess(it->getPid());

            // delete application
            it = m_processList.erase(it);

            LogDebug() << "New Registration - removed existing application " << name;
            return true; // we can assume there are no other processes with this name
        }
        ++it;
    }
    return false;
}

void ProcessManager::updateLivelinessOfProcess(const ProcessName_t& name) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // reset timestamp
        process->setTimestamp(mepoo::BaseClock::now());
    }
    else
    {
        LogWarn() << "Received Keepalive from unknown process " << name;
    }
}

void ProcessManager::findServiceForProcess(const ProcessName_t& name, const capro::ServiceDescription& service) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        runtime::MqMessage instanceString({m_portManager.findService(service)});
        process->sendToMQ(instanceString);
        LogDebug() << "Sent InstanceString to application " << name;
    }
    else
    {
        LogWarn() << "Unknown process " << name << " requested an InstanceString.";
    }
}

void ProcessManager::addInterfaceForProcess(const ProcessName_t& name,
                                            capro::Interfaces interface,
                                            const RunnableName_t& runnable) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a ReceiverPort
        popo::InterfacePortData* port = m_portManager.acquireInterfacePortData(interface, name, runnable);

        // send ReceiverPort to app as a serialized relative pointer
        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, port);

        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_INTERFACE_ACK)
                   << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
        process->sendToMQ(sendBuffer);

        LogDebug() << "Created new interface for application " << name;
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested an interface.";
    }
}

void ProcessManager::sendServiceRegistryChangeCounterToProcess(const ProcessName_t& processName) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);
    RouDiProcess* process = getProcessFromList(processName);
    if (nullptr != process)
    {
        // send counter to app as a serialized relative pointer
        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, m_portManager.serviceRegistryChangeCounter());

        runtime::MqMessage sendBuffer;
        sendBuffer << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
        process->sendToMQ(sendBuffer);
    }
    else
    {
        LogWarn() << "Unknown application " << processName << " requested an serviceRegistryChangeCounter.";
    }
}

void ProcessManager::addApplicationForProcess(const ProcessName_t& name) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        popo::ApplicationPortData* port = m_portManager.acquireApplicationPortData(name);

        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, port);

        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_APPLICATION_ACK)
                   << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
        process->sendToMQ(sendBuffer);

        LogDebug() << "Created new ApplicationPort for application " << name;
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested an ApplicationPort." << name;
    }
}

void ProcessManager::addRunnableForProcess(const ProcessName_t& processName,
                                           const RunnableName_t& runnableName) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(processName);
    if (nullptr != process)
    {
        runtime::RunnableData* runnable =
            m_portManager.acquireRunnableData(cxx::string<100>(cxx::TruncateToCapacity, processName),
                                              cxx::string<100>(cxx::TruncateToCapacity, runnableName));

        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, runnable);

        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_RUNNABLE_ACK)
                   << std::to_string(offset) << std::to_string(m_mgmtSegmentId);

        process->sendToMQ(sendBuffer);
        m_processIntrospection->addRunnable(cxx::string<100>(cxx::TruncateToCapacity, processName.c_str()),
                                            cxx::string<100>(cxx::TruncateToCapacity, runnableName.c_str()));
        LogDebug() << "Created new runnable " << runnableName << " for application " << processName;
    }
    else
    {
        LogWarn() << "Unknown application " << processName << " requested a runnable.";
    }
}

void ProcessManager::sendMessageNotSupportedToRuntime(const ProcessName_t& name) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::MESSAGE_NOT_SUPPORTED);
        process->sendToMQ(sendBuffer);

        LogError() << "Application " << name << " sent a message, which is not supported by this RouDi";
    }
}

/// @deprecated #25
void ProcessManager::addReceiverForProcess(const ProcessName_t& name,
                                           const capro::ServiceDescription& service,
                                           const RunnableName_t& runnable,
                                           const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a ReceiverPort

        /// @todo: it might be useful to encapsulate this into some kind of port factory
        /// (which maybe contains a m_shmMgr)
        /// main goal would be to isolate the port creation logic as it becomes more complex
        /// pursuing this further could lead to a separate management entity for ports
        /// which could support queries like: find all ports with a given service or some other
        /// specific attribute (to allow efficient and well encapsulated lookup)

        ReceiverPortType::MemberType_t* receiver =
            m_portManager.acquireReceiverPortData(service, name, runnable, portConfigInfo);

        // send ReceiverPort to app as a serialized relative pointer
        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, receiver);

        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_RECEIVER_ACK)
                   << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
        process->sendToMQ(sendBuffer);

        LogDebug() << "Created new ReceiverPortImpl for application " << name;
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested a ReceiverPortImpl.";
    }
}

/// @deprecated #25
void ProcessManager::addSenderForProcess(const ProcessName_t& name,
                                         const capro::ServiceDescription& service,
                                         const RunnableName_t& runnable,
                                         const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a SenderPort
        auto maybeSender = m_portManager.acquireSenderPortData(
            service, name, process->getPayloadMemoryManager(), runnable, portConfigInfo);

        if (!maybeSender.has_error())
        {
            // send SenderPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybeSender.get_value());

            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_SENDER_ACK)
                       << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
            process->sendToMQ(sendBuffer);

            LogDebug() << "Created new SenderPortImpl for application " << name;
        }
        else
        {
            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::ERROR);
            sendBuffer << runtime::mqMessageErrorTypeToString( // map error codes
                (maybeSender.get_error() == PortPoolError::UNIQUE_SENDER_PORT_ALREADY_EXISTS
                     ? runtime::MqMessageErrorType::NO_UNIQUE_CREATED
                     : runtime::MqMessageErrorType::SENDERLIST_FULL));
            process->sendToMQ(sendBuffer);
            LogError() << "Could not create SenderPortImpl for application " << name;
        }
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested a SenderPortImpl.";
    }
}

void ProcessManager::addSubscriberForProcess(const ProcessName_t& name,
                                             const capro::ServiceDescription& service,
                                             const uint64_t& historyRequest,
                                             const RunnableName_t& runnable,
                                             const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a SubscriberPort
        auto maybeSubscriber =
            m_portManager.acquireSubscriberPortData(service, historyRequest, name, runnable, portConfigInfo);

        if (!maybeSubscriber.has_error())
        {
            // send SubscriberPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybeSubscriber.get_value());

            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_SUBSCRIBER_ACK)
                       << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
            process->sendToMQ(sendBuffer);

            LogDebug() << "Created new SubscriberPort for application " << name;
        }
        else
        {
            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::ERROR);
            sendBuffer << runtime::mqMessageErrorTypeToString(runtime::MqMessageErrorType::SUBSCRIBERLIST_FULL);
            process->sendToMQ(sendBuffer);
            LogError() << "Could not create SubscriberPort for application " << name;
        }
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested a SubscriberPort.";
    }
}

void ProcessManager::addPublisherForProcess(const ProcessName_t& name,
                                            const capro::ServiceDescription& service,
                                            const uint64_t& historyCapacity,
                                            const RunnableName_t& runnable,
                                            const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a PublisherPort
        auto maybePublisher = m_portManager.acquirePublisherPortData(
            service, historyCapacity, name, process->getPayloadMemoryManager(), runnable, portConfigInfo);

        if (!maybePublisher.has_error())
        {
            // send PublisherPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybePublisher.get_value());

            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_PUBLISHER_ACK)
                       << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
            process->sendToMQ(sendBuffer);

            LogDebug() << "Created new PublisherPort for application " << name;
        }
        else
        {
            runtime::MqMessage sendBuffer;
            sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::ERROR);
            sendBuffer << runtime::mqMessageErrorTypeToString( // map error codes
                (maybePublisher.get_error() == PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS
                     ? runtime::MqMessageErrorType::NO_UNIQUE_CREATED
                     : runtime::MqMessageErrorType::PUBLISHERLIST_FULL));
            process->sendToMQ(sendBuffer);
            LogError() << "Could not create PublisherPort for application " << name;
        }
    }
    else
    {
        LogWarn() << "Unknown application " << name << " requested a PublisherPort.";
    }
}

void ProcessManager::addConditionVariableForProcess(const ProcessName_t& processName) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(processName);
    if (nullptr != process)
    {
        // Try to create a condition variable
        m_portManager.acquireConditionVariableData()
            .and_then([&](popo::ConditionVariableData* condVar) {
                auto offset = RelativePointer::getOffset(m_mgmtSegmentId, condVar);

                runtime::MqMessage sendBuffer;
                sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_CONDITION_VARIABLE_ACK)
                           << std::to_string(offset) << std::to_string(m_mgmtSegmentId);
                process->sendToMQ(sendBuffer);

                LogDebug() << "Created new ConditionVariableImpl for application " << processName;
            })
            .or_else([&](PortPoolError error) {
                runtime::MqMessage sendBuffer;
                sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::ERROR);
                if (error == PortPoolError::CONDITION_VARIABLE_LIST_FULL)
                {
                    sendBuffer << runtime::mqMessageErrorTypeToString(
                        runtime::MqMessageErrorType::CONDITION_VARIABLE_LIST_FULL);
                }
                process->sendToMQ(sendBuffer);

                LogDebug() << "Could not create new ConditionVariableImpl for application " << processName;
            });
    }
    else
    {
        LogWarn() << "Unknown application " << processName << " requested a ConditionVariableImpl.";
    }
}

void ProcessManager::initIntrospection(ProcessIntrospectionType* processIntrospection) noexcept
{
    m_processIntrospection = processIntrospection;
}

void ProcessManager::run() noexcept
{
    monitorProcesses();
    discoveryUpdate();
    std::this_thread::sleep_for(std::chrono::milliseconds(DISCOVERY_INTERVAL.milliSeconds<int64_t>()));
}

SenderPortType ProcessManager::addIntrospectionSenderPort(const capro::ServiceDescription& service,
                                                          const ProcessName_t& process_name) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    return SenderPortType(
        m_portManager.acquireSenderPortData(service, process_name, m_introspectionMemoryManager).get_value());
}

RouDiProcess* ProcessManager::getProcessFromList(const ProcessName_t& name) noexcept
{
    RouDiProcess* processPtr = nullptr;

    typename ProcessList_t::iterator it = m_processList.begin();
    const typename ProcessList_t::iterator itEnd = m_processList.end();

    for (; itEnd != it; ++it)
    {
        if (name == it->getName())
        {
            processPtr = &(*it);
            break;
        }
    }

    return processPtr;
}

void ProcessManager::monitorProcesses() noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    auto currentTimestamp = mepoo::BaseClock::now();

    auto processIterator = m_processList.begin();
    while (processIterator != m_processList.end())
    {
        if (processIterator->isMonitored())
        {
            auto timediff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimestamp
                                                                                     - processIterator->getTimestamp())
                                   .count();

            static_assert(PROCESS_KEEP_ALIVE_TIMEOUT > PROCESS_KEEP_ALIVE_INTERVAL, "keep alive timeout too small");
            if (std::chrono::milliseconds(timediff_ms)
                > std::chrono::milliseconds(PROCESS_KEEP_ALIVE_TIMEOUT.milliSeconds<int64_t>()))
            {
                LogWarn() << "Application " << processIterator->getName() << " not responding (last response "
                          << timediff_ms << " milliseconds ago) --> removing it";

                // note: if we would want to use the removeProcess function, it would search for the process again (but
                // we already found it and have an iterator to remove it)

                // delete all associated receiver and sender impl in shared
                // memory and the associated RouDi discovery ports
                // @todo Check if ShmManager and Process Manager end up in unintended condition
                m_portManager.deletePortsOfProcess(processIterator->getName());

                m_processIntrospection->removeProcess(processIterator->getPid());

                // delete application
                processIterator = m_processList.erase(processIterator);
                continue; // erase returns first element after the removed one --> skip iterator increment
            }
        }
        ++processIterator;
    }
}

void ProcessManager::discoveryUpdate() noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    m_portManager.doDiscovery();
}

} // namespace roudi
} // namespace iox
