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
RouDiProcess::RouDiProcess(std::string f_name,
                           int f_pid,
                           mepoo::MemoryManager* f_payloadMemoryManager,
                           bool isMonitored,
                           const uint64_t f_payloadSegmentId)
    : m_pid(f_pid)
    , m_mq(f_name)
    , m_timestamp(mepoo::BaseClock::now())
    , m_payloadMemoryManager(f_payloadMemoryManager)
    , m_isMonitored(isMonitored)
    , m_payloadSegmentId(f_payloadSegmentId)
{
}

int RouDiProcess::getPid() const
{
    return m_pid;
}

const std::string& RouDiProcess::getName() const
{
    return m_mq.getInterfaceName();
}

void RouDiProcess::sendToMQ(const runtime::MqMessage& f_data)
{
    m_mq.send(f_data);
}

void RouDiProcess::setTimestamp(const mepoo::TimePointNs f_timestamp)
{
    m_timestamp = f_timestamp;
}

mepoo::TimePointNs RouDiProcess::getTimestamp()
{
    return m_timestamp;
}

mepoo::MemoryManager* RouDiProcess::getPayloadMemoryManager() const
{
    return m_payloadMemoryManager;
}

uint64_t RouDiProcess::getPayloadSegmentId() const
{
    return m_payloadSegmentId;
}

bool RouDiProcess::isMonitored() const
{
    return m_isMonitored;
}

//--------------------------------------------------------------------------------------------------

ProcessManager::ProcessManager(SharedMemoryManager& f_shmMgr)
    : m_shmMgr(f_shmMgr)
{
    auto currentUser = posix::PosixUser::getUserOfCurrentProcess();
    auto m_segmentInfo =
        m_shmMgr.getShmInterface().getShmInterface()->m_segmentManager.getSegmentInformationForUser(currentUser);
    m_memoryManagerOfCurrentProcess = m_segmentInfo.m_memoryManager;
    m_segmentIdOfCurrentProcess = m_segmentInfo.m_segmentID;
}

void ProcessManager::killAllProcesses()
{
    std::lock_guard<std::mutex> g(m_mutex);

    // send SIGTERM to all running applications
    typename ProcessList_t::iterator l_it = m_processList.begin();
    const typename ProcessList_t::iterator l_itEnd = m_processList.end();

    for (; l_itEnd != l_it; ++l_it)
    {
        if (-1 == kill(static_cast<pid_t>(l_it->getPid()), 15))
        {
            WARN_PRINTF("Process %d could not be killed\n", l_it->getPid());
        }
    }
}

bool ProcessManager::registerProcess(
    const std::string& f_name, int f_pid, posix::PosixUser f_user, bool f_isMonitored, int64_t transmissionTimestamp)
{
    bool isMonitored; // must be in outer scope but is only initialized before use
    bool processExists = false;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        auto process = getProcessFromList(f_name); // process existence check
        if (process)
        {
            processExists = true;
            isMonitored = process->isMonitored(); // needs to be read here under lock
        }
        else
        {
            isMonitored = false; // does not really matter (only for static analysis - avoid unitialized var)
        }
    }
    // lock is not required anymore


    auto segmentInfo =
        m_shmMgr.getShmInterface().getShmInterface()->m_segmentManager.getSegmentInformationForUser(f_user);

    if (!processExists)
    {
        // process does not exist in list and can be added
        return addProcess(
            f_name, f_pid, segmentInfo.m_memoryManager, f_isMonitored, transmissionTimestamp, segmentInfo.m_segmentID);
    }

    // process is already in list (i.e. registered)
    // depending on the mode we clean up the process resources and register it again
    // if it is monitored, we reject the registration and wait for automatic cleanup
    // otherwise we remove the process ourselves and register it again

    if (isMonitored)
    {
        // process exists and is monitored - we rely on monitoring for removal
        WARN_PRINTF("Received REG from %s, but another application with this "
                    "name is already registered\n",
                    f_name.c_str());
    }
    else
    {
        // process exists and is not monitored - remove it and add the new process afterwards
        DEBUG_PRINTF("Registering already existing application %s\n", f_name.c_str());

        // remove existing process
        if (!removeProcess(f_name)) // call will acquire lock
        {
            WARN_PRINTF("Received REG from %s, but another application with this "
                        "name is already registered and could not be removed\n",
                        f_name.c_str());
            return false;
        }

        DEBUG_PRINTF("Registering already existing application %s - removed existing application\n", f_name.c_str());

        // try registration again, should succeed since removal was successful
        return addProcess(f_name,
                          f_pid,
                          segmentInfo.m_memoryManager,
                          f_isMonitored,
                          transmissionTimestamp,
                          segmentInfo.m_segmentID); // call will acquire lock
    }

    return false;
}

