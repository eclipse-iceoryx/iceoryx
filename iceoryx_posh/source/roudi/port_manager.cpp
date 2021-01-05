// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/runtime/node.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToCaProInterface(const capro::IdString_t& str) noexcept
{
    int32_t i{0};
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
    auto introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

    popo::PublisherOptions options;
    options.historyCapacity = 1;
    // Remark: m_portIntrospection is not fully functional in base class RouDiBase (has no active publisher port)
    // are there used instances of RouDiBase?
    auto maybePublisher = acquirePublisherPortData(IntrospectionPortService,
                                                   options,
                                                   MQ_ROUDI_NAME,
                                                   introspectionMemoryManager,
                                                   INTROSPECTION_SERVICE_ID,
                                                   PortConfigInfo());
    if (maybePublisher.has_error())
    {
        LogError() << "Could not create PublisherPort for IntrospectionPortService";
        errorHandler(
            Error::kPORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTROSPECTIONPORTSERVICE, nullptr, iox::ErrorLevel::SEVERE);
    }
    auto portGeneric = maybePublisher.value();

    maybePublisher = acquirePublisherPortData(IntrospectionPortThroughputService,
                                              options,
                                              MQ_ROUDI_NAME,
                                              introspectionMemoryManager,
                                              INTROSPECTION_SERVICE_ID,
                                              PortConfigInfo());
    if (maybePublisher.has_error())
    {
        LogError() << "Could not create PublisherPort for IntrospectionPortThroughputService";
        errorHandler(Error::kPORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTROSPECTIONPORTTHROUGHPUTSERVICE,
                     nullptr,
                     iox::ErrorLevel::SEVERE);
    }
    auto portThroughput = maybePublisher.value();

    maybePublisher = acquirePublisherPortData(IntrospectionSubscriberPortChangingDataService,
                                              options,
                                              MQ_ROUDI_NAME,
                                              introspectionMemoryManager,
                                              INTROSPECTION_SERVICE_ID,
                                              PortConfigInfo());
    if (maybePublisher.has_error())
    {
        LogError() << "Could not create PublisherPort for IntrospectionSubscriberPortChangingDataService";
        errorHandler(Error::kPORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTROSPECTIONCHANGINGDATASERVICE,
                     nullptr,
                     iox::ErrorLevel::SEVERE);
    }
    auto subscriberPortsData = maybePublisher.value();

    m_portIntrospection.registerPublisherPort(PublisherPortUserType(std::move(portGeneric)),
                                              PublisherPortUserType(std::move(portThroughput)),
                                              PublisherPortUserType(std::move(subscriberPortsData)));
    m_portIntrospection.run();
}

void PortManager::stopPortIntrospection() noexcept
{
    m_portIntrospection.stop();
}

void PortManager::doDiscovery() noexcept
{
    handlePublisherPorts();

    handleSubscriberPorts();

    handleApplications();

    handleInterfaces();

    handleNodes();

    handleConditionVariables();
}

