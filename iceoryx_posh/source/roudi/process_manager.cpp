// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/process_manager.hpp"
#include "iceoryx_platform/signal.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/wait.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"
#include "iox/relative_pointer.hpp"
#include "iox/std_chrono_support.hpp"
#include "iox/vector.hpp"

#include <chrono>
#include <thread>

using namespace iox::units::duration_literals;

namespace iox
{
namespace roudi
{
ProcessManager::ProcessManager(RouDiMemoryInterface& roudiMemoryInterface,
                               PortManager& portManager,
                               const DomainId domainId,
                               const version::CompatibilityCheckLevel compatibilityCheckLevel) noexcept
    : m_roudiMemoryInterface(roudiMemoryInterface)
    , m_portManager(portManager)
    , m_domainId(domainId)
    , m_compatibilityCheckLevel(compatibilityCheckLevel)
{
    bool fatalError{false};

    auto maybeSegmentManager = m_roudiMemoryInterface.segmentManager();
    if (!maybeSegmentManager.has_value())
    {
        IOX_LOG(FATAL, "Invalid state! Could not obtain SegmentManager!");
        fatalError = true;
    }
    else
    {
        m_segmentManager = maybeSegmentManager.value();
    }

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface.introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        IOX_LOG(FATAL, "Invalid state! Could not obtain MemoryManager for instrospection!");
        fatalError = true;
    }
    else
    {
        m_introspectionMemoryManager = maybeIntrospectionMemoryManager.value();
    }

    auto maybeMgmtSegmentId = m_roudiMemoryInterface.mgmtMemoryProvider()->segmentId();
    if (!maybeMgmtSegmentId.has_value())
    {
        IOX_LOG(FATAL, "Invalid state! Could not obtain SegmentId for iceoryx management segment!");
        fatalError = true;
    }
    else
    {
        m_mgmtSegmentId = maybeMgmtSegmentId.value();
    }

    auto maybeHeartbeatPool = m_roudiMemoryInterface.heartbeatPool();
    if (!maybeHeartbeatPool.has_value())
    {
        IOX_LOG(FATAL, "Invalid state! Could not obtain HeartbeatPool!");
        fatalError = true;
    }
    else
    {
        m_heartbeatPool = maybeHeartbeatPool.value();
    }

    if (fatalError)
    {
        /// @todo iox-#539 Use separate error enums once RouDi is more modular
        IOX_REPORT_FATAL(PoshError::ROUDI__PRECONDITIONS_FOR_PROCESS_MANAGER_NOT_FULFILLED);
    }
}

void ProcessManager::handleProcessShutdownPreparationRequest(const RuntimeName_t& name) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) {
            m_portManager.unblockProcessShutdown(name);
            // Reply with PREPARE_APP_TERMINATION_ACK and let process shutdown
            runtime::IpcMessage sendBuffer;
            sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::PREPARE_APP_TERMINATION_ACK);
            process->sendViaIpcChannel(sendBuffer);
        })
        .or_else([&]() { IOX_LOG(WARN, "Unknown application " << name << " requested shutdown preparation."); });
}

void ProcessManager::requestShutdownOfAllProcesses() noexcept
{
    // send SIG_TERM to all running applications and wait for processes to answer with TERMINATION
    for (auto& process : m_processList)
    {
        IOX_LOG(DEBUG, "Sending SIGTERM to Process ID " << process.getPid() << " named '" << process.getName());
        requestShutdownOfProcess(process, ShutdownPolicy::SIG_TERM);
    }

    // this unblocks the RouDi shutdown if a publisher port is blocked by a full subscriber queue
    m_portManager.unblockRouDiShutdown();
}

uint64_t ProcessManager::registeredProcessCount() const noexcept
{
    return m_processList.size();
}

bool ProcessManager::probeRegisteredProcessesAliveWithSigTerm() noexcept
{
    for (auto& process : m_processList)
    {
        if (probeProcessAliveWithSigTerm(process))
        {
            return true;
        }
    }
    return false;
}

void ProcessManager::killAllProcesses() noexcept
{
    for (auto& process : m_processList)
    {
        IOX_LOG(WARN,
                "Process ID " << process.getPid() << " named '" << process.getName()
                              << "' is still running after SIGTERM was sent. RouDi is sending SIGKILL now.");
        requestShutdownOfProcess(process, ShutdownPolicy::SIG_KILL);
    }
}

