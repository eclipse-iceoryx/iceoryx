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

#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/runtime/runnable.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToEInterfaces(std::string f_str)
{
    int32_t i;
    cxx::convert::fromString(f_str.c_str(), i);
    if (i >= static_cast<int32_t>(capro::Interfaces::INTERFACE_END))
    {
        WARN_PRINTF("invalid enum (out of range: %d) \n", i);
        return capro::Interfaces::INTERNAL;
    }
    return static_cast<capro::Interfaces>(i);
}

PortManager::PortManager(RouDiMemoryInterface* roudiMemoryInterface)
{
    m_roudiMemoryInterface = roudiMemoryInterface;

    auto maybePortPool = m_roudiMemoryInterface->portPool();
    if (!maybePortPool.has_value())
    {
        /// @todo errorHandler
        LogFatal() << "Could not get PortPool!";
        std::terminate();
    }
    m_portPool = maybePortPool.value();

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface->introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        /// @todo errorHandler
        LogFatal() << "Could not get MemoryManager for introspection!";
        std::terminate();
    }
    auto introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

    // Remark: m_portIntrospection is not fully functional in base class RouDiBase (has no active senderport)
    // are there used instances of RouDiBase?
    auto portGeneric =
        acquireSenderPortData(IntrospectionPortService, MQ_ROUDI_NAME, introspectionMemoryManager).get_value();

    auto portThroughput =
        acquireSenderPortData(IntrospectionPortThroughputService, MQ_ROUDI_NAME, introspectionMemoryManager)
            .get_value();

    auto receiverPortsData =
        acquireSenderPortData(IntrospectionReceiverPortChangingDataService, MQ_ROUDI_NAME, introspectionMemoryManager)
            .get_value();

    m_portIntrospection.registerSenderPort(portGeneric, portThroughput, receiverPortsData);
    m_portIntrospection.run();
}

void PortManager::stopPortIntrospection()
{
    m_portIntrospection.stop();
}

void PortManager::doDiscovery()
{
    handleSenderPorts();

    handleReceiverPorts();

    handleApplications();

    handleInterfaces();

    handleRunnables();
}

void PortManager::handleSenderPorts()
{
    // get the changes of sender port offer state
    for (auto l_senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType l_senderPort(l_senderPortData);
        auto returnedCaproMessage = l_senderPort.getCaProMessage();
        if (returnedCaproMessage.has_value())
        {
            auto& caproMessage = returnedCaproMessage.value();

            m_portIntrospection.reportMessage(caproMessage);

            if (capro::CaproMessageType::OFFER == caproMessage.m_type)
            {
                addEntryToServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                          caproMessage.m_serviceDescription.getInstanceIDString());

                sendToAllMatchingReceiverPorts(caproMessage, l_senderPort);
            }
            else if (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type)
            {
                removeEntryFromServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                               caproMessage.m_serviceDescription.getInstanceIDString());

                sendToAllMatchingReceiverPorts(caproMessage, l_senderPort);
            }
            else
            {
                // protocol error
                assert(false);
            }

            // forward to interfaces
            sendToAllMatchingInterfacePorts(caproMessage);
        }
        // check if we have to destroy this sender port
        if (l_senderPort.toBeDestroyed())
        {
            destroySenderPort(l_senderPortData);
        }
    }
}

void PortManager::handleReceiverPorts()
{
    // get requests for change of subscription state of receivers
    for (auto l_receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType l_receiverPort(l_receiverPortData);
        auto returnedCaproMessage = l_receiverPort.getCaProMessage();
        if (returnedCaproMessage.has_value())
        {
            auto& caproMessage = returnedCaproMessage.value();

            m_portIntrospection.reportMessage(caproMessage);

            if (!sendToAllMatchingSenderPorts(caproMessage, l_receiverPort))
            {
                DEBUG_PRINTF("capro::SUB/UNSUB, no matching sender!!\n");
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK,
                                                l_receiverPort.getCaProServiceDescription());
                l_receiverPort.dispatchCaProMessage(nackMessage);
            }
        }
        // check if we have to destroy this sender port
        if (l_receiverPort.toBeDestroyed())
        {
            destroyReceiverPort(l_receiverPortData);
        }
    }
}