void PortManager::handlePublisherPorts() noexcept
{
    // get the changes of publisher port offer state
    for (auto publisherPortData : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(publisherPortData);

        publisherPort.tryGetCaProMessage().and_then([&](auto caproMessage) {
            if (capro::CaproMessageType::OFFER == caproMessage.m_type)
            {
                addEntryToServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                          caproMessage.m_serviceDescription.getInstanceIDString());
            }
            else if (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type)
            {
                removeEntryFromServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                               caproMessage.m_serviceDescription.getInstanceIDString());
            }
            else
            {
                // protocol error
                errorHandler(Error::kPORT_MANAGER__HANDLE_PUBLISHER_PORTS_INVALID_CAPRO_MESSAGE,
                             nullptr,
                             iox::ErrorLevel::MODERATE);
            }

            m_portIntrospection.reportMessage(caproMessage);
            sendToAllMatchingSubscriberPorts(caproMessage, publisherPort);
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

        subscriberPort.tryGetCaProMessage().and_then([&](auto caproMessage) {
            if ((capro::CaproMessageType::SUB == caproMessage.m_type)
                || (capro::CaproMessageType::UNSUB == caproMessage.m_type))
            {
                if (!sendToAllMatchingPublisherPorts(caproMessage, subscriberPort))
                {
                    LogDebug() << "capro::SUB/UNSUB, no matching publisher!!";
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

            m_portIntrospection.reportMessage(caproMessage);
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
        if (interfacePortData->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            m_portPool->removeInterfacePort(interfacePortData);
            LogDebug() << "Destroyed InterfacePortData";
        }
    }

    if (interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active publisher ports to all new interfaces
        capro::CaproMessage caproMessage;
        caproMessage.m_type = capro::CaproMessageType::OFFER;
        for (auto publisherPortData : m_portPool->getPublisherPortDataList())
        {
            PublisherPortUserType publisherPort(publisherPortData);
            if (publisherPort.isOffered())
            {
                caproMessage.m_serviceDescription = publisherPort.getCaProServiceDescription();
                for (auto& interfacePortData : interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    // do not offer on same interface
                    if (publisherPort.getCaProServiceDescription().getSourceInterface()
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

void PortManager::handleNodes() noexcept
{
    /// @todo we have to update the introspection but node information is in process introspection which is not
    // accessible here. So currently nodes will be removed not before a process is removed
    // m_processIntrospection->removeNode(ProcessName_t(process.c_str()),
    // NodeName_t(node.c_str()));

    for (auto nodeData : m_portPool->getNodeDataList())
    {
        if (nodeData->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            m_portPool->removeNodeData(nodeData);
            LogDebug() << "Destroyed NodeData";
        }
    }
}

void PortManager::handleConditionVariables() noexcept
{
    for (auto conditionVariableData : m_portPool->getConditionVariableDataList())
    {
        if (conditionVariableData->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            m_portPool->removeConditionVariableData(conditionVariableData);
            LogDebug() << "Destroyed ConditionVariableData";
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
    for (auto port : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType sender(port);
        if (processName == sender.getProcessName())
        {
            destroyPublisherPort(port);
        }
    }

    for (auto port : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortUserType subscriber(port);
        if (processName == subscriber.getProcessName())
        {
            destroySubscriberPort(port);
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

    for (auto nodeData : m_portPool->getNodeDataList())
    {
        if (processName == nodeData->m_process)
        {
            m_portPool->removeNodeData(nodeData);
            LogDebug() << "Deleted node of application " << processName;
        }
    }

    for (auto conditionVariableData : m_portPool->getConditionVariableDataList())
    {
        if (processName == conditionVariableData->m_process)
        {
            m_portPool->removeConditionVariableData(conditionVariableData);
            LogDebug() << "Deleted condition variable of application" << processName;
        }
    }
}

void PortManager::destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept
{
    // create temporary publisher ports to orderly shut this publisher down
    PublisherPortRouDiType publisherPortRoudi{publisherPortData};
    PublisherPortUserType publisherPortUser{publisherPortData};

    publisherPortRoudi.releaseAllChunks();
    publisherPortUser.stopOffer();

    // process STOP_OFFER for this publisher in RouDi and distribute it
    publisherPortRoudi.tryGetCaProMessage().and_then([&](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER);

        m_portIntrospection.reportMessage(caproMessage);
        removeEntryFromServiceRegistry(caproMessage.m_serviceDescription.getServiceIDString(),
                                       caproMessage.m_serviceDescription.getInstanceIDString());
        sendToAllMatchingSubscriberPorts(caproMessage, publisherPortRoudi);
        sendToAllMatchingInterfacePorts(caproMessage);
    });

    m_portIntrospection.removePublisher(publisherPortRoudi.getProcessName(),
                                        publisherPortRoudi.getCaProServiceDescription());

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
    subscriberPortRoudi.tryGetCaProMessage().and_then([&](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::UNSUB);

        m_portIntrospection.reportMessage(caproMessage);
        sendToAllMatchingPublisherPorts(caproMessage, subscriberPortRoudi);
    });

    m_portIntrospection.removeSubscriber(subscriberPortRoudi.getProcessName(),
                                         subscriberPortRoudi.getCaProServiceDescription());
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

cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortManager::acquirePublisherPortData(const capro::ServiceDescription& service,
                                      const popo::PublisherOptions& publisherOptions,
                                      const ProcessName_t& processName,
                                      mepoo::MemoryManager* payloadMemoryManager,
                                      const NodeName_t& node,
                                      const PortConfigInfo& portConfigInfo) noexcept
{
    if (doesViolateCommunicationPolicy<iox::build::CommunicationPolicy>(service).and_then(
            [&](const auto& usedByProcess) {
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
        service, payloadMemoryManager, processName, publisherOptions, portConfigInfo.memoryInfo);
    if (!maybePublisherPortData.has_error())
    {
        m_portIntrospection.addPublisher(maybePublisherPortData.value(), processName, service, node);
    }

    return maybePublisherPortData;
}

cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortManager::acquireSubscriberPortData(const capro::ServiceDescription& service,
                                       const popo::SubscriberOptions& subscriberOptions,
                                       const ProcessName_t& processName,
                                       const NodeName_t& node,
                                       const PortConfigInfo& portConfigInfo) noexcept
{
    auto maybeSubscriberPortData =
        m_portPool->addSubscriberPort(service, processName, subscriberOptions, portConfigInfo.memoryInfo);
    if (!maybeSubscriberPortData.has_error())
    {
        m_portIntrospection.addSubscriber(maybeSubscriberPortData.value(), processName, service, node);
    }

    return maybeSubscriberPortData;
}


/// @todo return a cxx::expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces interface,
                                                               const ProcessName_t& processName,
                                                               const NodeName_t& /*node*/) noexcept
{
    auto result = m_portPool->addInterfacePort(processName, interface);
    if (!result.has_error())
    {
        return result.value();
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
        return result.value();
    }
    else
    {
        return nullptr;
    }
}

void PortManager::addEntryToServiceRegistry(const capro::IdString_t& service,
                                            const capro::IdString_t& instance) noexcept
{
    m_serviceRegistry.add(service, instance);
    m_portPool->serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
}

void PortManager::removeEntryFromServiceRegistry(const capro::IdString_t& service,
                                                 const capro::IdString_t& instance) noexcept
{
    m_serviceRegistry.remove(service, instance);
    m_portPool->serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
}

/// @todo return a cxx::expected
runtime::NodeData* PortManager::acquireNodeData(const ProcessName_t& process, const NodeName_t& node) noexcept
{
    auto result = m_portPool->addNodeData(process, node, 0);
    if (!result.has_error())
    {
        return result.value();
    }
    else
    {
        return nullptr;
    }
}

cxx::expected<popo::ConditionVariableData*, PortPoolError>
PortManager::acquireConditionVariableData(const ProcessName_t& process) noexcept
{
    return m_portPool->addConditionVariableData(process);
}

} // namespace roudi
} // namespace iox