void ProcessManager::printWarningForRegisteredProcessesAndClearProcessList() noexcept
{
    for (auto& process : m_processList)
    {
        IOX_LOG(WARN,
                "Process ID " << process.getPid() << " named '" << process.getName()
                              << "' is still running after SIGKILL was sent. RouDi is ignoring this process.");
    }
    m_processList.clear();
}

bool ProcessManager::requestShutdownOfProcess(Process& process, ShutdownPolicy shutdownPolicy) noexcept
{
    static constexpr int32_t ERROR_CODE = -1;

    return !IOX_POSIX_CALL(kill)(static_cast<pid_t>(process.getPid()),
                                 (shutdownPolicy == ShutdownPolicy::SIG_KILL) ? SIGKILL : SIGTERM)
                .failureReturnValue(ERROR_CODE)
                .ignoreErrnos(ESRCH)
                .evaluate()
                .or_else([&](auto& r) {
                    this->evaluateKillError(process, r.errnum, r.getHumanReadableErrnum().c_str(), shutdownPolicy);
                })
                .has_error();
}

bool ProcessManager::probeProcessAliveWithSigTerm(const Process& process) noexcept
{
    static constexpr int32_t ERROR_CODE = -1;
    auto checkCommand = IOX_POSIX_CALL(kill)(static_cast<pid_t>(process.getPid()), SIGTERM)
                            .failureReturnValue(ERROR_CODE)
                            .ignoreErrnos(ESRCH)
                            .evaluate()
                            .or_else([&](auto& r) {
                                this->evaluateKillError(
                                    process, r.errnum, r.getHumanReadableErrnum().c_str(), ShutdownPolicy::SIG_TERM);
                            });

    return !(checkCommand && checkCommand->errnum == ESRCH);
}

void ProcessManager::evaluateKillError(const Process& process,
                                       const int32_t& errnum,
                                       const char* errorString,
                                       ShutdownPolicy shutdownPolicy) noexcept
{
    if ((errnum == EINVAL) || (errnum == EPERM) || (errnum == ESRCH))
    {
        IOX_LOG(WARN,
                "Process ID " << process.getPid() << " named '" << process.getName() << "' could not be killed with "
                              << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                              << ", because the command failed with the following error: " << errorString
                              << " See manpage for kill(2) or type 'man 2 kill' in console for more information");
        IOX_REPORT(PoshError::POSH__ROUDI_PROCESS_SHUTDOWN_FAILED, iox::er::RUNTIME_ERROR);
    }
    else
    {
        IOX_LOG(WARN,
                "Process ID " << process.getPid() << " named '" << process.getName() << "' could not be killed with"
                              << (shutdownPolicy == ShutdownPolicy::SIG_KILL ? "SIGKILL" : "SIGTERM")
                              << " for unknown reason: '" << errorString << "'");
        IOX_REPORT(PoshError::POSH__ROUDI_PROCESS_SHUTDOWN_FAILED, iox::er::RUNTIME_ERROR);
    }
}

bool ProcessManager::registerProcess(const RuntimeName_t& name,
                                     const uint32_t pid,
                                     const PosixUser user,
                                     const bool isMonitored,
                                     const int64_t transmissionTimestamp,
                                     const uint64_t sessionId,
                                     const version::VersionInfo& versionInfo) noexcept
{
    bool returnValue{false};

    findProcess(name)
        .and_then([&](auto& process) {
            // process is already in list (i.e. registered)
            // depending on the mode we clean up the process resources and register it again
            // if it is monitored, we reject the registration and wait for automatic cleanup
            // otherwise we remove the process ourselves and register it again

            if (process->isMonitored())
            {
                IOX_LOG(WARN, "Received register request, but termination of " << name << " not detected yet");
            }

            // process exists, we expect that the existing process crashed
            IOX_LOG(WARN,
                    "Application '" << name
                                    << "' is still registered but it seems it has been terminated without RouDi "
                                       "recognizing it. Re-registering application");

            // remove the existing process and add the new process afterwards, we do not send ack to new process
            constexpr TerminationFeedback TERMINATION_FEEDBACK{TerminationFeedback::DO_NOT_SEND_ACK_TO_PROCESS};
            if (!this->searchForProcessAndRemoveIt(name, TERMINATION_FEEDBACK))
            {
                IOX_LOG(WARN, "Application " << name << " could not be removed");
                return;
            }
            else
            {
                // try registration again, should succeed since removal was successful
                returnValue =
                    this->addProcess(name, pid, user, isMonitored, transmissionTimestamp, sessionId, versionInfo);
            }
        })
        .or_else([&]() {
            // process does not exist in list and can be added
            returnValue = this->addProcess(name, pid, user, isMonitored, transmissionTimestamp, sessionId, versionInfo);
        });

    return returnValue;
}

