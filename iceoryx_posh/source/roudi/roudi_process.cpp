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
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/platform/signal.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/wait.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

#include <chrono>
#include <thread>

using namespace iox::units::duration_literals;

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

void ProcessManager::killAllProcesses(const units::Duration processKillDelay) noexcept
{
    std::lock_guard<std::mutex> lockGuard(m_mutex);
    cxx::vector<bool, MAX_PROCESS_NUMBER> processStillRunning(m_processList.size(), true);
    uint64_t i{0};
    bool haveAllProcessesFinished{false};
    posix::Timer finalKillTimer(processKillDelay);

    auto awaitProcessTermination = [&]() {
        bool shouldCheckProcessState = true;
        finalKillTimer.resetCreationTime();

        // try to shut down all processes until either all processes have terminated or a timer set to processKillDelay
        // has expired
        while (!haveAllProcessesFinished && !finalKillTimer.hasExpiredComparedToCreationTime())
        {
            i = 0;

            // give processes some time to terminate before checking their state
            std::this_thread::sleep_for(
                std::chrono::milliseconds(PROCESS_TERMINATED_CHECK_INTERVAL.milliSeconds<int64_t>()));

            for (auto& process : m_processList)
            {
                if (processStillRunning[i]
                    && !requestShutdownOfProcess(process, ShutdownPolicy::SIG_TERM, ShutdownLog::NONE))
                {
                    processStillRunning[i] = false;
                    shouldCheckProcessState = true;
                }
                ++i;
            }

            // check if we are done
            if (shouldCheckProcessState)
            {
                shouldCheckProcessState = false;
                haveAllProcessesFinished = true;
                for (bool isRunning : processStillRunning)
                {
                    haveAllProcessesFinished &= !isRunning;
                }
            }
        }
    };

    i = 0;
    // send SIG_TERM to all running applications and wait for process to complete
    for (auto& process : m_processList)
    {
        // if it was killed we need to check if it really has terminated, if we can't kill it, we consider it
        // terminated
        processStillRunning[i] = requestShutdownOfProcess(process, ShutdownPolicy::SIG_TERM, ShutdownLog::FULL);
        ++i;
    }

    // we sent SIG_TERM, now wait till they have terminated
    awaitProcessTermination();

    // any processes still alive? Time to send SIG_KILL to kill.
    if (finalKillTimer.hasExpiredComparedToCreationTime())
    {
        i = 0;
        for (auto& process : m_processList)
        {
            if (processStillRunning[i])
            {
                LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                          << "' is still running after SIGTERM was sent " << processKillDelay.seconds<int>()
                          << " seconds ago. RouDi is sending SIGKILL now.";
                processStillRunning[i] = requestShutdownOfProcess(process, ShutdownPolicy::SIG_KILL, ShutdownLog::FULL);
            }
            ++i;
        }

        // we sent SIG_KILL to kill, now wait till they have terminated
        awaitProcessTermination();

        // any processes still alive? Time to ignore them.
        if (finalKillTimer.hasExpiredComparedToCreationTime())
        {
            i = 0;
            for (auto& process : m_processList)
            {
                if (processStillRunning[i])
                {
                    LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                              << "' is still running after SIGKILL was sent " << processKillDelay.seconds<int>()
                              << " seconds ago. RouDi is ignoring this process.";
                }
                ++i;
            }
        }
    }

    auto it = m_processList.begin();
    while (removeProcess(lockGuard, it))
    {
        it = m_processList.begin();
    }
}

bool ProcessManager::requestShutdownOfProcess(const RouDiProcess& process,
                                              ShutdownPolicy shutdownPolicy,
                                              ShutdownLog shutdownLog) noexcept
{
    static constexpr int32_t ERROR_CODE = -1;
    auto killC = iox::cxx::makeSmartC(kill,
                                      iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                      {ERROR_CODE},
                                      {},
                                      static_cast<pid_t>(process.getPid()),
                                      (shutdownPolicy == ShutdownPolicy::SIG_KILL ? SIGKILL : SIGTERM));
    if (killC.hasErrors())
    {
        if (shutdownLog == ShutdownLog::FULL)
        {
            switch (killC.getErrNum())
            {
            case EINVAL:
                LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                          << "' could not be killed with "
                          << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                          << ", because the signal sent was invalid.";
                break;
            case EPERM:
                LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                          << "' could not be killed with "
                          << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                          << ", because RouDi doesn't have the permission to send the signal to the target processes.";
                break;
            case ESRCH:
                LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                          << "' could not be killed with "
                          << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                          << ", because the target process or process group does not exist.";
                break;
            default:
                LogWarn() << "Process ID " << process.getPid() << " named '" << process.getName()
                          << "' could not be killed with"
                          << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                          << " for unknown reason: ’" << killC.getErrorString() << "'";
            }
        }
        return false;
    }
    return true;
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
    std::lock_guard<std::mutex> lockGuard(m_mutex);
    // we need to search for the process (currently linear search)

    auto it = m_processList.begin();
    while (it != m_processList.end())
    {
        auto otherName = it->getName();
        if (name == otherName)
        {
            if (removeProcess(lockGuard, it))
            {
                LogDebug() << "New Registration - removed existing application " << name;
            }
            return true; // we can assume there are no other processes with this name
        }
        ++it;
    }
    return false;
}

