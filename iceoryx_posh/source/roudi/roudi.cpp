// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_dust/cxx/convert.hpp"
#include "iceoryx_dust/cxx/std_string_support.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/system_configuration.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_posh/internal/runtime/node_property.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace roudi
{
RouDi::RouDi(RouDiMemoryInterface& roudiMemoryInterface,
             PortManager& portManager,
             RoudiStartupParameters roudiStartupParameters) noexcept
    : m_killProcessesInDestructor(roudiStartupParameters.m_killProcessesInDestructor)
    , m_runMonitoringAndDiscoveryThread(true)
    , m_runHandleRuntimeMessageThread(true)
    , m_roudiMemoryInterface(&roudiMemoryInterface)
    , m_portManager(&portManager)
    , m_prcMgr(concurrent::ForwardArgsToCTor,
               *m_roudiMemoryInterface,
               portManager,
               roudiStartupParameters.m_compatibilityCheckLevel)
    , m_mempoolIntrospection(
          *m_roudiMemoryInterface->introspectionMemoryManager().value(),
          *m_roudiMemoryInterface->segmentManager().value(),
          PublisherPortUserType(m_prcMgr->addIntrospectionPublisherPort(IntrospectionMempoolService)))
    , m_monitoringMode(roudiStartupParameters.m_monitoringMode)
    , m_processTerminationDelay(roudiStartupParameters.m_processTerminationDelay)
    , m_processKillDelay(roudiStartupParameters.m_processKillDelay)
{
    if (internal::isCompiledOn32BitSystem())
    {
        IOX_LOG(WARN, "Runnning RouDi on 32-bit architectures is not supported! Use at your own risk!");
    }
    m_processIntrospection.registerPublisherPort(
        PublisherPortUserType(m_prcMgr->addIntrospectionPublisherPort(IntrospectionProcessService)));
    m_prcMgr->initIntrospection(&m_processIntrospection);
    m_processIntrospection.run();
    m_mempoolIntrospection.run();

    // since RouDi offers the introspection services, also add it to the list of processes
    m_processIntrospection.addProcess(getpid(), IPC_CHANNEL_ROUDI_NAME);

    // initialize semaphore for discovery loop finish indicator
    iox::posix::UnnamedSemaphoreBuilder()
        .initialValue(0U)
        .isInterProcessCapable(false)
        .create(m_discoveryFinishedSemaphore)
        .expect("Valid Semaphore");

    // run the threads
    m_monitoringAndDiscoveryThread = std::thread(&RouDi::monitorAndDiscoveryUpdate, this);
    posix::setThreadName(m_monitoringAndDiscoveryThread.native_handle(), "Mon+Discover");

    if (roudiStartupParameters.m_runtimesMessagesThreadStart == RuntimeMessagesThreadStart::IMMEDIATE)
    {
        startProcessRuntimeMessagesThread();
    }
}

RouDi::~RouDi() noexcept
{
    shutdown();
}

void RouDi::startProcessRuntimeMessagesThread() noexcept
{
    m_handleRuntimeMessageThread = std::thread(&RouDi::processRuntimeMessages, this);
    posix::setThreadName(m_handleRuntimeMessageThread.native_handle(), "IPC-msg-process");
}

void RouDi::shutdown() noexcept
{
    // trigger the shutdown of the monitoring and discovery thread in order to prevent application to register while
    // shutting down
    m_runMonitoringAndDiscoveryThread = false;
    m_discoveryLoopTrigger.trigger();

    // stop the introspection
    m_processIntrospection.stop();
    m_mempoolIntrospection.stop();
    m_portManager->stopPortIntrospection();

    // wait for the monitoring and discovery thread to stop
    if (m_monitoringAndDiscoveryThread.joinable())
    {
        IOX_LOG(DEBUG, "Joining 'Mon+Discover' thread...");
        m_monitoringAndDiscoveryThread.join();
        IOX_LOG(DEBUG, "...'Mon+Discover' thread joined.");
    }

    if (m_killProcessesInDestructor)
    {
        deadline_timer terminationDelayTimer(m_processTerminationDelay);
        using namespace units::duration_literals;
        auto remainingDurationForInfoPrint = m_processTerminationDelay - 1_s;
        while (!terminationDelayTimer.hasExpired() && m_prcMgr->registeredProcessCount() > 0)
        {
            if (remainingDurationForInfoPrint > terminationDelayTimer.remainingTime())
            {
                IOX_LOG(WARN,
                        "Some applications seem to be still running! Time until graceful shutdown: "
                            << terminationDelayTimer.remainingTime().toSeconds() << "s!");
                remainingDurationForInfoPrint = remainingDurationForInfoPrint - 5_s;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(PROCESS_TERMINATED_CHECK_INTERVAL.toMilliseconds()));
        }

        m_prcMgr->requestShutdownOfAllProcesses();

        deadline_timer finalKillTimer(m_processKillDelay);
        auto remainingDurationForWarnPrint = m_processKillDelay - 2_s;
        while (m_prcMgr->probeRegisteredProcessesAliveWithSigTerm() && !finalKillTimer.hasExpired())
        {
            if (remainingDurationForWarnPrint > finalKillTimer.remainingTime())
            {
                IOX_LOG(WARN,
                        "Some applications seem to not shutdown gracefully! Time until hard shutdown: "
                            << finalKillTimer.remainingTime().toSeconds() << "s!");
                remainingDurationForWarnPrint = remainingDurationForWarnPrint - 5_s;
            }
            // give processes some time to terminate
            std::this_thread::sleep_for(std::chrono::milliseconds(PROCESS_TERMINATED_CHECK_INTERVAL.toMilliseconds()));
        }

        // Is any processes still alive?
        if (m_prcMgr->probeRegisteredProcessesAliveWithSigTerm() && finalKillTimer.hasExpired())
        {
            // Time to kill them
            m_prcMgr->killAllProcesses();
        }

        if (m_prcMgr->probeRegisteredProcessesAliveWithSigTerm())
        {
            m_prcMgr->printWarningForRegisteredProcessesAndClearProcessList();
        }
    }

    // Postpone the IpcChannelThread in order to receive TERMINATION
    m_runHandleRuntimeMessageThread = false;

    if (m_handleRuntimeMessageThread.joinable())
    {
        IOX_LOG(DEBUG, "Joining 'IPC-msg-process' thread...");
        m_handleRuntimeMessageThread.join();
        IOX_LOG(DEBUG, "...'IPC-msg-process' thread joined.");
    }
}

void RouDi::cyclicUpdateHook() noexcept
{
    // default implementation; do nothing
}

void RouDi::triggerDiscoveryLoopAndWaitToFinish(units::Duration timeout) noexcept
{
    bool decrementSemaphoreCount{true};
    while (decrementSemaphoreCount)
    {
        m_discoveryFinishedSemaphore->tryWait()
            .and_then([&decrementSemaphoreCount](const auto& countNonZero) { decrementSemaphoreCount = countNonZero; })
            .or_else([&decrementSemaphoreCount](const auto& error) {
                decrementSemaphoreCount = false;
                IOX_LOG(ERROR,
                        "Could not decrement count of the semaphore which signals a finished run of the "
                        "discovery loop! Error: "
                            << static_cast<uint32_t>(error));
            });
    }
    m_discoveryLoopTrigger.trigger();
    m_discoveryFinishedSemaphore->timedWait(timeout).or_else([](const auto& error) {
        IOX_LOG(ERROR,
                "A timed wait on the semaphore which signals a finished run of the "
                "discovery loop failed! Error: "
                    << static_cast<uint32_t>(error));
    });
}

void RouDi::monitorAndDiscoveryUpdate() noexcept
{
    class DiscoveryWaitSet : public popo::WaitSet<1>
    {
      public:
        DiscoveryWaitSet(popo::ConditionVariableData& condVarData) noexcept
            : WaitSet(condVarData)
        {
        }
    };

    popo::ConditionVariableData conditionVariableData;
    DiscoveryWaitSet discoveryLoopWaitset{conditionVariableData};
    discoveryLoopWaitset.attachEvent(m_discoveryLoopTrigger).expect("Failed to attach a single event");
    bool manuallyTriggered{false};

    while (m_runMonitoringAndDiscoveryThread)
    {
        m_prcMgr->run();

        cyclicUpdateHook();

        if (manuallyTriggered)
        {
            m_discoveryFinishedSemaphore->post().or_else([](const auto& error) {
                IOX_LOG(ERROR,
                        "Could not trigger semaphore to signal a finished run of the discovery loop! Error: "
                            << static_cast<uint32_t>(error));
            });
        }

        manuallyTriggered = false;
        for (const auto& notification : discoveryLoopWaitset.timedWait(DISCOVERY_INTERVAL))
        {
            if (notification->doesOriginateFrom(&m_discoveryLoopTrigger))
            {
                manuallyTriggered = true;
                break;
            }
        }
    }
}

void RouDi::processRuntimeMessages() noexcept
{
    runtime::IpcInterfaceCreator roudiIpcInterface{IPC_CHANNEL_ROUDI_NAME};

    IOX_LOG(INFO, "RouDi is ready for clients");
    fflush(stdout); // explicitly flush 'stdout' for 'launch_testing'

    while (m_runHandleRuntimeMessageThread)
    {
        // read RouDi's IPC channel
        runtime::IpcMessage message;
        if (roudiIpcInterface.timedReceive(m_runtimeMessagesThreadTimeout, message))
        {
            auto cmd = runtime::stringToIpcMessageType(message.getElementAtIndex(0).c_str());
            RuntimeName_t runtimeName{into<lossy<RuntimeName_t>>(message.getElementAtIndex(1))};

            processMessage(message, cmd, runtimeName);
        }
    }
}

version::VersionInfo RouDi::parseRegisterMessage(const runtime::IpcMessage& message,
                                                 uint32_t& pid,
                                                 uid_t& userId,
                                                 int64_t& transmissionTimestamp) noexcept
{
    cxx::convert::fromString(message.getElementAtIndex(2).c_str(), pid);
    cxx::convert::fromString(message.getElementAtIndex(3).c_str(), userId);
    cxx::convert::fromString(message.getElementAtIndex(4).c_str(), transmissionTimestamp);
    cxx::Serialization serializationVersionInfo(message.getElementAtIndex(5));
    return serializationVersionInfo;
}

void RouDi::processMessage(const runtime::IpcMessage& message,
                           const iox::runtime::IpcMessageType& cmd,
                           const RuntimeName_t& runtimeName) noexcept
{
    switch (cmd)
    {
    case runtime::IpcMessageType::REG:
    {
        if (message.getNumberOfElements() != 6)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::REG\" from \"" << runtimeName << "\"received!");
        }
        else
        {
            uint32_t pid{0U};
            uid_t userId{0};
            int64_t transmissionTimestamp{0};
            version::VersionInfo versionInfo = parseRegisterMessage(message, pid, userId, transmissionTimestamp);

            registerProcess(runtimeName,
                            pid,
                            iox::posix::PosixUser{userId},
                            transmissionTimestamp,
                            getUniqueSessionIdForProcess(),
                            versionInfo);
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_PUBLISHER:
    {
        if (message.getNumberOfElements() != 5)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_PUBLISHER\" from \"" << runtimeName
                                                                                                  << "\"received!");
        }
        else
        {
            auto deserializationResult =
                capro::ServiceDescription::deserialize(cxx::Serialization(message.getElementAtIndex(2)));
            if (deserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization failed when '" << message.getElementAtIndex(2).c_str() << "' was provided\n");
                break;
            }
            const auto& service = deserializationResult.value();

            auto publisherOptionsDeserializationResult =
                popo::PublisherOptions::deserialize(cxx::Serialization(message.getElementAtIndex(3)));
            if (publisherOptionsDeserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization of 'PublisherOptions' failed when '" << message.getElementAtIndex(3).c_str()
                                                                              << "' was provided\n");
                break;
            }
            const auto& publisherOptions = publisherOptionsDeserializationResult.value();

            cxx::Serialization portConfigInfoSerialization(message.getElementAtIndex(4));

            m_prcMgr->addPublisherForProcess(
                runtimeName, service, publisherOptions, iox::runtime::PortConfigInfo(portConfigInfoSerialization));
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_SUBSCRIBER:
    {
        if (message.getNumberOfElements() != 5)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_SUBSCRIBER\" from \"" << runtimeName
                                                                                                   << "\"received!");
        }
        else
        {
            auto deserializationResult =
                capro::ServiceDescription::deserialize(cxx::Serialization(message.getElementAtIndex(2)));
            if (deserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization failed when '" << message.getElementAtIndex(2).c_str() << "' was provided\n");
                break;
            }

            const auto& service = deserializationResult.value();

            auto subscriberOptionsDeserializationResult =
                popo::SubscriberOptions::deserialize(cxx::Serialization(message.getElementAtIndex(3)));
            if (subscriberOptionsDeserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization of 'SubscriberOptions' failed when '" << message.getElementAtIndex(3).c_str()
                                                                               << "' was provided\n");
                break;
            }
            const auto& subscriberOptions = subscriberOptionsDeserializationResult.value();

            cxx::Serialization portConfigInfoSerialization(message.getElementAtIndex(4));

            m_prcMgr->addSubscriberForProcess(
                runtimeName, service, subscriberOptions, iox::runtime::PortConfigInfo(portConfigInfoSerialization));
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_CLIENT:
    {
        if (message.getNumberOfElements() != 5)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_CLIENT\" from \"" << runtimeName
                                                                                               << "\"received!");
        }
        else
        {
            auto deserializationResult =
                capro::ServiceDescription::deserialize(cxx::Serialization(message.getElementAtIndex(2)));
            if (deserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization failed when '" << message.getElementAtIndex(2).c_str() << "' was provided\n");
                break;
            }

            const auto& service = deserializationResult.value();

            auto clientOptionsDeserializationResult =
                popo::ClientOptions::deserialize(cxx::Serialization(message.getElementAtIndex(3)));
            if (clientOptionsDeserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization of 'ClientOptions' failed when '" << message.getElementAtIndex(3).c_str()
                                                                           << "' was provided\n");
                break;
            }
            const auto& clientOptions = clientOptionsDeserializationResult.value();

            runtime::PortConfigInfo portConfigInfo{cxx::Serialization(message.getElementAtIndex(4))};

            m_prcMgr->addClientForProcess(runtimeName, service, clientOptions, portConfigInfo);
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_SERVER:
    {
        if (message.getNumberOfElements() != 5)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_SERVER\" from \"" << runtimeName
                                                                                               << "\"received!");
        }
        else
        {
            auto deserializationResult =
                capro::ServiceDescription::deserialize(cxx::Serialization(message.getElementAtIndex(2)));
            if (deserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization failed when '" << message.getElementAtIndex(2).c_str() << "' was provided\n");
                break;
            }

            const auto& service = deserializationResult.value();

            auto serverOptionsDeserializationResult =
                popo::ServerOptions::deserialize(cxx::Serialization(message.getElementAtIndex(3)));
            if (serverOptionsDeserializationResult.has_error())
            {
                IOX_LOG(ERROR,
                        "Deserialization of 'ServerOptions' failed when '" << message.getElementAtIndex(3).c_str()
                                                                           << "' was provided\n");
                break;
            }
            const auto& serverOptions = serverOptionsDeserializationResult.value();

            runtime::PortConfigInfo portConfigInfo{cxx::Serialization(message.getElementAtIndex(4))};

            m_prcMgr->addServerForProcess(runtimeName, service, serverOptions, portConfigInfo);
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_CONDITION_VARIABLE:
    {
        if (message.getNumberOfElements() != 2)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_CONDITION_VARIABLE\" from \""
                        << runtimeName << "\"received!");
        }
        else
        {
            m_prcMgr->addConditionVariableForProcess(runtimeName);
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_INTERFACE:
    {
        if (message.getNumberOfElements() != 4)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_INTERFACE\" from \"" << runtimeName
                                                                                                  << "\"received!");
        }
        else
        {
            capro::Interfaces interface =
                StringToCaProInterface(into<lossy<capro::IdString_t>>(message.getElementAtIndex(2)));

            m_prcMgr->addInterfaceForProcess(
                runtimeName, interface, into<lossy<NodeName_t>>(message.getElementAtIndex(3)));
        }
        break;
    }
    case runtime::IpcMessageType::CREATE_NODE:
    {
        if (message.getNumberOfElements() != 3)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::CREATE_NODE\" from \"" << runtimeName
                                                                                             << "\"received!");
        }
        else
        {
            runtime::NodeProperty nodeProperty(cxx::Serialization(message.getElementAtIndex(2)));
            m_prcMgr->addNodeForProcess(runtimeName, nodeProperty.m_name);
        }
        break;
    }
    case runtime::IpcMessageType::PREPARE_APP_TERMINATION:
    {
        if (message.getNumberOfElements() != 2)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::PREPARE_APP_TERMINATION\" from \""
                        << runtimeName << "\"received!");
        }
        else
        {
            // this is used to unblock a potentially block application by blocking publisher
            m_prcMgr->handleProcessShutdownPreparationRequest(runtimeName);
        }
        break;
    }
    case runtime::IpcMessageType::TERMINATION:
    {
        if (message.getNumberOfElements() != 2)
        {
            IOX_LOG(ERROR,
                    "Wrong number of parameters for \"IpcMessageType::TERMINATION\" from \"" << runtimeName
                                                                                             << "\"received!");
        }
        else
        {
            IOX_DISCARD_RESULT(m_prcMgr->unregisterProcess(runtimeName));
        }
        break;
    }
    default:
    {
        IOX_LOG(ERROR, "Unknown IPC message command [" << runtime::IpcMessageTypeToString(cmd) << "]");

        m_prcMgr->sendMessageNotSupportedToRuntime(runtimeName);
        break;
    }
    }
}

void RouDi::registerProcess(const RuntimeName_t& name,
                            const uint32_t pid,
                            const posix::PosixUser user,
                            const int64_t transmissionTimestamp,
                            const uint64_t sessionId,
                            const version::VersionInfo& versionInfo) noexcept
{
    bool monitorProcess = (m_monitoringMode == roudi::MonitoringMode::ON);
    IOX_DISCARD_RESULT(
        m_prcMgr->registerProcess(name, pid, user, monitorProcess, transmissionTimestamp, sessionId, versionInfo));
}

uint64_t RouDi::getUniqueSessionIdForProcess() noexcept
{
    static uint64_t sessionId = 0;
    return ++sessionId;
}

void RouDi::IpcMessageErrorHandler() noexcept
{
}

} // namespace roudi
} // namespace iox