bool ProcessManager::addProcess(const RuntimeName_t& name,
                                const uint32_t pid,
                                const PosixUser& user,
                                const bool isMonitored,
                                const int64_t transmissionTimestamp,
                                const uint64_t sessionId,
                                const version::VersionInfo& versionInfo) noexcept
{
    if (!version::VersionInfo::getCurrentVersion().checkCompatibility(versionInfo, m_compatibilityCheckLevel))
    {
        IOX_LOG(
            ERROR,
            "Version mismatch from '"
                << name
                << "'! Please build your app and RouDi against the same iceoryx version (version & commitID). RouDi: "
                << version::VersionInfo::getCurrentVersion().operator iox::Serialization().toString()
                << " App: " << versionInfo.operator iox::Serialization().toString());
        return false;
    }
    // overflow check
    if (m_processList.size() >= MAX_PROCESS_NUMBER)
    {
        IOX_LOG(ERROR, "Could not register process '" << name << "' - too many processes");
        return false;
    }

    auto heartbeatPoolIndex = HeartbeatPool::Index::INVALID;
    iox::UntypedRelativePointer::offset_t heartbeatOffset{iox::UntypedRelativePointer::NULL_POINTER_OFFSET};

    if (isMonitored)
    {
        auto heartbeat = m_heartbeatPool->emplace();
        heartbeatPoolIndex = heartbeat.to_index();
        heartbeatOffset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, heartbeat.to_ptr());
    }
    m_processList.emplace_back(name, m_domainId, pid, user, heartbeatPoolIndex, sessionId);

    // send REG_ACK and BaseAddrString
    runtime::IpcMessage sendBuffer;

    auto segmentManagerOffset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, m_segmentManager);
    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::REG_ACK)
               << m_roudiMemoryInterface.mgmtMemoryProvider()->size() << segmentManagerOffset << transmissionTimestamp
               << m_mgmtSegmentId << heartbeatOffset;

    m_processList.back().sendViaIpcChannel(sendBuffer);

    m_processIntrospection->addProcess(static_cast<int>(pid), name);

    IOX_LOG(DEBUG, "Registered new application " << name);
    return true;
}

bool ProcessManager::unregisterProcess(const RuntimeName_t& name) noexcept
{
    constexpr TerminationFeedback FEEDBACK{TerminationFeedback::SEND_ACK_TO_PROCESS};
    if (!searchForProcessAndRemoveIt(name, FEEDBACK))
    {
        IOX_LOG(ERROR, "Application " << name << " could not be unregistered!");
        return false;
    }
    return true;
}

bool ProcessManager::searchForProcessAndRemoveIt(const RuntimeName_t& name, const TerminationFeedback feedback) noexcept
{
    // we need to search for the process (currently linear search)
    auto it = m_processList.begin();
    while (it != m_processList.end())
    {
        auto otherName = it->getName();
        if (name == otherName)
        {
            if (removeProcessAndDeleteRespectiveSharedMemoryObjects(it, feedback))
            {
                IOX_LOG(DEBUG, "Removed existing application " << name);
            }
            return true; // we can assume there are no other processes with this name
        }
        ++it;
    }
    return false;
}

bool ProcessManager::removeProcessAndDeleteRespectiveSharedMemoryObjects(ProcessList_t::iterator& processIter,
                                                                         const TerminationFeedback feedback) noexcept
{
    if (processIter != m_processList.end())
    {
        m_portManager.deletePortsOfProcess(processIter->getName());
        m_processIntrospection->removeProcess(static_cast<int32_t>(processIter->getPid()));

        if (feedback == TerminationFeedback::SEND_ACK_TO_PROCESS)
        {
            // Reply with TERMINATION_ACK and let process shutdown
            runtime::IpcMessage sendBuffer;
            sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::TERMINATION_ACK);
            processIter->sendViaIpcChannel(sendBuffer);
        }

        auto heartbeatIter = m_heartbeatPool->iter_from_index(processIter->getHeartbeatPoolIndex());
        if (heartbeatIter != m_heartbeatPool->end())
        {
            m_heartbeatPool->erase(heartbeatIter);
        }
        processIter = m_processList.erase(processIter); // delete application
        return true;
    }
    return false;
}

