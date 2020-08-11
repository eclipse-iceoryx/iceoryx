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
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/runtime/runnable.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToCaProInterface(const std::string& str)
{
    int32_t i;
    cxx::convert::fromString(str.c_str(), i);
    if (i >= static_cast<int32_t>(capro::Interfaces::INTERFACE_END))
    {
        LogWarn() << "invalid enum (out of range: " << i << ")";
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
    for (auto senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType senderPort(senderPortData);
        auto returnedCaproMessage = senderPort.getCaProMessage();
        if (returnedCaproMessage.has_value())
        {
            auto& caproMessage = returnedCaproMessage.value();

            m_portIntrospection.reportMessage(caproMessage);

            if (capro::CaproMessageType::OFFER == caproMessage.m_type)
            {
                addEntryToServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                          caproMessage.m_serviceDescription.getInstanceIDString());

                sendToAllMatchingReceiverPorts(caproMessage, senderPort);
            }
            else if (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type)
            {
                removeEntryFromServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                               caproMessage.m_serviceDescription.getInstanceIDString());

                sendToAllMatchingReceiverPorts(caproMessage, senderPort);
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
        if (senderPort.toBeDestroyed())
        {
            destroySenderPort(senderPortData);
        }
    }
}

void PortManager::handleReceiverPorts()
{
    // get requests for change of subscription state of receivers
    for (auto receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType receiverPort(receiverPortData);
        auto returnedCaproMessage = receiverPort.getCaProMessage();
        if (returnedCaproMessage.has_value())
        {
            auto& caproMessage = returnedCaproMessage.value();

            m_portIntrospection.reportMessage(caproMessage);

            if (!sendToAllMatchingSenderPorts(caproMessage, receiverPort))
            {
                LogDebug() << "capro::SUB/UNSUB, no matching sender!!";
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK,
                                                receiverPort.getCaProServiceDescription());
                receiverPort.dispatchCaProMessage(nackMessage);
            }
        }
        // check if we have to destroy this sender port
        if (receiverPort.toBeDestroyed())
        {
            destroyReceiverPort(receiverPortData);
        }
    }
}

void PortManager::handleInterfaces()
{
    // check if there are new interfaces that must get an initial offer information
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> interfacePortsForInitialForwarding;


    for (auto interfacePortData : m_portPool->interfacePortDataList())
    {
        if (interfacePortData->m_doInitialOfferForward)
        {
            interfacePortsForInitialForwarding.push_back(interfacePortData);
            interfacePortData->m_doInitialOfferForward = false;
        }

        // check if we have to destroy this interface port
        if (interfacePortData->m_toBeDestroyed)
        {
            m_portPool->removeInterfacePort(interfacePortData);
            LogDebug() << "Destroyed InterfacePortData";
        }
    }

    if (interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active sender ports to all new interfaces
        capro::CaproMessage caproMessage;
        caproMessage.m_type = capro::CaproMessageType::OFFER;
        for (auto senderPortData : m_portPool->senderPortDataList())
        {
            SenderPortType senderPort(senderPortData);
            if (senderPort.isPortActive())
            {
                if (senderPort.doesDeliverOnSubscribe())
                {
                    caproMessage.m_subType = capro::CaproMessageSubType::FIELD;
                }
                else
                {
                    caproMessage.m_subType = capro::CaproMessageSubType::EVENT;
                }
                caproMessage.m_serviceDescription = senderPort.getCaProServiceDescription();
                for (auto& interfacePortData : interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    // do not offer on same interface
                    if (senderPort.getCaProServiceDescription().getSourceInterface()
                        != interfacePort.getCaProServiceDescription().getSourceInterface())
                    {
                        interfacePort.dispatchCaProMessage(caproMessage);
                    }
                }
            }
        }
        // also forward services from service registry
        auto serviceMap = m_serviceRegistry.getServiceMap();

        caproMessage.m_subType = capro::CaproMessageSubType::SERVICE;

        for (auto const& x : serviceMap)
        {
            for (auto& instance : x.second.instanceSet)
            {
                caproMessage.m_serviceDescription = capro::ServiceDescription(x.first, instance, capro::AnyEventString);

                for (auto& interfacePortData : interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    interfacePort.dispatchCaProMessage(caproMessage);
                }
            }
        }
    }
}

