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

#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
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
Interfaces StringToEInterfaces(std::string f_str)
{
    int32_t i;
    cxx::convert::fromString(f_str.c_str(), i);
    if (i >= static_cast<int32_t>(Interfaces::INTERFACE_END))
    {
        WARN_PRINTF("invalid enum (out of range: %d) \n", i);
        return Interfaces::INTERNAL;
    }
    return static_cast<Interfaces>(i);
}

SharedMemoryManager::SharedMemoryManager(const RouDiConfig_t& f_config)
    : m_ShmInterface(f_config)
{
    // Remark: m_portIntrospection is not fully functional in base class RouDiBase (has no active senderport)
    // are there used instances of RouDiBase?

    auto portGeneric = acquireSenderPortData(IntrospectionPortService,
                                             Interfaces::INTERNAL,
                                             PORT_INTROSPECTION_MQ_APP_NAME,
                                             &m_ShmInterface.getShmInterface()->m_roudiMemoryManager);

    auto portThroughput = acquireSenderPortData(IntrospectionPortThroughputService,
                                                Interfaces::INTERNAL,
                                                PORT_INTROSPECTION_MQ_APP_NAME,
                                                &m_ShmInterface.getShmInterface()->m_roudiMemoryManager);

    auto receiverPortsData = acquireSenderPortData(IntrospectionReceiverPortChangingDataService,
                                                   Interfaces::INTERNAL,
                                                   PORT_INTROSPECTION_MQ_APP_NAME,
                                                   &m_ShmInterface.getShmInterface()->m_roudiMemoryManager);

    m_portIntrospection.registerSenderPort(portGeneric, portThroughput, receiverPortsData);
    m_portIntrospection.run();
}

void SharedMemoryManager::stopPortIntrospection()
{
    m_portIntrospection.stop();
}

void SharedMemoryManager::doDiscovery()
{
    handleSenderPorts();

    handleReceiverPorts();

    handleApplications();

    handleInterfaces();
}

void SharedMemoryManager::handleSenderPorts()
{
    // get the changes of sender port offer state
    for (auto l_senderPortData : m_ShmInterface.getShmInterface()->m_senderPortMembers.content())
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
            sendToAllMatchingInterfacePorts(caproMessage, l_senderPort.getInterface());
        }
    }
}

void SharedMemoryManager::handleReceiverPorts()
{
    // get requests for change of subscription state of receivers
    for (auto l_receiverPortData : m_ShmInterface.getShmInterface()->m_receiverPortMembers.content())
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
    }
}