void ProcessManager::addInterfaceForProcess(const RuntimeName_t& name, capro::Interfaces interface) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) {
            // create a ReceiverPort
            popo::InterfacePortData* port = m_portManager.acquireInterfacePortData(interface, name);

            // send ReceiverPort to app as a serialized relative pointer
            auto offset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, port);

            runtime::IpcMessage sendBuffer;
            sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::CREATE_INTERFACE_ACK)
                       << convert::toString(offset) << convert::toString(m_mgmtSegmentId);
            process->sendViaIpcChannel(sendBuffer);

            IOX_LOG(DEBUG, "Created new interface for application " << name);
        })
        .or_else([&]() { IOX_LOG(WARN, "Unknown application " << name << " requested an interface."); });
}

void ProcessManager::sendMessageNotSupportedToRuntime(const RuntimeName_t& name) noexcept
{
    findProcess(name).and_then([&](auto& process) {
        runtime::IpcMessage sendBuffer;
        sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::MESSAGE_NOT_SUPPORTED);
        process->sendViaIpcChannel(sendBuffer);

        IOX_LOG(ERROR, "Application " << name << " sent a message, which is not supported by this RouDi");
    });
}

void ProcessManager::addSubscriberForProcess(const RuntimeName_t& name,
                                             const capro::ServiceDescription& service,
                                             const popo::SubscriberOptions& subscriberOptions,
                                             const PortConfigInfo& portConfigInfo) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) {
            // create a SubscriberPort
            auto maybeSubscriber =
                m_portManager.acquireSubscriberPortData(service, subscriberOptions, name, portConfigInfo);

            if (maybeSubscriber.has_value())
            {
                // send SubscriberPort to app as a serialized relative pointer
                auto offset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, maybeSubscriber.value());

                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::CREATE_SUBSCRIBER_ACK)
                           << convert::toString(offset) << convert::toString(m_mgmtSegmentId);
                process->sendViaIpcChannel(sendBuffer);

                IOX_LOG(DEBUG,
                        "Created new SubscriberPort for application '" << name << "' with service description '"
                                                                       << service << "'");
            }
            else
            {
                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                sendBuffer << runtime::IpcMessageErrorTypeToString(runtime::IpcMessageErrorType::SUBSCRIBER_LIST_FULL);
                process->sendViaIpcChannel(sendBuffer);
                IOX_LOG(ERROR,
                        "Could not create SubscriberPort for application '" << name << "' with service description '"
                                                                            << service << "'");
            }
        })
        .or_else([&]() {
            IOX_LOG(WARN,
                    "Unknown application '" << name << "' requested a SubscriberPort with service description '"
                                            << service << "'");
        });
}

void ProcessManager::addPublisherForProcess(const RuntimeName_t& name,
                                            const capro::ServiceDescription& service,
                                            const popo::PublisherOptions& publisherOptions,
                                            const PortConfigInfo& portConfigInfo) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) { // create a PublisherPort
            auto segmentInfo = m_segmentManager->getSegmentInformationWithWriteAccessForUser(process->getUser());

            if (!segmentInfo.m_memoryManager.has_value())
            {
                // Tell the app no writable shared memory segment was found
                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                sendBuffer << runtime::IpcMessageErrorTypeToString(
                    runtime::IpcMessageErrorType::REQUEST_PUBLISHER_NO_WRITABLE_SHM_SEGMENT);
                process->sendViaIpcChannel(sendBuffer);
                return;
            }

            auto maybePublisher = m_portManager.acquirePublisherPortData(
                service, publisherOptions, name, &segmentInfo.m_memoryManager.value().get(), portConfigInfo);

            if (maybePublisher.has_value())
            {
                // send PublisherPort to app as a serialized relative pointer
                auto offset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, maybePublisher.value());

                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::CREATE_PUBLISHER_ACK)
                           << convert::toString(offset) << convert::toString(m_mgmtSegmentId);
                process->sendViaIpcChannel(sendBuffer);

                IOX_LOG(DEBUG,
                        "Created new PublisherPort for application '" << name << "' with service description '"
                                                                      << service << "'");
            }
            else
            {
                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);

                std::string error;
                switch (maybePublisher.error())
                {
                case PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS:
                {
                    error = runtime::IpcMessageErrorTypeToString(runtime::IpcMessageErrorType::NO_UNIQUE_CREATED);
                    break;
                }
                case PortPoolError::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN:
                {
                    error = runtime::IpcMessageErrorTypeToString(
                        runtime::IpcMessageErrorType::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN);
                    break;
                }
                default:
                {
                    error = runtime::IpcMessageErrorTypeToString(runtime::IpcMessageErrorType::PUBLISHER_LIST_FULL);
                    break;
                }
                }
                sendBuffer << error;

                process->sendViaIpcChannel(sendBuffer);
                IOX_LOG(ERROR,
                        "Could not create PublisherPort for application '" << name << "' with service description '"
                                                                           << service << "'");
            }
        })
        .or_else([&]() {
            IOX_LOG(WARN,
                    "Unknown application '" << name << "' requested a PublisherPort with service description '"
                                            << service << "'");
        });
}