void PortManager::handleApplications()
{
    capro::CaproMessage caproMessage;

    for (auto applicationPortData : m_portPool->appliactionPortDataList())
    {
        iox::popo::ApplicationPort applicationPort(applicationPortData);

        for (auto maybeCaproMessage = applicationPort.getCaProMessage(); maybeCaproMessage.has_value();
             maybeCaproMessage = applicationPort.getCaProMessage())
        {
            auto caproMessage = maybeCaproMessage.value();
            switch (caproMessage.m_type)
            {
            case capro::CaproMessageType::OFFER:
            {
                auto serviceDescription = caproMessage.m_serviceDescription;
                addEntryToServiceRegistry(serviceDescription.getServiceIDString(),
                                          serviceDescription.getInstanceIDString());
                break;
            }
            case capro::CaproMessageType::STOP_OFFER:
            {
                auto serviceDescription = caproMessage.m_serviceDescription;
                removeEntryFromServiceRegistry(serviceDescription.getServiceIDString(),
                                               serviceDescription.getInstanceIDString());

                break;
            }
            default:
            {
                LogError() << "Roudi: Something went wrong in receiving CaproMessage in ApplicationPortList!";
            }
            }

            // forward to interfaces
            sendToAllMatchingInterfacePorts(caproMessage);
        }

        // check if we have to destroy this application port
        if (applicationPort.toBeDestroyed())
        {
            m_portPool->removeApplicationPort(applicationPortData);
            LogDebug() << "Destroyed ApplicationPortData";
        }
    }
}

void PortManager::handleRunnables()
{
    /// @todo we have to update the introspection but runnable information is in process introspection which is not
    // accessible here. So currently runnables will be removed not before a process is removed
    // m_processIntrospection->removeRunnable(cxx::CString100(process.c_str()),
    // cxx::CString100(runnable.c_str()));

    for (auto runnableData : m_portPool->runnableDataList())
    {
        if (runnableData->m_toBeDestroyed)
        {
            m_portPool->removeRunnableData(runnableData);
            LogDebug() << "Destroyed RunnableData";
        }
    }
}

bool PortManager::sendToAllMatchingSenderPorts(const capro::CaproMessage& message, ReceiverPortType& receiverSource)
{
    bool senderFound = false;
    for (auto senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType senderPort(senderPortData);
        if (receiverSource.getCaProServiceDescription() == senderPort.getCaProServiceDescription())
        {
            auto senderResponse = senderPort.dispatchCaProMessage(message);
            if (senderResponse.has_value())
            {
                // sende response to receiver port
                auto returnMessage = receiverSource.dispatchCaProMessage(senderResponse.value());

                // ACK or NACK are sent back to the receiver port, no further response from this one expected
                cxx::Ensures(!returnMessage.has_value());

                // inform introspection
                m_portIntrospection.reportMessage(senderResponse.value());
            }
            senderFound = true;
        }
    }
    return senderFound;
}

void PortManager::sendToAllMatchingReceiverPorts(const capro::CaproMessage& message, SenderPortType& senderSource)
{
    for (auto receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType receiverPort(receiverPortData);
        if (receiverPort.getCaProServiceDescription() == senderSource.getCaProServiceDescription())
        {
            auto receiverResponse = receiverPort.dispatchCaProMessage(message);

            // if the receivers react on the change, process it immediately on sender side
            if (receiverResponse.has_value())
            {
                // we only expect reaction on OFFER
                assert(capro::CaproMessageType::OFFER == message.m_type);

                // inform introspection
                m_portIntrospection.reportMessage(receiverResponse.value());

                auto senderResponse = senderSource.dispatchCaProMessage(receiverResponse.value());
                if (senderResponse.has_value())
                {
                    // sende responsee to receiver port
                    auto returnMessage = receiverPort.dispatchCaProMessage(senderResponse.value());

                    // ACK or NACK are sent back to the receiver port, no further response from this one expected
                    cxx::Ensures(!returnMessage.has_value());

                    // inform introspection
                    m_portIntrospection.reportMessage(senderResponse.value());
                }
            }
        }
    }
}

void PortManager::sendToAllMatchingInterfacePorts(const capro::CaproMessage& message)
{
    for (auto interfacePortData : m_portPool->interfacePortDataList())
    {
        iox::popo::InterfacePort interfacePort(interfacePortData);
        // not to the interface the port is located
        if (message.m_serviceDescription.getSourceInterface()
            != interfacePort.getCaProServiceDescription().getSourceInterface())
        {
            interfacePort.dispatchCaProMessage(message);
        }
    }
}

bool PortManager::areAllReceiverPortsSubscribed(std::string appName)
{
    int32_t numberOfReceiverPorts{0};
    int32_t numberOfConnectedReceiverPorts{0};
    for (auto receiverPortData : m_portPool->receiverPortDataList())
    {
        ReceiverPortType receiver(receiverPortData);
        if (receiver.getProcessName() == iox::cxx::string<100>(iox::cxx::TruncateToCapacity, appName))
        {
            numberOfReceiverPorts++;
            numberOfConnectedReceiverPorts += receiver.isSubscribed() ? 1 : 0;
        }
    }

    return numberOfReceiverPorts == numberOfConnectedReceiverPorts;
}