bool ProcessManager::addProcess(const std::string& f_name,
                                int f_pid,
                                mepoo::MemoryManager* f_payloadMemoryManager,
                                bool f_isMonitored,
                                int64_t transmissionTimestamp,
                                const uint64_t f_segmentId)
{
    std::lock_guard<std::mutex> g(m_mutex);
    // overflow check
    if (m_processList.size() >= MAX_PROCESS_NUMBER)
    {
        LOG_ERR("Could not register process - too many processes");
        return false;
    }

    m_processList.emplace_back(f_name, f_pid, f_payloadMemoryManager, f_isMonitored, f_segmentId);

    // send REG_ACK and BaseAddrString
    runtime::MqMessage l_sendBuffer;

    auto segmentId = m_shmMgr.getShmInterface().getShmInterface()->m_segmentId;
    auto offset =
        RelativePointer::getOffset(segmentId, &m_shmMgr.getShmInterface().getShmInterface()->m_segmentManager);
    l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::REG_ACK) << m_shmMgr.GetShmAddrString()
                 << m_shmMgr.getShmSizeInBytes() << offset << transmissionTimestamp << segmentId;

    m_processList.back().sendToMQ(l_sendBuffer);

    // set current timestamp again (already done in RouDiProcess's constructor
    m_processList.back().setTimestamp(mepoo::BaseClock::now());

    m_processIntrospection->addProcess(f_pid, cxx::CString100(f_name.c_str()));

    DEBUG_PRINTF("Registered new application %s\n", f_name.c_str());
    return true;
}

bool ProcessManager::removeProcess(const std::string& f_name)
{
    std::lock_guard<std::mutex> g(m_mutex);
    // we need to search for the process (currently linear search)

    auto it = m_processList.begin();
    while (it != m_processList.end())
    {
        auto name = it->getName();
        if (name == f_name)
        {
            m_shmMgr.deletePortsOfProcess(name);

            m_processIntrospection->removeProcess(it->getPid());

            // delete application
            it = m_processList.erase(it);

            DEBUG_PRINTF("New Registration - removed existing application %s\n", f_name.c_str());
            return true; // we can assume there are no other processes with this name
        }
        ++it;
    }
    return false;
}

bool ProcessManager::sendMessageToProcess(const std::string& f_name, const iox::runtime::MqMessage& f_message)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (l_process == nullptr)
    {
        WARN_PRINTF("Received message for unknown process %s\n", f_name.c_str());
        return false;
    }

    DEBUG_PRINTF("Send message to application %s\n", f_name.c_str());
    l_process->sendToMQ(f_message);

    return true;
}

void ProcessManager::updateLivlinessOfProcess(const std::string& f_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        // reset timestamp
        l_process->setTimestamp(mepoo::BaseClock::now());
    }
    else
    {
        WARN_PRINTF("Received Keepalive from unknown process %s\n ", f_name.c_str());
    }
}

void ProcessManager::findServiceForProcess(const std::string& f_name, const capro::ServiceDescription& f_service)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        runtime::MqMessage l_instanceString({m_shmMgr.findService(f_service)});
        l_process->sendToMQ(l_instanceString);
        DEBUG_PRINTF("Sent InstanceString to application %s\n", f_name.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown process %s requested an InstanceString.\n", f_name.c_str());
    }
}

void ProcessManager::addInterfaceForProcess(const std::string& f_name,
                                            Interfaces f_interface,
                                            const std::string& f_runnable)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        // create a ReceiverPort
        popo::InterfacePortData* l_port = m_shmMgr.acquireInterfacePortData(f_interface, f_name, f_runnable);

        // send ReceiverPort to app as a serialized relative pointer
        RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
        auto offset = RelativePointer::getOffset(segmentId, l_port);

        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::IMPL_INTERFACE_ACK)
                     << std::to_string(offset) << std::to_string(segmentId);
        l_process->sendToMQ(l_sendBuffer);

        DEBUG_PRINTF("Created new interface for application %s\n", f_name.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested an interface.\n", f_name.c_str());
    }
}

