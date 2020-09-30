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
capro::Interfaces StringToCaProInterface(const capro::IdString& str) noexcept
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

PortManager::PortManager(RouDiMemoryInterface* roudiMemoryInterface) noexcept
{
    m_roudiMemoryInterface = roudiMemoryInterface;

    auto maybePortPool = m_roudiMemoryInterface->portPool();
    if (!maybePortPool.has_value())
    {
        LogFatal() << "Could not get PortPool!";
        errorHandler(Error::kPORT_MANAGER__PORT_POOL_UNAVAILABLE, nullptr, iox::ErrorLevel::FATAL);
    }
    m_portPool = maybePortPool.value();

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface->introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        LogFatal() << "Could not get MemoryManager for introspection!";
        errorHandler(Error::kPORT_MANAGER__INTROSPECTION_MEMORY_MANAGER_UNAVAILABLE, nullptr, iox::ErrorLevel::FATAL);
    }
    auto& introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

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

void PortManager::stopPortIntrospection() noexcept
{
    m_portIntrospection.stop();
}

void PortManager::doDiscovery() noexcept
{
    handleSenderPorts();

    handleReceiverPorts();

    handleApplications();

    handleInterfaces();

    handleRunnables();
}

/// @deprecated #25
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

/// @deprecated #25
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

void PortManager::handlePublisherPorts() noexcept
{
    // get the changes of publisher port offer state
    for (auto publisherPortData : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(publisherPortData);

        publisherPort.tryGetCaProMessage().and_then([&](capro::CaproMessage caproMessage) {
            m_portIntrospection.reportMessage(caproMessage);

            if ((capro::CaproMessageType::OFFER == caproMessage.m_type)
                || (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type))
            {
                addEntryToServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                          caproMessage.m_serviceDescription.getInstanceIDString());

                sendToAllMatchingSubscriberPorts(caproMessage, publisherPort);
            }
            else
            {
                // protocol error
                errorHandler(Error::kPORT_MANAGER__HANDLE_PUBLISHER_PORTS_INVALID_CAPRO_MESSAGE,
                             nullptr,
                             iox::ErrorLevel::MODERATE);
            }

            // forward to interfaces
            sendToAllMatchingInterfacePorts(caproMessage);
        });

        // check if we have to destroy this publisher port
        if (publisherPort.toBeDestroyed())
        {
            destroyPublisherPort(publisherPortData);
        }
    }
}

void PortManager::handleSubscriberPorts() noexcept
{
    // get requests for change of subscription state of subscribers
    for (auto subscriberPortData : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortType subscriberPort(subscriberPortData);

        subscriberPort.tryGetCaProMessage().and_then([&](capro::CaproMessage caproMessage) {
            m_portIntrospection.reportMessage(caproMessage);

            if ((capro::CaproMessageType::SUB == caproMessage.m_type)
                || (capro::CaproMessageType::UNSUB == caproMessage.m_type))
            {
                if (!sendToAllMatchingPublisherPorts(caproMessage, subscriberPort))
                {
                    LogDebug() << "capro::SUB/UNSUB, no matching sender!!";
                    capro::CaproMessage nackMessage(capro::CaproMessageType::NACK,
                                                    subscriberPort.getCaProServiceDescription());
                    auto returnMessage = subscriberPort.dispatchCaProMessageAndGetPossibleResponse(nackMessage);
                    // No response on NACK messages
                    cxx::Ensures(!returnMessage.has_value());
                }
            }
            else
            {
                // protocol error
                errorHandler(Error::kPORT_MANAGER__HANDLE_SUBSCRIBER_PORTS_INVALID_CAPRO_MESSAGE,
                             nullptr,
                             iox::ErrorLevel::MODERATE);
            }
        });

        // check if we have to destroy this subscriber port
        if (subscriberPort.toBeDestroyed())
        {
            destroySubscriberPort(subscriberPortData);
        }
    }
}