void ProcessManager::addClientForProcess(const RuntimeName_t& name,
                                         const capro::ServiceDescription& service,
                                         const popo::ClientOptions& clientOptions,
                                         const PortConfigInfo& portConfigInfo) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) { // create a ClientPort
            auto segmentInfo = m_segmentManager->getSegmentInformationWithWriteAccessForUser(process->getUser());

            if (!segmentInfo.m_memoryManager.has_value())
            {
                // Tell the app no writable shared memory segment was found
                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                sendBuffer << runtime::IpcMessageErrorTypeToString(
                    runtime::IpcMessageErrorType::REQUEST_CLIENT_NO_WRITABLE_SHM_SEGMENT);
                process->sendViaIpcChannel(sendBuffer);
                return;
            }

            m_portManager
                .acquireClientPortData(
                    service, clientOptions, name, &segmentInfo.m_memoryManager.value().get(), portConfigInfo)
                .and_then([&](auto& clientPort) {
                    auto relativePtrToClientPort =
                        UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, clientPort);

                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::CREATE_CLIENT_ACK)
                               << convert::toString(relativePtrToClientPort) << convert::toString(m_mgmtSegmentId);
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(DEBUG,
                            "Created new ClientPort for application '" << name << "' with service description '"
                                                                       << service << "'");
                })
                .or_else([&](auto&) {
                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                    sendBuffer << runtime::IpcMessageErrorTypeToString(runtime::IpcMessageErrorType::CLIENT_LIST_FULL);
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(ERROR,
                            "Could not create ClientPort for application '" << name << "' with service description '"
                                                                            << service << "'");
                });
        })
        .or_else([&]() {
            IOX_LOG(WARN,
                    "Unknown application '" << name << "' requested a ClientPort with service description '" << service
                                            << "'");
        });
}

void ProcessManager::addServerForProcess(const RuntimeName_t& name,
                                         const capro::ServiceDescription& service,
                                         const popo::ServerOptions& serverOptions,
                                         const PortConfigInfo& portConfigInfo) noexcept
{
    findProcess(name)
        .and_then([&](auto& process) { // create a ServerPort
            auto segmentInfo = m_segmentManager->getSegmentInformationWithWriteAccessForUser(process->getUser());

            if (!segmentInfo.m_memoryManager.has_value())
            {
                // Tell the app no writable shared memory segment was found
                runtime::IpcMessage sendBuffer;
                sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                sendBuffer << runtime::IpcMessageErrorTypeToString(
                    runtime::IpcMessageErrorType::REQUEST_SERVER_NO_WRITABLE_SHM_SEGMENT);
                process->sendViaIpcChannel(sendBuffer);
                return;
            }

            m_portManager
                .acquireServerPortData(
                    service, serverOptions, name, &segmentInfo.m_memoryManager.value().get(), portConfigInfo)
                .and_then([&](auto& serverPort) {
                    auto relativePtrToServerPort =
                        UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, serverPort);

                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::CREATE_SERVER_ACK)
                               << convert::toString(relativePtrToServerPort) << convert::toString(m_mgmtSegmentId);
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(DEBUG,
                            "Created new ServerPort for application '" << name << "' with service description '"
                                                                       << service << "'");
                })
                .or_else([&](auto&) {
                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                    sendBuffer << runtime::IpcMessageErrorTypeToString(runtime::IpcMessageErrorType::SERVER_LIST_FULL);
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(ERROR,
                            "Could not create ServerPort for application '" << name << "' with service description '"
                                                                            << service << "'");
                });
        })
        .or_else([&]() {
            IOX_LOG(WARN,
                    "Unknown application '" << name << "' requested a ServerPort with service description '" << service
                                            << "'");
        });
}