void PortManager::handleInterfaces()
{
    // check if there are new interfaces that must get an initial offer information
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> l_interfacePortsForInitialForwarding;


    for (auto l_interfacePortData : m_portPool->interfacePortDataList())
    {
        if (l_interfacePortData->m_doInitialOfferForward)
        {
            l_interfacePortsForInitialForwarding.push_back(l_interfacePortData);
            l_interfacePortData->m_doInitialOfferForward = false;
        }

        // check if we have to destroy this interface port
        if (l_interfacePortData->m_toBeDestroyed)
        {
            m_portPool->removeInterfacePort(l_interfacePortData);
            DEBUG_PRINTF("Destroyed InterfacePortData\n");
        }
    }

    if (l_interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active sender ports to all new interfaces
        capro::CaproMessage l_caproMessage;
        l_caproMessage.m_type = capro::CaproMessageType::OFFER;
        for (auto l_senderPortData : m_portPool->senderPortDataList())
        {
            SenderPortType l_senderPort(l_senderPortData);
            if (l_senderPort.isPortActive())
            {
                if (l_senderPort.doesDeliverOnSubscribe())
                {
                    l_caproMessage.m_subType = capro::CaproMessageSubType::FIELD;
                }
                else
                {
                    l_caproMessage.m_subType = capro::CaproMessageSubType::EVENT;
                }
                l_caproMessage.m_serviceDescription = l_senderPort.getCaProServiceDescription();
                for (auto& interfacePortData : l_interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    // do not offer on same interface
                    if (l_senderPort.getCaProServiceDescription().getSourceInterface()
                        != interfacePort.getCaProServiceDescription().getSourceInterface())
                    {
                        interfacePort.dispatchCaProMessage(l_caproMessage);
                    }
                }
            }
        }
        // also forward services from service registry
        auto serviceMap = m_serviceRegistry.getServiceMap();

        l_caproMessage.m_subType = capro::CaproMessageSubType::SERVICE;

        for (auto const& x : serviceMap)
        {
            for (auto& instance : x.second.instanceSet)
            {
                l_caproMessage.m_serviceDescription =
                    capro::ServiceDescription(x.first, instance, capro::AnyEventString);

                for (auto& interfacePortData : l_interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    interfacePort.dispatchCaProMessage(l_caproMessage);
                }
            }
        }
    }
}

void PortManager::handleApplications()
{
    capro::CaproMessage l_caproMessage;

    for (auto l_applicationPortData : m_portPool->appliactionPortDataList())
    {
        iox::popo::ApplicationPort l_applicationPort(l_applicationPortData);
        while (l_applicationPort.getCaProMessage(l_caproMessage))
        {
            switch (l_caproMessage.m_type)
            {
            case capro::CaproMessageType::OFFER: {
                auto l_serviceDescription = l_caproMessage.m_serviceDescription;
                addEntryToServiceRegistry(l_serviceDescription.getServiceIDString(),
                                          l_serviceDescription.getInstanceIDString());
                break;
            }
            case capro::CaproMessageType::STOP_OFFER: {
                auto l_serviceDescription = l_caproMessage.m_serviceDescription;
                removeEntryFromServiceRegistry(l_serviceDescription.getServiceIDString(),
                                               l_serviceDescription.getInstanceIDString());

                break;
            }
            default: {
                LOG_ERR("Roudi: Something went wrong in receiving CaproMessage in ApplicationPortList!");
            }
            }

            // forward to interfaces
            sendToAllMatchingInterfacePorts(l_caproMessage);
        }

        // check if we have to destroy this application port
        if (l_applicationPort.toBeDestroyed())
        {
            m_portPool->removeApplicationPort(l_applicationPortData);
            DEBUG_PRINTF("Destroyed ApplicationPortData\n");
        }
    }
}

void PortManager::handleRunnables()
{
    /// @todo we have to update the introspection but runnable information is in process introspection which is not
    // accessible here. So currently runnables will be removed not before a process is removed
    // m_processIntrospection->removeRunnable(cxx::CString100(f_process.c_str()), cxx::CString100(f_runnable.c_str()));

    for (auto runnableData : m_portPool->runnableDataList())
    {
        if (runnableData->m_toBeDestroyed)
        {
            m_portPool->removeRunnableData(runnableData);
            DEBUG_PRINTF("Destroyed RunnableData\n");
        }
    }
}

bool PortManager::sendToAllMatchingSenderPorts(const capro::CaproMessage& f_message,
                                                       ReceiverPortType& f_receiverSource)
{
    bool l_senderFound = false;
    for (auto l_senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType l_senderPort(l_senderPortData);
        if (f_receiverSource.getCaProServiceDescription() == l_senderPort.getCaProServiceDescription())
        {
            auto senderResponse = l_senderPort.dispatchCaProMessage(f_message);
            if (senderResponse.has_value())
            {
                // inform introspection
                m_portIntrospection.reportMessage(senderResponse.value());
            }
            l_senderFound = true;
        }
    }
    return l_senderFound;
}