void SharedMemoryManager::handleInterfaces()
{
    // check if there are new interfaces that must get an initial offer information
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> l_interfacePortsForInitialForwarding;


    for (auto l_interfacePortData : m_ShmInterface.getShmInterface()->m_interfacePortMembers.content())
    {
        if (l_interfacePortData->m_doInitialOfferForward)
        {
            l_interfacePortsForInitialForwarding.push_back(l_interfacePortData);
            l_interfacePortData->m_doInitialOfferForward = false;
        }
    }

    if (l_interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active sender ports to all new interfaces
        capro::CaproMessage l_caproMessage;
        l_caproMessage.m_type = capro::CaproMessageType::OFFER;
        for (auto l_senderPortData : m_ShmInterface.getShmInterface()->m_senderPortMembers.content())
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
                    if (l_senderPort.getInterface() != interfacePort.getInterface())
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

void SharedMemoryManager::handleApplications()
{
    capro::CaproMessage l_caproMessage;

    for (auto l_applicationPortData : m_ShmInterface.getShmInterface()->m_applicationPortMembers.content())
    {
        iox::popo::ApplicationPort l_applicationPort(l_applicationPortData);
        while (l_applicationPort.getCaProMessage(l_caproMessage))
        {
            switch (l_caproMessage.m_type)
            {
            case capro::CaproMessageType::OFFER:
            {
                auto l_serviceDescription = l_caproMessage.m_serviceDescription;
                addEntryToServiceRegistry(l_serviceDescription.getServiceIDString(),
                                          l_serviceDescription.getInstanceIDString());
                break;
            }
            case capro::CaproMessageType::STOP_OFFER:
            {
                auto l_serviceDescription = l_caproMessage.m_serviceDescription;
                removeEntryFromServiceRegistry(l_serviceDescription.getServiceIDString(),
                                               l_serviceDescription.getInstanceIDString());

                break;
            }
            default:
            {
                LOG_ERR("Roudi: Something went wrong in receiving CaproMessage in ApplicationPortList!");
            }
            }

            // forward to interfaces
            sendToAllMatchingInterfacePorts(l_caproMessage, l_applicationPort.getInterface());
        }
    }
}

bool SharedMemoryManager::sendToAllMatchingSenderPorts(const capro::CaproMessage& f_message,
                                                       ReceiverPortType& f_receiverSource)
{
    bool l_senderFound = false;
    for (auto l_senderPortData : m_ShmInterface.getShmInterface()->m_senderPortMembers.content())
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

void SharedMemoryManager::sendToAllMatchingReceiverPorts(const capro::CaproMessage& f_message,
                                                         SenderPortType& f_senderSource)
{
    for (auto l_receiverPortData : m_ShmInterface.getShmInterface()->m_receiverPortMembers.content())
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

void SharedMemoryManager::sendToAllMatchingInterfacePorts(const capro::CaproMessage& f_message,
                                                          const iox::Interfaces& f_interfaceSource)
{
    for (auto l_interfacePortData : m_ShmInterface.getShmInterface()->m_interfacePortMembers.content())
    {
        iox::popo::InterfacePort l_interfacePort(l_interfacePortData);
        // not to the interface the port is located
        if (f_interfaceSource != l_interfacePort.getInterface())
        {
            if (!l_interfacePort.dispatchCaProMessage(f_message))
            {
                errorHandler(Error::kROUDI_SHM__INTERFACE_FIFO_OVERFLOW);
            }
        }
    }
}

bool SharedMemoryManager::areAllReceiverPortsSubscribed(std::string f_appName)
{
    int numberOfReceiverPorts{0};
    int numberOfConnectedReceiverPorts{0};
    for (auto l_receiverPortData : m_ShmInterface.getShmInterface()->m_receiverPortMembers.content())
    {
        ReceiverPortType receiver(l_receiverPortData);
        if (receiver.getApplicationName() == f_appName)
        {
            numberOfReceiverPorts++;
            numberOfConnectedReceiverPorts += receiver.isSubscribed() ? 1 : 0;
        }
    }

    return numberOfReceiverPorts == numberOfConnectedReceiverPorts;
}

void SharedMemoryManager::deletePortsOfProcess(std::string f_processName)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();

    for (auto port : l_shm->m_senderPortMembers.content())
    {
        SenderPortType l_sender(port);
        if (f_processName == l_sender.getApplicationName())
        {
            const auto& serviceDescription = l_sender.getCaProServiceDescription();
            removeEntryFromServiceRegistry(serviceDescription.getServiceIDString(),
                                           serviceDescription.getInstanceIDString());
            l_sender.cleanup();

            capro::CaproMessage message(capro::CaproMessageType::STOP_OFFER, serviceDescription);
            m_portIntrospection.reportMessage(message);

            sendToAllMatchingReceiverPorts(message, l_sender);

            m_portIntrospection.removeSender(f_processName, serviceDescription);

            // delete sender impl from list after StopOffer was processed
            l_shm->m_senderPortMembers.erase(port);
            DEBUG_PRINTF("Deleted SenderPortImpl of application %s\n", f_processName.c_str());
        }
    }

    for (auto port : l_shm->m_receiverPortMembers.content())
    {
        ReceiverPortType l_receiver(port);
        if (f_processName == l_receiver.getApplicationName())
        {
            // do the complete cleanup for the receiver port for being able to erase it
            l_receiver.cleanup();

            const auto& serviceDescription = l_receiver.getCaProServiceDescription();
            capro::CaproMessage message(capro::CaproMessageType::UNSUB, serviceDescription);
            message.m_requestPort = port;
            m_portIntrospection.reportMessage(message);

            sendToAllMatchingSenderPorts(message, l_receiver);

            m_portIntrospection.removeReceiver(f_processName, serviceDescription);

            // delete receiver impl from list after unsubscribe was processed
            l_shm->m_receiverPortMembers.erase(port);
            DEBUG_PRINTF("Deleted ReceiverPortImpl of application %s\n", f_processName.c_str());
        }
    }

    for (auto port : l_shm->m_interfacePortMembers.content())
    {
        popo::InterfacePort l_interface(port);
        if (f_processName == l_interface.getApplicationName())
        {
            l_shm->m_interfacePortMembers.erase(port);
            DEBUG_PRINTF("Deleted Interface of application %s\n", f_processName.c_str());
        }
    }

    for (auto port : l_shm->m_applicationPortMembers.content())
    {
        popo::ApplicationPort l_application(port);
        if (f_processName == l_application.getApplicationName())
        {
            l_shm->m_applicationPortMembers.erase(port);
            DEBUG_PRINTF("Deleted ApplicationPort of application %s\n", f_processName.c_str());
        }
    }

    for (auto runnableData : l_shm->m_runnableMembers.content())
    {
        if (f_processName == runnableData->m_process)
        {
            l_shm->m_runnableMembers.erase(runnableData);
            DEBUG_PRINTF("Deleted runnable of application %s\n", f_processName.c_str());
        }
    }
}

void SharedMemoryManager::deleteRunnableAndItsPorts(std::string f_runnableName)
{
    ERR_PRINTF("Not yet supported");
    assert(false);
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();

    // @todo do we delete all the ports that are related to the runnable for ensuring that there are no dangling
    // references
    // search the ports delete ports then delete the runnable

    for (auto runnableData : l_shm->m_runnableMembers.content())
    {
        if (f_runnableName == runnableData->m_runnable)
        {
            l_shm->m_runnableMembers.erase(runnableData);
            DEBUG_PRINTF("Deleted runnable %s\n", f_runnableName.c_str());
        }
    }
}

std::string SharedMemoryManager::GetShmAddrString()
{
    return m_ShmInterface.getBaseAddrString();
}

uint64_t SharedMemoryManager::getShmSizeInBytes() const
{
    return m_ShmInterface.getShmSizeInBytes();
}

runtime::MqMessage SharedMemoryManager::findService(const capro::ServiceDescription& f_service)
{
    // send find to all interfaces
    capro::CaproMessage l_caproMessage(capro::CaproMessageType::FIND, f_service);

    for (auto l_interfacePortData : m_ShmInterface.getShmInterface()->m_interfacePortMembers.content())
    {
        iox::popo::InterfacePort l_interfacePort(l_interfacePortData);
        if (!l_interfacePort.dispatchCaProMessage(l_caproMessage))
        {
            errorHandler(Error::kROUDI_SHM__INTERFACE_FIFO_OVERFLOW);
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

const runtime::SharedMemoryCreator<MiddlewareShm>& SharedMemoryManager::getShmInterface()
{
    return m_ShmInterface;
}

SenderPortType::MemberType_t* SharedMemoryManager::acquireSenderPortData(const capro::ServiceDescription& f_service,
                                                                         Interfaces f_interface,
                                                                         const std::string& f_processName,
                                                                         mepoo::MemoryManager* f_payloadMemoryManager,
                                                                         const std::string& f_runnable)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();

    // check if already in list, we currently do not support multi publisher for one CaPro ID
    for (auto l_senderPortData : l_shm->m_senderPortMembers.content())
    {
        SenderPortType l_senderPort(l_senderPortData);
        if (f_service == l_senderPort.getCaProServiceDescription())
        {
            /// @todo report an error but not one that leads to termination of RouDi
            WARN_PRINTF("Multiple sender ports with same CaPro ID currently not supported\n %s %s %s",
                        f_service.getServiceIDString().to_cstring(),
                        f_service.getInstanceIDString().to_cstring(),
                        f_service.getEventIDString().to_cstring());
            /// @todo we have to tolerate it currently for ROS2 support. Needs multiple sender feature
            // return nullptr;
        }
    }

    if (l_shm->m_senderPortMembers.hasFreeSpace())
    {
        //  we don't use the runnable reference in the ports yet. So set it to nullptr
        auto senderPortData =
            l_shm->m_senderPortMembers.insert(f_service, f_payloadMemoryManager, f_processName, f_interface, nullptr);
        m_portIntrospection.addSender(senderPortData, f_processName, f_service, f_runnable);
        return senderPortData;
    }
    else
    {
        errorHandler(Error::kROUDI_SHM__MIDDLEWARESENDERLIST_OVERFLOW);
        return nullptr;
    }
}

ReceiverPortType::MemberType_t* SharedMemoryManager::acquireReceiverPortData(const capro::ServiceDescription& f_service,
                                                                             Interfaces f_interface,
                                                                             const std::string& f_processName,
                                                                             const std::string& f_runnable)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();

    capro::ServiceDescription const* service = &f_service;
    auto DebugMsg = [&](const char* msg) {
        DEBUG_PRINTF("%s for %s - service: %s - %s - %s (%d, %d, %d)\n",
                     msg,
                     f_processName.c_str(),
                     service->getServiceIDString().to_cstring(),
                     service->getInstanceIDString().to_cstring(),
                     service->getEventIDString().to_cstring(),
                     service->getServiceID(),
                     service->getInstanceID(),
                     service->getEventID());
    };

    if (l_shm->m_receiverPortMembers.hasFreeSpace())
    {
        //  we don't use the runnable reference in the ports yet. So set it to nullptr
        auto port = l_shm->m_receiverPortMembers.insert(*service, f_processName, f_interface, nullptr);

        m_portIntrospection.addReceiver(port, f_processName, *service, f_runnable);

        return port;
    }
    else
    {
        errorHandler(Error::kROUDI_SHM__MIDDLEWARERECEIVERLIST_OVERFLOW);
        return nullptr;
    }
}

popo::InterfacePortData* SharedMemoryManager::acquireInterfacePortData(Interfaces f_interface,
                                                                       const std::string& f_processName,
                                                                       const std::string& /*f_runnable*/)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();
    if (l_shm->m_interfacePortMembers.hasFreeSpace())
    {
        //  we don't use the runnable reference in the ports yet. So set it to nullptr
        return l_shm->m_interfacePortMembers.insert(f_processName, f_interface, nullptr);
    }
    else
    {
        errorHandler(Error::kROUDI_SHM__MIDDLEWAREINTERFACELIST_OVERFLOW);
        return nullptr;
    }
}

popo::ApplicationPortData* SharedMemoryManager::acquireApplicationPortData(Interfaces f_interface,
                                                                           const std::string& f_processName)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();
    if (l_shm->m_applicationPortMembers.hasFreeSpace())
    {
        return l_shm->m_applicationPortMembers.insert(f_processName, f_interface);
    }
    else
    {
        errorHandler(Error::kROUDI_SHM__MIDDLEWAREAPPLICATIONLIST_OVERFLOW);
        return nullptr;
    }
}

void SharedMemoryManager::addEntryToServiceRegistry(const capro::ServiceDescription::IdString& service,
                                                    const capro::ServiceDescription::IdString& instance) noexcept
{
    m_serviceRegistry.add(service, instance);
    m_ShmInterface.getShmInterface()->m_serviceRegistryChangeCounter++;
}

void SharedMemoryManager::removeEntryFromServiceRegistry(const capro::ServiceDescription::IdString& service,
                                                         const capro::ServiceDescription::IdString& instance) noexcept
{
    m_serviceRegistry.remove(service, instance);
    m_ShmInterface.getShmInterface()->m_serviceRegistryChangeCounter++;
}

runtime::RunnableData* SharedMemoryManager::acquireRunnableData(const cxx::CString100& f_process,
                                                                const cxx::CString100& f_runnable)
{
    MiddlewareShm* const l_shm = m_ShmInterface.getShmInterface();
    if (l_shm->m_runnableMembers.hasFreeSpace())
    {
        /// for now there is no additional data like device identifier
        return l_shm->m_runnableMembers.insert(f_process, f_runnable, 0);
    }
    else
    {
        errorHandler(Error::kROUDI_SHM__MIDDLEWARERUNNABLELIST_OVERFLOW);
        return nullptr;
    }
}


} // namespace roudi
} // namespace iox