void PortManager::handleInterfaces() noexcept
{
    // check if there are new interfaces that must get an initial offer information
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> interfacePortsForInitialForwarding;


    for (auto interfacePortData : m_portPool->getInterfacePortDataList())
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

void PortManager::handleApplications() noexcept
{
    capro::CaproMessage caproMessage;

    for (auto applicationPortData : m_portPool->getApplicationPortDataList())
    {
        iox::popo::ApplicationPort applicationPort(applicationPortData);

        while (auto maybeCaproMessage = applicationPort.tryGetCaProMessage())
        {
            auto& caproMessage = maybeCaproMessage.value();
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

void PortManager::handleRunnables() noexcept
{
    /// @todo we have to update the introspection but runnable information is in process introspection which is not
    // accessible here. So currently runnables will be removed not before a process is removed
    // m_processIntrospection->removeRunnable(ProcessName_t(process.c_str()),
    // RunnableName_t(runnable.c_str()));

    for (auto runnableData : m_portPool->getRunnableDataList())
    {
        if (runnableData->m_toBeDestroyed)
        {
            m_portPool->removeRunnableData(runnableData);
            LogDebug() << "Destroyed RunnableData";
        }
    }
}

/// @deprecated #25
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

/// @deprecated #25
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

bool PortManager::sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                                  SubscriberPortType& subscriberSource) noexcept
{
    bool publisherFound = false;
    for (auto publisherPortData : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(publisherPortData);
        if (subscriberSource.getCaProServiceDescription() == publisherPort.getCaProServiceDescription())
        {
            auto publisherResponse = publisherPort.dispatchCaProMessageAndGetPossibleResponse(message);
            if (publisherResponse.has_value())
            {
                // send response to subscriber port
                auto returnMessage =
                    subscriberSource.dispatchCaProMessageAndGetPossibleResponse(publisherResponse.value());

                // ACK or NACK are sent back to the subscriber port, no further response from this one expected
                cxx::Ensures(!returnMessage.has_value());

                // inform introspection
                m_portIntrospection.reportMessage(publisherResponse.value());
            }
            publisherFound = true;
        }
    }
    return publisherFound;
}

void PortManager::sendToAllMatchingSubscriberPorts(const capro::CaproMessage& message,
                                                   PublisherPortRouDiType& publisherSource) noexcept
{
    for (auto subscriberPortData : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortType subscriberPort(subscriberPortData);
        if (subscriberPort.getCaProServiceDescription() == publisherSource.getCaProServiceDescription())
        {
            auto subscriberResponse = subscriberPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the subscribers react on the change, process it immediately on publisher side
            if (subscriberResponse.has_value())
            {
                // we only expect reaction on OFFER
                cxx::Expects(capro::CaproMessageType::OFFER == message.m_type);

                // inform introspection
                m_portIntrospection.reportMessage(subscriberResponse.value());

                auto publisherResponse =
                    publisherSource.dispatchCaProMessageAndGetPossibleResponse(subscriberResponse.value());
                if (publisherResponse.has_value())
                {
                    // sende responsee to subscriber port
                    auto returnMessage =
                        subscriberPort.dispatchCaProMessageAndGetPossibleResponse(publisherResponse.value());

                    // ACK or NACK are sent back to the subscriber port, no further response from this one expected
                    cxx::Ensures(!returnMessage.has_value());

                    // inform introspection
                    m_portIntrospection.reportMessage(publisherResponse.value());
                }
            }
        }
    }
}

void PortManager::sendToAllMatchingInterfacePorts(const capro::CaproMessage& message) noexcept
{
    for (auto interfacePortData : m_portPool->getInterfacePortDataList())
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

void PortManager::deletePortsOfProcess(const ProcessName_t& processName) noexcept
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

    for (auto port : m_portPool->getInterfacePortDataList())
    {
        popo::InterfacePort interface(port);
        if (processName == interface.getProcessName())
        {
            m_portPool->removeInterfacePort(port);
            LogDebug() << "Deleted Interface of application " << processName;
        }
    }

    for (auto port : m_portPool->getApplicationPortDataList())
    {
        popo::ApplicationPort application(port);
        if (processName == application.getProcessName())
        {
            m_portPool->removeApplicationPort(port);
            LogDebug() << "Deleted ApplicationPort of application " << processName;
        }
    }

    for (auto runnableData : m_portPool->getRunnableDataList())
    {
        if (processName == runnableData->m_process)
        {
            m_portPool->removeRunnableData(runnableData);
            LogDebug() << "Deleted runnable of application " << processName;
        }
    }
}

/// @deprecated #25
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

/// @deprecated #25
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

void PortManager::destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept
{
    // create temporary publisher ports to orderly shut this publisher down
    PublisherPortRouDiType publisherPortRoudi{publisherPortData};
    PublisherPortUserType publisherPortUser{publisherPortData};

    publisherPortRoudi.releaseAllChunks();
    publisherPortUser.stopOffer();

    // process STOP_OFFER for this publisher in RouDi and distribute it
    publisherPortRoudi.tryGetCaProMessage().and_then([&](capro::CaproMessage caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER);

        m_portIntrospection.reportMessage(caproMessage);
        removeEntryFromServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                       caproMessage.m_serviceDescription.getInstanceIDString());
        sendToAllMatchingSubscriberPorts(caproMessage, publisherPortRoudi);
        sendToAllMatchingInterfacePorts(caproMessage);
    });

    /// @todo #25 Fix introspection
    // m_portIntrospection.removePublisher(publisherPort.getProcessName(), serviceDescription);

    // delete publisher port from list after STOP_OFFER was processed
    m_portPool->removePublisherPort(publisherPortData);

    LogDebug() << "Destroyed publisher port";
}

void PortManager::destroySubscriberPort(SubscriberPortType::MemberType_t* const subscriberPortData) noexcept
{
    // create temporary subscriber ports to orderly shut this subscriber down
    SubscriberPortType subscriberPortRoudi(subscriberPortData);
    SubscriberPortUserType subscriberPortUser(subscriberPortData);

    subscriberPortRoudi.releaseAllChunks();
    subscriberPortUser.unsubscribe();

    // process UNSUB for this subscriber in RouDi and distribute it
    subscriberPortRoudi.tryGetCaProMessage().and_then([&](capro::CaproMessage caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::UNSUB);

        m_portIntrospection.reportMessage(caproMessage);
        sendToAllMatchingPublisherPorts(caproMessage, subscriberPortRoudi);
    });

    /// @todo #25 Fix introspection
    // m_portIntrospection.removeSubscriber(subscriberPort.getProcessName(), serviceDescription);

    // delete subscriber port from list after UNSUB was processed
    m_portPool->removeSubscriberPort(subscriberPortData);

    LogDebug() << "Destroyed subscriber port";
}