void PortManager::sendToAllMatchingReceiverPorts(const capro::CaproMessage& f_message,
                                                         SenderPortType& f_senderSource)
{
    for (auto l_receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType l_receiverPort(l_receiverPortData);
        if (l_receiverPort.getCaProServiceDescription() == f_senderSource.getCaProServiceDescription())
        {
            auto receiverResponse = l_receiverPort.dispatchCaProMessage(f_message);

            // if the receivers react on the change, process it immediately on sender side
            if (receiverResponse.has_value())
            {
                // we only expect reaction on OFFER
                assert(capro::CaproMessageType::OFFER == f_message.m_type);

                // inform introspection
                m_portIntrospection.reportMessage(receiverResponse.value());

                auto senderResponse = f_senderSource.dispatchCaProMessage(receiverResponse.value());
                if (senderResponse.has_value())
                {
                    // inform introspection
                    m_portIntrospection.reportMessage(senderResponse.value());
                }
            }
        }
    }
}

void PortManager::sendToAllMatchingInterfacePorts(const capro::CaproMessage& f_message)
{
    for (auto l_interfacePortData : m_portPool->interfacePortDataList())
    {
        iox::popo::InterfacePort l_interfacePort(l_interfacePortData);
        // not to the interface the port is located
        if (f_message.m_serviceDescription.getSourceInterface()
            != l_interfacePort.getCaProServiceDescription().getSourceInterface())
        {
            if (!l_interfacePort.dispatchCaProMessage(f_message))
            {
                errorHandler(Error::kPORT_MANAGER__INTERFACE_FIFO_OVERFLOW);
            }
        }
    }
}

bool PortManager::areAllReceiverPortsSubscribed(std::string f_appName)
{
    int numberOfReceiverPorts{0};
    int numberOfConnectedReceiverPorts{0};
    for (auto l_receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType receiver(l_receiverPortData);
        if (receiver.getApplicationName() == iox::cxx::string<100>(iox::cxx::TruncateToCapacity, f_appName))
        {
            numberOfReceiverPorts++;
            numberOfConnectedReceiverPorts += receiver.isSubscribed() ? 1 : 0;
        }
    }

    return numberOfReceiverPorts == numberOfConnectedReceiverPorts;
}

void PortManager::deletePortsOfProcess(std::string f_processName)
{
    for (auto port : m_portPool->senderPortDataList())
    {
        SenderPortType l_sender(port);
        if (f_processName == l_sender.getApplicationName())
        {
            destroySenderPort(port);
        }
    }

    for (auto port : m_portPool->receiverPortDataList())
    {
        ReceiverPortType l_receiver(port);
        if (f_processName == l_receiver.getApplicationName())
        {
            destroyReceiverPort(port);
        }
    }

    for (auto port : m_portPool->interfacePortDataList())
    {
        popo::InterfacePort l_interface(port);
        if (f_processName == l_interface.getApplicationName())
        {
            m_portPool->removeInterfacePort(port);
            DEBUG_PRINTF("Deleted Interface of application %s\n", f_processName.c_str());
        }
    }

    for (auto port : m_portPool->appliactionPortDataList())
    {
        popo::ApplicationPort l_application(port);
        if (f_processName == l_application.getApplicationName())
        {
            m_portPool->removeApplicationPort(port);
            DEBUG_PRINTF("Deleted ApplicationPort of application %s\n", f_processName.c_str());
        }
    }

    for (auto runnableData : m_portPool->runnableDataList())
    {
        if (f_processName == runnableData->m_process)
        {
            m_portPool->removeRunnableData(runnableData);
            DEBUG_PRINTF("Deleted runnable of application %s\n", f_processName.c_str());
        }
    }
}

void PortManager::destroySenderPort(SenderPortType::MemberType_t* const senderPortData)
{
    SenderPortType senderPort(senderPortData);

    const auto& serviceDescription = senderPort.getCaProServiceDescription();
    removeEntryFromServiceRegistry(serviceDescription.getServiceIDString(), serviceDescription.getInstanceIDString());
    senderPort.cleanup();

    const capro::CaproMessage message(capro::CaproMessageType::STOP_OFFER, serviceDescription);
    m_portIntrospection.reportMessage(message);

    sendToAllMatchingReceiverPorts(message, senderPort);
    sendToAllMatchingInterfacePorts(message);

    m_portIntrospection.removeSender(senderPort.getApplicationName(), serviceDescription);

    // delete sender impl from list after StopOffer was processed
    m_portPool->removeSenderPort(senderPortData);
    DEBUG_PRINTF("Destroyed SenderPortImpl\n");
}

void PortManager::destroyReceiverPort(ReceiverPortType::MemberType_t* const receiverPortData)
{
    ReceiverPortType receiverPort(receiverPortData);

    receiverPort.cleanup();

    const auto& serviceDescription = receiverPort.getCaProServiceDescription();
    capro::CaproMessage message(capro::CaproMessageType::UNSUB, serviceDescription);
    message.m_requestPort = receiverPortData;
    m_portIntrospection.reportMessage(message);

    sendToAllMatchingSenderPorts(message, receiverPort);

    m_portIntrospection.removeReceiver(receiverPort.getApplicationName(), serviceDescription);

    // delete receiver impl from list after unsubscribe was processed
    m_portPool->removeReceiverPort(receiverPortData);
    DEBUG_PRINTF("Destroyed ReceiverPortImpl\n");
}

