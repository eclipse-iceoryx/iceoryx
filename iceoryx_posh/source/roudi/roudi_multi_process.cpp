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

#include "iceoryx_posh/internal/roudi/roudi_multi_process.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/runnable_property.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

namespace iox
{
namespace roudi
{
RouDiMultiProcess::RouDiMultiProcess(RouDiApp::MonitoringMode monitoringMode,
                                     const bool killProcessesInDestructor,
                                     const RouDiConfig_t config)
    : m_killProcessesInDestructor(killProcessesInDestructor)
    , m_runThreads(true)
    , m_roudilock()
    , m_cleanupBeforeStart(cleanupBeforeStart())
    , m_shmMgr(config)
    , m_prcMgr(m_shmMgr)
    , m_mempoolIntrospection(
          m_shmMgr.getShmInterface().getShmInterface()->m_roudiMemoryManager,
          m_shmMgr.getShmInterface().getShmInterface()->m_segmentManager,
          m_prcMgr.addIntrospectionSenderPort(IntrospectionMempoolService, MEMPOOL_INTROSPECTION_MQ_APP_NAME))
    , m_monitoringMode(monitoringMode)
{
    m_processIntrospection.registerSenderPort(
        m_prcMgr.addIntrospectionSenderPort(IntrospectionProcessService, PROCESS_INTROSPECTION_MQ_APP_NAME));
    m_prcMgr.initIntrospection(&m_processIntrospection);
    m_processIntrospection.run();
    m_mempoolIntrospection.start();

    // run the threads
    m_processManagementThread = std::thread(&RouDiMultiProcess::processThread, this);
    pthread_setname_np(m_processManagementThread.native_handle(), "ProcessMgmt");

    m_processMQThread = std::thread(&RouDiMultiProcess::mqThread, this);
    pthread_setname_np(m_processMQThread.native_handle(), "MQ-processing");

#ifdef PRINT_MEMORY_CONSUMPTION
    INFO_PRINTF("-----------------------\n");
    INFO_PRINTF("Static Sizes [kB]:\n");
    INFO_PRINTF("* RouDiMultiProcess ~ %d kB\n", (int)((sizeof(RouDiMultiProcess) / 1024.) + .5));
    INFO_PRINTF("* SharedMemoryManager    ~ %6d kB\n", (int)((sizeof(SharedMemoryManager) / 1024.) + .5));
    INFO_PRINTF("* ProcessManager    ~ %6d kB\n", (int)((sizeof(ProcessManager) / 1024.) + .5));
    INFO_PRINTF("-----------------------\n");
#endif
}

RouDiMultiProcess::~RouDiMultiProcess()
{
    shutdown();
}

void RouDiMultiProcess::shutdown()
{
    m_processIntrospection.stop();
    m_shmMgr.stopPortIntrospection();
    // roudi will exit soon, stopping all threads
    m_runThreads = false;

    if (m_killProcessesInDestructor)
    {
        m_prcMgr.killAllProcesses();
    }

    if (m_processManagementThread.joinable())
    {
        LOG_DEBUG("Joining 'ProcessMgmt' thread...");
        m_processManagementThread.join();
        LOG_DEBUG("...'ProcessMgmt' thread joined.");
    }
    if (m_processMQThread.joinable())
    {
        LOG_DEBUG("Joining 'MQ-processing' thread...");
        m_processMQThread.join();
        LOG_DEBUG("...'MQ-processing' thread joined.");
    }
}

void RouDiMultiProcess::cyclicUpdateHook()
{
    // default implementation; do nothing
}

void RouDiMultiProcess::processThread()
{
    while (m_runThreads)
    {
        m_prcMgr.run();

        cyclicUpdateHook();
    }
}

void RouDiMultiProcess::mqThread()
{
    runtime::MqInterfaceCreator roudiMqInterface{MQ_ROUDI_NAME};
    while (m_runThreads)
    {
        // read RouDi message queue
        runtime::MqMessage message;
        /// @todo do we really need timedReceive? an alternative solution would be to close the message queue,
        /// which also results in a return from mq_receive, and check the relevant errno and shutdown RouDi
        if (!roudiMqInterface.timedReceive(m_MessageQueueTimeoutMilliseconds, message))
        {
            // TODO: errorHandling
        }
        else
        {
            auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
            std::string processName = message.getElementAtIndex(1);

            processMessage(message, cmd, processName);
        }
    }
}

void RouDiMultiProcess::parseRegisterMessage(const runtime::MqMessage& message,
                                             int& pid,
                                             uid_t& userId,
                                             int64_t& transmissionTimestamp)
{
    cxx::convert::fromString(message.getElementAtIndex(2).c_str(), pid);
    cxx::convert::fromString(message.getElementAtIndex(3).c_str(), userId);
    cxx::convert::fromString(message.getElementAtIndex(4).c_str(), transmissionTimestamp);
}


void RouDiMultiProcess::processMessage(const runtime::MqMessage& message,
                                       const iox::runtime::MqMessageType& cmd,
                                       const std::string& processName)
{
    switch (cmd)
    {
    case runtime::MqMessageType::SERVICE_REGISTRY_CHANGE_COUNTER:
    {
        m_prcMgr.sendServiceRegistryChangeCounterToProcess(processName);
        break;
    }
    case runtime::MqMessageType::REG:
    {
        if (message.getNumberOfElements() != 5)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::REG\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            int pid;
            uid_t userId;
            int64_t transmissionTimestamp;

            parseRegisterMessage(message, pid, userId, transmissionTimestamp);

            registerProcess(processName, pid, {userId}, transmissionTimestamp);
        }
        break;
    }
    case runtime::MqMessageType::IMPL_SENDER:
    {
        if (message.getNumberOfElements() != 5)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::IMPL_SENDER\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            capro::ServiceDescription service(cxx::Serialization(message.getElementAtIndex(2)));
            Interfaces interface = StringToEInterfaces(message.getElementAtIndex(3));

            m_prcMgr.addSenderForProcess(processName, service, interface, message.getElementAtIndex(4));
        }
        break;
    }
    case runtime::MqMessageType::IMPL_RECEIVER:
    {
        if (message.getNumberOfElements() != 5)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::IMPL_RECEIVER\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            capro::ServiceDescription service(cxx::Serialization(message.getElementAtIndex(2)));
            Interfaces interface = StringToEInterfaces(message.getElementAtIndex(3));

            m_prcMgr.addReceiverForProcess(processName, service, interface, message.getElementAtIndex(4));
        }
        break;
    }
    case runtime::MqMessageType::IMPL_INTERFACE:
    {
        if (message.getNumberOfElements() != 4)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::IMPL_INTERFACE\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            Interfaces interface = StringToEInterfaces(message.getElementAtIndex(2));

            m_prcMgr.addInterfaceForProcess(processName, interface, message.getElementAtIndex(3));
        }
        break;
    }
    case runtime::MqMessageType::IMPL_APPLICATION:
    {
        if (message.getNumberOfElements() != 3)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::IMPL_APPLICATION\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            Interfaces interface = StringToEInterfaces(message.getElementAtIndex(2));

            m_prcMgr.addApplicationForProcess(processName, interface);
        }
        break;
    }
    case runtime::MqMessageType::CREATE_RUNNABLE:
    {
        if (message.getNumberOfElements() != 3)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::CREATE_RUNNABLE\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            runtime::RunnableProperty runnableProperty(message.getElementAtIndex(2));
            m_prcMgr.addRunnableForProcess(processName, runnableProperty.m_name);
        }
        break;
    }
    case runtime::MqMessageType::REMOVE_RUNNABLE:
    {
        if (message.getNumberOfElements() != 3)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::REMOVE_RUNNABLE\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            m_prcMgr.removeRunnableForProcess(processName, message.getElementAtIndex(2));
        }
        break;
    }
    case runtime::MqMessageType::FIND_SERVICE:
    {
        if (message.getNumberOfElements() != 3)
        {
            ERR_PRINTF("Wrong number of parameter for \"MqMessageType::FIND_SERVICE\" from \"%s\"received!\n",
                       processName.c_str());
        }
        else
        {
            capro::ServiceDescription service(cxx::Serialization(message.getElementAtIndex(2)));

            m_prcMgr.findServiceForProcess(processName, service);
        }
        break;
    }
    case runtime::MqMessageType::KEEPALIVE:
    {
        m_prcMgr.updateLivlinessOfProcess(processName);
        break;
    }
    default:
    {
        ERR_PRINTF("Unknown MQ Command [%s]\n", runtime::mqMessageTypeToString(cmd).c_str());

        m_prcMgr.sendMessageNotSupportedToRuntime(processName);
        break;
    }
    }
}

bool RouDiMultiProcess::registerProcess(const std::string& name,
                                        int pid,
                                        posix::PosixUser user,
                                        int64_t transmissionTimestamp)
{
    bool monitorProcess = (m_monitoringMode == RouDiApp::MonitoringMode::ON);

    return m_prcMgr.registerProcess(name, pid, user, monitorProcess, transmissionTimestamp);
}

bool RouDiMultiProcess::cleanupBeforeStart()
{
    // this temporary object will create a roudi mqueue and close it immediatelly
    // if there was an outdated roudi message queue, it will be cleaned up
    // if there is an outdated mqueue, the start of the apps will be terminated
    runtime::MqBase::cleanupOutdatedMessageQueue(MQ_ROUDI_NAME);
    return true;
}

void RouDiMultiProcess::mqMessageErrorHandler()
{
}

} // namespace roudi
} // namespace iox