runtime::MqMessage PortManager::findService(const capro::ServiceDescription& service) noexcept
{
    // send find to all interfaces
    capro::CaproMessage caproMessage(capro::CaproMessageType::FIND, service);

    for (auto interfacePortData : m_portPool->getInterfacePortDataList())
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

const std::atomic<uint64_t>* PortManager::serviceRegistryChangeCounter() noexcept
{
    return m_portPool->serviceRegistryChangeCounter();
}

/// @deprecated #25
cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
PortManager::acquireSenderPortData(const capro::ServiceDescription& service,
                                   const ProcessName_t& processName,
                                   mepoo::MemoryManager* payloadMemoryManager,
                                   const RunnableName_t& runnable,
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
                errorHandler(Error::kPOSH__PORT_MANAGER_SENDERPORT_NOT_UNIQUE, nullptr, ErrorLevel::MODERATE);
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

/// @deprecated #25
ReceiverPortType::MemberType_t* PortManager::acquireReceiverPortData(const capro::ServiceDescription& service,
                                                                     const ProcessName_t& processName,
                                                                     const RunnableName_t& runnable,
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

cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortManager::acquirePublisherPortData(const capro::ServiceDescription& service,
                                      const uint64_t& historyCapacity,
                                      const ProcessName_t& processName,
                                      mepoo::MemoryManager* payloadMemoryManager,
                                      const RunnableName_t& runnable [[gnu::unused]], // @todo #25 Fix introspection
                                      const PortConfigInfo& portConfigInfo) noexcept
{
    if (doesViolateCommunicationPolicy<iox::build::CommunicationPolicy>(service).and_then(
            [&](const ProcessName_t& usedByProcess) {
                LogWarn()
                    << "Process '" << processName
                    << "' violates the communication policy by requesting a PublisherPort which is already used by '"
                    << usedByProcess << "' with service '" << service.operator cxx::Serialization().toString() << "'.";
            }))
    {
        errorHandler(Error::kPOSH__PORT_MANAGER_PUBLISHERPORT_NOT_UNIQUE, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS);
    }

    // we can create a new port
    auto maybePublisherPortData = m_portPool->addPublisherPort(
        service, historyCapacity, payloadMemoryManager, processName, portConfigInfo.memoryInfo);
    if (!maybePublisherPortData.has_error())
    {
        /// @todo #25 Fix introspection
        // m_portIntrospection.addSender(result.get_value(), processName, service, runnable);
    }

    return maybePublisherPortData;
}

cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortManager::acquireSubscriberPortData(const capro::ServiceDescription& service,
                                       const uint64_t& historyRequest,
                                       const ProcessName_t& processName,
                                       const RunnableName_t& runnable [[gnu::unused]], // @todo #25 Fix introspection
                                       const PortConfigInfo& portConfigInfo) noexcept
{
    auto maybeSubscriberPortData =
        m_portPool->addSubscriberPort(service, historyRequest, processName, portConfigInfo.memoryInfo);
    if (!maybeSubscriberPortData.has_error())
    {
        /// @todo #25 Fix introspection
        // m_portIntrospection.addReceiver(result.get_value(), processName, service, runnable);
    }

    return maybeSubscriberPortData;
}


/// @todo return a cxx::expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces interface,
                                                               const ProcessName_t& processName,
                                                               const RunnableName_t& /*runnable*/) noexcept
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
popo::ApplicationPortData* PortManager::acquireApplicationPortData(const ProcessName_t& processName) noexcept
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
runtime::RunnableData* PortManager::acquireRunnableData(const ProcessName_t& process,
                                                        const RunnableName_t& runnable) noexcept
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

cxx::expected<popo::ConditionVariableData*, PortPoolError> PortManager::acquireConditionVariableData() noexcept
{
    return m_portPool->addConditionVariableData();
}

} // namespace roudi
} // namespace iox