bool ProcessManager::removeProcess(const std::lock_guard<std::mutex>& lockGuard [[gnu::unused]],
                                   ProcessList_t::iterator& processIter) noexcept
{
    // don't take the lock, else it needs to be recursive
    if (processIter != m_processList.end())
    {
        m_portManager.deletePortsOfProcess(processIter->getName());
        m_processIntrospection->removeProcess(processIter->getPid());
        processIter = m_processList.erase(processIter); // delete application
        return true;
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
                                            const NodeName_t& node) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a ReceiverPort
        popo::InterfacePortData* port = m_portManager.acquireInterfacePortData(interface, name, node);

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

void ProcessManager::addNodeForProcess(const ProcessName_t& processName, const NodeName_t& nodeName) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(processName);
    if (nullptr != process)
    {
        runtime::NodeData* node = m_portManager.acquireNodeData(ProcessName_t(cxx::TruncateToCapacity, processName),
                                                                NodeName_t(cxx::TruncateToCapacity, nodeName));

        auto offset = RelativePointer::getOffset(m_mgmtSegmentId, node);

        runtime::MqMessage sendBuffer;
        sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_NODE_ACK) << std::to_string(offset)
                   << std::to_string(m_mgmtSegmentId);

        process->sendToMQ(sendBuffer);
        m_processIntrospection->addNode(ProcessName_t(cxx::TruncateToCapacity, processName.c_str()),
                                        NodeName_t(cxx::TruncateToCapacity, nodeName.c_str()));
        LogDebug() << "Created new node " << nodeName << " for process " << processName;
    }
    else
    {
        LogWarn() << "Unknown process " << processName << " requested a node.";
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
                                           const NodeName_t& node,
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
            m_portManager.acquireReceiverPortData(service, name, node, portConfigInfo);

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
                                         const NodeName_t& node,
                                         const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a SenderPort
        auto maybeSender = m_portManager.acquireSenderPortData(
            service, name, process->getPayloadMemoryManager(), node, portConfigInfo);

        if (!maybeSender.has_error())
        {
            // send SenderPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybeSender.value());

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
                                             const NodeName_t& node,
                                             const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a SubscriberPort
        auto maybeSubscriber =
            m_portManager.acquireSubscriberPortData(service, historyRequest, name, node, portConfigInfo);

        if (!maybeSubscriber.has_error())
        {
            // send SubscriberPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybeSubscriber.value());

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
            sendBuffer << runtime::mqMessageErrorTypeToString(runtime::MqMessageErrorType::SUBSCRIBER_LIST_FULL);
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
                                            const NodeName_t& node,
                                            const PortConfigInfo& portConfigInfo) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* process = getProcessFromList(name);
    if (nullptr != process)
    {
        // create a PublisherPort
        auto maybePublisher = m_portManager.acquirePublisherPortData(
            service, historyCapacity, name, process->getPayloadMemoryManager(), node, portConfigInfo);

        if (!maybePublisher.has_error())
        {
            // send PublisherPort to app as a serialized relative pointer
            auto offset = RelativePointer::getOffset(m_mgmtSegmentId, maybePublisher.value());

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
                     : runtime::MqMessageErrorType::PUBLISHER_LIST_FULL));
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
        m_portManager.acquireSenderPortData(service, process_name, m_introspectionMemoryManager).value());
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

                // note: if we would want to use the removeProcess function, it would search for the process again
                // (but we already found it and have an iterator to remove it)

                // delete all associated receiver and sender impl in shared
                // memory and the associated RouDi discovery ports
                // @todo Check if ShmManager and Process Manager end up in unintended condition
                m_portManager.deletePortsOfProcess(processIterator->getName());

                /// @todo #25 Need to delete condition variables used by terminating processes...

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