void ProcessManager::addConditionVariableForProcess(const RuntimeName_t& runtimeName) noexcept
{
    findProcess(runtimeName)
        .and_then([&](auto& process) { // Try to create a condition variable
            m_portManager.acquireConditionVariableData(runtimeName)
                .and_then([&](auto condVar) {
                    auto offset = UntypedRelativePointer::getOffset(segment_id_t{m_mgmtSegmentId}, condVar);

                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(
                        runtime::IpcMessageType::CREATE_CONDITION_VARIABLE_ACK)
                               << convert::toString(offset) << convert::toString(m_mgmtSegmentId);
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(DEBUG, "Created new ConditionVariable for application " << runtimeName);
                })
                .or_else([&](PortPoolError error) {
                    runtime::IpcMessage sendBuffer;
                    sendBuffer << runtime::IpcMessageTypeToString(runtime::IpcMessageType::ERROR);
                    if (error == PortPoolError::CONDITION_VARIABLE_LIST_FULL)
                    {
                        sendBuffer << runtime::IpcMessageErrorTypeToString(
                            runtime::IpcMessageErrorType::CONDITION_VARIABLE_LIST_FULL);
                    }
                    process->sendViaIpcChannel(sendBuffer);

                    IOX_LOG(DEBUG, "Could not create new ConditionVariable for application " << runtimeName);
                });
        })
        .or_else([&]() { IOX_LOG(WARN, "Unknown application " << runtimeName << " requested a ConditionVariable."); });
}

void ProcessManager::initIntrospection(ProcessIntrospectionType* processIntrospection) noexcept
{
    m_processIntrospection = processIntrospection;
}

void ProcessManager::run() noexcept
{
    monitorProcesses();
    discoveryUpdate();
}

popo::PublisherPortData*
ProcessManager::addIntrospectionPublisherPort(const capro::ServiceDescription& service) noexcept
{
    popo::PublisherOptions options;
    options.historyCapacity = 1U;
    options.nodeName = INTROSPECTION_NODE_NAME;
    return m_portManager.acquireInternalPublisherPortData(service, options, m_introspectionMemoryManager);
}

optional<Process*> ProcessManager::findProcess(const RuntimeName_t& name) noexcept
{
    for (auto& process : m_processList)
    {
        if (process.getName() == name)
        {
            return make_optional<Process*>(&process);
        }
    }

    return nullopt;
}

void ProcessManager::monitorProcesses() noexcept
{
    static_assert(runtime::PROCESS_KEEP_ALIVE_TIMEOUT > runtime::PROCESS_KEEP_ALIVE_INTERVAL,
                  "keep alive timeout too small");
    auto timeout = runtime::PROCESS_KEEP_ALIVE_TIMEOUT.toMilliseconds();
    auto heartbeatIterator = m_heartbeatPool->begin();
    while (heartbeatIterator != m_heartbeatPool->end())
    {
        auto currentHeartbeatIterator = heartbeatIterator++;
        auto elapsedMilliseconds = currentHeartbeatIterator->elapsed_milliseconds_since_last_beat();
        if (elapsedMilliseconds > timeout)
        {
            bool removed{false};
            auto processIterator = m_processList.begin();
            while (processIterator != m_processList.end())
            {
                if (processIterator->getHeartbeatPoolIndex() != currentHeartbeatIterator.to_index())
                {
                    ++processIterator;
                    continue;
                }

                IOX_LOG(WARN,
                        "Application " << processIterator->getName() << " not responding (last response "
                                       << elapsedMilliseconds << " milliseconds ago) --> removing it");

                removed = removeProcessAndDeleteRespectiveSharedMemoryObjects(
                    processIterator, TerminationFeedback::DO_NOT_SEND_ACK_TO_PROCESS);
                break;
            }
            if (!removed)
            {
                IOX_LOG(WARN,
                        "Could not find application for corresponding heartbeat! HeartbeatPoolIndex: "
                            << currentHeartbeatIterator.to_index());
                m_heartbeatPool->erase(currentHeartbeatIterator);
            }
        }
    }
}

void ProcessManager::discoveryUpdate() noexcept
{
    m_portManager.doDiscovery();
}

} // namespace roudi
} // namespace iox