void ProcessManager::sendServiceRegistryChangeCounterToProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> g(m_mutex);
    RouDiProcess* l_process = getProcessFromList(processName);
    if (nullptr != l_process)
    {
        std::atomic<uint64_t>* counter = &m_shmMgr.getShmInterface().getShmInterface()->m_serviceRegistryChangeCounter;

        // send counter to app as a serialized relative pointer
        RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
        auto offset = RelativePointer::getOffset(segmentId, counter);

        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << std::to_string(offset) << std::to_string(segmentId);
        l_process->sendToMQ(l_sendBuffer);
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested an serviceRegistryChangeCounter.\n", processName.c_str());
    }
}

void ProcessManager::addApplicationForProcess(const std::string& f_name, Interfaces f_interface)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        popo::ApplicationPortData* l_port = m_shmMgr.acquireApplicationPortData(f_interface, f_name);

        RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
        auto offset = RelativePointer::getOffset(segmentId, l_port);

        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::IMPL_APPLICATION_ACK)
                     << std::to_string(offset) << std::to_string(segmentId);
        l_process->sendToMQ(l_sendBuffer);

        DEBUG_PRINTF("Created new ApplicationPort for application %s\n", f_name.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested an ApplicationPort.\n", f_name.c_str());
    }
}

void ProcessManager::addRunnableForProcess(const std::string& f_process, const std::string& f_runnable)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_process);
    if (nullptr != l_process)
    {
        runtime::RunnableData* l_runnable = m_shmMgr.acquireRunnableData(f_process, f_runnable);

        RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
        auto offset = RelativePointer::getOffset(segmentId, l_runnable);

        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::CREATE_RUNNABLE_ACK)
                        << std::to_string(offset) << std::to_string(segmentId);

        l_process->sendToMQ(l_sendBuffer);
        m_processIntrospection->addRunnable(cxx::CString100(f_process.c_str()), cxx::CString100(f_runnable.c_str()));
        DEBUG_PRINTF("Created new runnable %s for application %s\n", f_runnable.c_str(), f_process.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested a runnable.\n", f_process.c_str());
    }
}

void ProcessManager::removeRunnableForProcess(const std::string& f_process, const std::string& f_runnable)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_process);
    if (nullptr != l_process)
    {
        m_processIntrospection->removeRunnable(cxx::CString100(f_process.c_str()), cxx::CString100(f_runnable.c_str()));
        m_shmMgr.deleteRunnableAndItsPorts(f_runnable);
        DEBUG_PRINTF("Removed runnable of application %s\n", f_process.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested runnable remove.\n", f_process.c_str());
    }
}


void ProcessManager::sendMessageNotSupportedToRuntime(const std::string& f_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::MESSAGE_NOT_SUPPORTED);
        l_process->sendToMQ(l_sendBuffer);

        ERR_PRINTF("Application %s sent a message, which is not supported by this RouDi\n", f_name.c_str());
    }
}

void ProcessManager::addReceiverForProcess(const std::string& f_name,
                                           const capro::ServiceDescription& f_service,
                                           Interfaces f_interface,
                                           const std::string& f_runnable)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        // create a ReceiverPort
        ReceiverPortType::MemberType_t* l_receiver =
            m_shmMgr.acquireReceiverPortData(f_service, f_interface, f_name, f_runnable);

        // send ReceiverPort to app as a serialized relative pointer
        RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
        auto offset = RelativePointer::getOffset(segmentId, l_receiver);

        runtime::MqMessage l_sendBuffer;
        l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::IMPL_RECEIVER_ACK)
                     << std::to_string(offset) << std::to_string(segmentId);
        l_process->sendToMQ(l_sendBuffer);

        DEBUG_PRINTF("Created new ReceiverPortImpl for application %s\n", f_name.c_str());
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested a ReceiverPortImpl.\n", f_name.c_str());
    }
}