runtime::MqMessage PortManager::findService(const capro::ServiceDescription& f_service)
{
    // send find to all interfaces
    capro::CaproMessage l_caproMessage(capro::CaproMessageType::FIND, f_service);

    for (auto l_interfacePortData : m_portPool->interfacePortDataList())
    {
        iox::popo::InterfacePort l_interfacePort(l_interfacePortData);
        if (!l_interfacePort.dispatchCaProMessage(l_caproMessage))
        {
            errorHandler(Error::kPORT_MANAGER__INTERFACE_FIFO_OVERFLOW);
        }
    }

    // add all found instances to instanceString
    runtime::MqMessage l_instanceMessage;

    ServiceRegistry::InstanceSet_t l_instances;
    m_serviceRegistry.find(l_instances, f_service.getServiceIDString(), f_service.getInstanceIDString());
    for (auto& instance : l_instances)
    {
        l_instanceMessage << instance;
    }

    return l_instanceMessage;
}

const std::atomic<uint64_t>* PortManager::serviceRegistryChangeCounter()
{
    return m_portPool->serviceRegistryChangeCounter();
}

cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
PortManager::acquireSenderPortData(const capro::ServiceDescription& f_service,
                                           const std::string& f_processName,
                                           mepoo::MemoryManager* f_payloadMemoryManager,
                                           const std::string& f_runnable,
                                           const PortConfigInfo& portConfigInfo)
{
    // check if already in list, we currently do not support multi publisher for one CaPro ID
    for (auto l_senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType l_senderPort(l_senderPortData);
        if (f_service == l_senderPort.getCaProServiceDescription())
        {
            std::stringstream ss;
            ss << "Process '" << f_processName << "' tried to register an unique SenderPort which is already used by '"
               << l_senderPortData->m_processName << "' with service '"
               << f_service.operator cxx::Serialization().toString() << "'.";
            LOG_WARN(ss.str().c_str());
            if (l_senderPort.isUnique())
            {
                errorHandler(Error::kPOSH__SENDERPORT_NOT_UNIQUE, nullptr, ErrorLevel::MODERATE);
                return cxx::error<PortPoolError>(PortPoolError::UNIQUE_SENDER_PORT_ALREADY_EXISTS);
            }
            else
            {
                break;
            }
        }
    }
    // we can create a new port

    auto result = m_portPool->addSenderPort(f_service, f_payloadMemoryManager, f_processName, portConfigInfo.memoryInfo);
    if (!result.has_error())
    {
        m_portIntrospection.addSender(result.get_value(), f_processName, f_service, f_runnable);
    }

    return result;
}

/// @todo return a cxx::expected
ReceiverPortType::MemberType_t* PortManager::acquireReceiverPortData(const capro::ServiceDescription& f_service,
                                                                             const std::string& f_processName,
                                                                             const std::string& f_runnable,
                                                                             const PortConfigInfo& portConfigInfo)
{
    auto result = m_portPool->addReceiverPort(f_service, f_processName, portConfigInfo.memoryInfo);
    if (!result.has_error())
    {
        m_portIntrospection.addReceiver(result.get_value(), f_processName, f_service, f_runnable);
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

/// @todo return a cxx::expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces f_interface,
                                                                       const std::string& f_processName,
                                                                       const std::string& /*f_runnable*/)
{
    auto result = m_portPool->addInterfacePort(f_processName, f_interface);
    if (!result.has_error())
    {
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

/// @todo return a cxx::expected
popo::ApplicationPortData* PortManager::acquireApplicationPortData(const std::string& f_processName)
{
    auto result = m_portPool->addApplicationPort(f_processName);
    if (!result.has_error())
    {
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

void PortManager::addEntryToServiceRegistry(const capro::IdString& service,
                                                    const capro::IdString& instance) noexcept
{
    m_serviceRegistry.add(service, instance);
    m_portPool->serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
}

void PortManager::removeEntryFromServiceRegistry(const capro::IdString& service,
                                                         const capro::IdString& instance) noexcept
{
    m_serviceRegistry.remove(service, instance);
    m_portPool->serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
}

/// @todo return a cxx::expected
runtime::RunnableData* PortManager::acquireRunnableData(const cxx::CString100& f_process,
                                                                const cxx::CString100& f_runnable)
{
    auto result = m_portPool->addRunnableData(f_process, f_runnable, 0);
    if (!result.has_error())
    {
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

} // namespace roudi
} // namespace iox