void PortManager::deletePortsOfProcess(std::string processName)
{
    for (auto port : m_portPool->senderPortDataList())
    {
        SenderPortType sender(port);
        if (processName == sender.getProcessName())
        {
            destroySenderPort(port);
        }
    }

    for (auto port : m_portPool->receiverPortDataList())
    {
        ReceiverPortType receiver(port);
        if (processName == receiver.getProcessName())
        {
            destroyReceiverPort(port);
        }
    }

    for (auto port : m_portPool->interfacePortDataList())
    {
        popo::InterfacePort interface(port);
        if (processName == interface.getProcessName())
        {
            m_portPool->removeInterfacePort(port);
            LogDebug() << "Deleted Interface of application " << processName;
        }
    }

    for (auto port : m_portPool->appliactionPortDataList())
    {
        popo::ApplicationPort application(port);
        if (processName == application.getProcessName())
        {
            m_portPool->removeApplicationPort(port);
            LogDebug() << "Deleted ApplicationPort of application " << processName;
        }
    }

    for (auto runnableData : m_portPool->runnableDataList())
    {
        if (processName == runnableData->m_process)
        {
            m_portPool->removeRunnableData(runnableData);
            LogDebug() << "Deleted runnable of application " << processName;
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

    m_portIntrospection.removeSender(senderPort.getProcessName(), serviceDescription);

    // delete sender impl from list after StopOffer was processed
    m_portPool->removeSenderPort(senderPortData);
    LogDebug() << "Destroyed SenderPortImpl";
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

    m_portIntrospection.removeReceiver(receiverPort.getProcessName(), serviceDescription);

    // delete receiver impl from list after unsubscribe was processed
    m_portPool->removeReceiverPort(receiverPortData);
    LogDebug() << "Destroyed ReceiverPortImpl";
}

runtime::MqMessage PortManager::findService(const capro::ServiceDescription& service)
{
    // send find to all interfaces
    capro::CaproMessage caproMessage(capro::CaproMessageType::FIND, service);

    for (auto interfacePortData : m_portPool->interfacePortDataList())
    {
        iox::popo::InterfacePort interfacePort(interfacePortData);
        interfacePort.dispatchCaProMessage(caproMessage);
    }

    // add all found instances to instanceString
    runtime::MqMessage instanceMessage;

    ServiceRegistry::InstanceSet_t instances;
    m_serviceRegistry.find(instances, service.getServiceIDString(), service.getInstanceIDString());
    for (auto& instance : instances)
    {
        instanceMessage << instance;
    }

    return instanceMessage;
}

const std::atomic<uint64_t>* PortManager::serviceRegistryChangeCounter()
{
    return m_portPool->serviceRegistryChangeCounter();
}

cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
PortManager::acquireSenderPortData(const capro::ServiceDescription& service,
                                   const std::string& processName,
                                   mepoo::MemoryManager* payloadMemoryManager,
                                   const std::string& runnable,
                                   const PortConfigInfo& portConfigInfo)
{
    // check if already in list, we currently do not support multi publisher for one CaPro ID
    for (auto senderPortData : m_portPool->senderPortDataList())
    {
        SenderPortType senderPort(senderPortData);
        if (service == senderPort.getCaProServiceDescription())
        {
            LogWarn() << "Process '" << processName
                      << "' tried to register an unique SenderPort which is already used by '"
                      << senderPortData->m_processName << "' with service '"
                      << service.operator cxx::Serialization().toString() << "'.";
            if (senderPort.isUnique())
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

    auto result = m_portPool->addSenderPort(service, payloadMemoryManager, processName, portConfigInfo.memoryInfo);
    if (!result.has_error())
    {
        m_portIntrospection.addSender(result.get_value(), processName, service, runnable);
    }

    return result;
}

/// @todo return a cxx::expected
ReceiverPortType::MemberType_t* PortManager::acquireReceiverPortData(const capro::ServiceDescription& service,
                                                                     const std::string& processName,
                                                                     const std::string& runnable,
                                                                     const PortConfigInfo& portConfigInfo)
{
    auto result = m_portPool->addReceiverPort(service, processName, portConfigInfo.memoryInfo);
    if (!result.has_error())
    {
        m_portIntrospection.addReceiver(result.get_value(), processName, service, runnable);
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

/// @todo return a cxx::expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces interface,
                                                               const std::string& processName,
                                                               const std::string& /*runnable*/)
{
    auto result = m_portPool->addInterfacePort(processName, interface);
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
popo::ApplicationPortData* PortManager::acquireApplicationPortData(const std::string& processName)
{
    auto result = m_portPool->addApplicationPort(processName);
    if (!result.has_error())
    {
        return result.get_value();
    }
    else
    {
        return nullptr;
    }
}

void PortManager::addEntryToServiceRegistry(const capro::IdString& service, const capro::IdString& instance) noexcept
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
runtime::RunnableData* PortManager::acquireRunnableData(const cxx::CString100& process, const cxx::CString100& runnable)
{
    auto result = m_portPool->addRunnableData(process, runnable, 0);
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