void ProcessManager::addSenderForProcess(const std::string& f_name,
                                         const capro::ServiceDescription& f_service,
                                         Interfaces f_interface,
                                         const std::string& f_runnable)
{
    std::lock_guard<std::mutex> g(m_mutex);

    RouDiProcess* l_process = getProcessFromList(f_name);
    if (nullptr != l_process)
    {
        // create a SenderPort
        SenderPortType::MemberType_t* l_sender = m_shmMgr.acquireSenderPortData(
            f_service, f_interface, f_name, l_process->getPayloadMemoryManager(), f_runnable);

        if (nullptr != l_sender)
        {
            // send SenderPort to app as a serialized relative pointer
            RelativePointer::id_t segmentId = m_shmMgr.getShmInterface().getSegmentId();
            auto offset = RelativePointer::getOffset(segmentId, l_sender);

            runtime::MqMessage l_sendBuffer;
            l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::IMPL_SENDER_ACK)
                         << std::to_string(offset) << std::to_string(segmentId);
            l_process->sendToMQ(l_sendBuffer);

            DEBUG_PRINTF("Created new SenderPortImpl for application %s\n", f_name.c_str());
        }
        else
        {
            runtime::MqMessage l_sendBuffer;
            l_sendBuffer << runtime::mqMessageTypeToString(runtime::MqMessageType::ERROR);
            l_process->sendToMQ(l_sendBuffer);
            ERR_PRINTF("Could not create SenderPortImpl for application %s\n", f_name.c_str());
        }
    }
    else
    {
        WARN_PRINTF("Unknown application %s requested a SenderPortImpl.\n", f_name.c_str());
    }
}

void ProcessManager::initIntrospection(ProcessIntrospectionType* f_processIntrospection)
{
    m_processIntrospection = f_processIntrospection;
}

void ProcessManager::run()
{
    monitorProcesses();
    discoveryUpdate();
    std::this_thread::sleep_for(std::chrono::milliseconds(DISCOVERY_INTERVAL.milliSeconds<int64_t>()));
}

SenderPortType ProcessManager::addIntrospectionSenderPort(const capro::ServiceDescription& f_service,
                                                          const std::string& f_process_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    return SenderPortType(
        m_shmMgr.acquireSenderPortData(f_service,
                                       Interfaces::INTERNAL,
                                       f_process_name,
                                       &m_shmMgr.getShmInterface().getShmInterface()->m_roudiMemoryManager));
}

ReceiverPortType ProcessManager::addInternalReceiverPort(const capro::ServiceDescription& f_service,
                                                         const std::string& f_process_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    return ReceiverPortType(m_shmMgr.acquireReceiverPortData(f_service, Interfaces::INTERNAL, f_process_name));
}

void ProcessManager::removeInternalPorts(const std::string& f_process_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    m_shmMgr.deletePortsOfProcess(f_process_name);
}


bool ProcessManager::areAllReceiverPortsSubscribed(const std::string& f_process_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    return m_shmMgr.areAllReceiverPortsSubscribed(f_process_name);
}

SenderPortType ProcessManager::addInternalSenderPort(const capro::ServiceDescription& f_service,
                                                     const std::string& f_process_name)
{
    std::lock_guard<std::mutex> g(m_mutex);

    return SenderPortType(m_shmMgr.acquireSenderPortData(
        f_service, Interfaces::INTERNAL, f_process_name, m_memoryManagerOfCurrentProcess));
}


RouDiProcess* ProcessManager::getProcessFromList(const std::string& f_name)
{
    RouDiProcess* l_processPtr = nullptr;

    typename ProcessList_t::iterator l_it = m_processList.begin();
    const typename ProcessList_t::iterator l_itEnd = m_processList.end();

    for (; l_itEnd != l_it; ++l_it)
    {
        if (f_name == l_it->getName())
        {
            l_processPtr = &(*l_it);
            break;
        }
    }

    return l_processPtr;
}

void ProcessManager::monitorProcesses()
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
                WARN_PRINTF("Application %s not responding (last response %d milliseconds ago) --> removing it\n",
                            processIterator->getName().c_str(),
                            timediff_ms);

                // note: if we would want to use the removeProcess function, it would search for the process again (but
                // we already found it and have an iterator to remove it)

                // delete all associated receiver and sender impl in shared
                // memory and the associated RouDi discovery ports
                // @todo Check if ShmManager and Process Manager end up in unintended condition
                m_shmMgr.deletePortsOfProcess(processIterator->getName());

                m_processIntrospection->removeProcess(processIterator->getPid());

                // delete application
                processIterator = m_processList.erase(processIterator);
                continue; // erase returns first element after the removed one --> skip iterator increment
            }
        }
        ++processIterator;
    }
}

void ProcessManager::discoveryUpdate()
{
    std::lock_guard<std::mutex> g(m_mutex);

    m_shmMgr.doDiscovery();
}

} // namespace roudi
} // namespace iox
