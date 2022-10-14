// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"

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
        errorHandler(PoshError::PORT_MANAGER__PORT_POOL_UNAVAILABLE, iox::ErrorLevel::FATAL);
    }
    m_portPool = maybePortPool.value();

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface->introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        LogFatal() << "Could not get MemoryManager for introspection!";
        errorHandler(PoshError::PORT_MANAGER__INTROSPECTION_MEMORY_MANAGER_UNAVAILABLE, iox::ErrorLevel::FATAL);
    }
    auto introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

    popo::PublisherOptions registryPortOptions;
    registryPortOptions.historyCapacity = 1U;
    registryPortOptions.nodeName = iox::NodeName_t("Service Registry");
    registryPortOptions.offerOnCreate = true;

    // we cannot (fully) perform discovery without this port
    m_serviceRegistryPublisherPortData = acquireInternalPublisherPortDataWithoutDiscovery(
        {SERVICE_DISCOVERY_SERVICE_NAME, SERVICE_DISCOVERY_INSTANCE_NAME, SERVICE_DISCOVERY_EVENT_NAME},
        registryPortOptions,
        introspectionMemoryManager);

    // if we arrive here, the port for service discovery exists and we perform the discovery
    PublisherPortRouDiType serviceRegistryPort(*m_serviceRegistryPublisherPortData);
    doDiscoveryForPublisherPort(serviceRegistryPort);

    popo::PublisherOptions options;
    options.historyCapacity = 1U;
    options.nodeName = INTROSPECTION_NODE_NAME;
    // Remark: m_portIntrospection is not fully functional in base class RouDiBase (has no active publisher port)
    // are there used instances of RouDiBase?
    auto portGeneric = acquireInternalPublisherPortData(IntrospectionPortService, options, introspectionMemoryManager);

    auto portThroughput =
        acquireInternalPublisherPortData(IntrospectionPortThroughputService, options, introspectionMemoryManager);

    auto subscriberPortsData = acquireInternalPublisherPortData(
        IntrospectionSubscriberPortChangingDataService, options, introspectionMemoryManager);

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

    handleServerPorts();

    handleClientPorts();

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

        doDiscoveryForPublisherPort(publisherPort);

        // check if we have to destroy this publisher port
        if (publisherPort.toBeDestroyed())
        {
            destroyPublisherPort(publisherPortData);
        }
    }
}

void PortManager::doDiscoveryForPublisherPort(PublisherPortRouDiType& publisherPort) noexcept
{
    publisherPort.tryGetCaProMessage().and_then([this, &publisherPort](auto caproMessage) {
        m_portIntrospection.reportMessage(caproMessage);
        if (capro::CaproMessageType::OFFER == caproMessage.m_type)
        {
            this->addPublisherToServiceRegistry(caproMessage.m_serviceDescription);
        }
        else if (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type)
        {
            this->removePublisherFromServiceRegistry(caproMessage.m_serviceDescription);
        }
        else
        {
            LogWarn() << "CaPro protocol error for publisher from runtime '" << publisherPort.getRuntimeName()
                      << "' and with service description '" << publisherPort.getCaProServiceDescription()
                      << "'! Cannot handle CaProMessageType '" << caproMessage.m_type << "'";
            errorHandler(PoshError::PORT_MANAGER__HANDLE_PUBLISHER_PORTS_INVALID_CAPRO_MESSAGE,
                         iox::ErrorLevel::MODERATE);
            return;
        }

        this->sendToAllMatchingSubscriberPorts(caproMessage, publisherPort);
        // forward to interfaces
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });
}

void PortManager::handleSubscriberPorts() noexcept
{
    // get requests for change of subscription state of subscribers
    for (auto subscriberPortData : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortType subscriberPort(subscriberPortData);

        doDiscoveryForSubscriberPort(subscriberPort);

        // check if we have to destroy this subscriber port
        if (subscriberPort.toBeDestroyed())
        {
            destroySubscriberPort(subscriberPortData);
        }
    }
}

void PortManager::doDiscoveryForSubscriberPort(SubscriberPortType& subscriberPort) noexcept
{
    subscriberPort.tryGetCaProMessage().and_then([this, &subscriberPort](auto caproMessage) {
        if ((capro::CaproMessageType::SUB == caproMessage.m_type)
            || (capro::CaproMessageType::UNSUB == caproMessage.m_type))
        {
            m_portIntrospection.reportMessage(caproMessage, subscriberPort.getUniqueID());
            if (!this->sendToAllMatchingPublisherPorts(caproMessage, subscriberPort))
            {
                LogDebug() << "capro::SUB/UNSUB, no matching publisher for subscriber from runtime '"
                           << subscriberPort.getRuntimeName() << "' and with service description '"
                           << caproMessage.m_serviceDescription << "'!";
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK,
                                                subscriberPort.getCaProServiceDescription());
                auto returnMessage = subscriberPort.dispatchCaProMessageAndGetPossibleResponse(nackMessage);
                // No response on NACK messages
                cxx::Ensures(!returnMessage.has_value());
            }
        }
        else
        {
            LogWarn() << "CaPro protocol error for subscriber from runtime '" << subscriberPort.getRuntimeName()
                      << "' and with service description '" << subscriberPort.getCaProServiceDescription()
                      << "'! Cannot handle CaProMessageType '" << caproMessage.m_type << "'";
            errorHandler(PoshError::PORT_MANAGER__HANDLE_SUBSCRIBER_PORTS_INVALID_CAPRO_MESSAGE,
                         iox::ErrorLevel::MODERATE);
            return;
        }
    });
}

void PortManager::destroyClientPort(popo::ClientPortData* const clientPortData) noexcept
{
    cxx::Ensures(clientPortData != nullptr && "clientPortData must not be a nullptr");

    // create temporary client ports to orderly shut this client down
    popo::ClientPortRouDi clientPortRoudi(*clientPortData);
    popo::ClientPortUser clientPortUser(*clientPortData);

    clientPortUser.disconnect();

    // process DISCONNECT for this client in RouDi and distribute it
    clientPortRoudi.tryGetCaProMessage().and_then([this, &clientPortRoudi](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::DISCONNECT);

        /// @todo iox-#1128 report to port introspection
        this->sendToAllMatchingServerPorts(caproMessage, clientPortRoudi);
    });

    clientPortRoudi.releaseAllChunks();

    /// @todo iox-#1128 remove from to port introspection

    LogDebug() << "Destroy client port from runtime '" << clientPortData->m_runtimeName
               << "' and with service description '" << clientPortData->m_serviceDescription << "'";

    // delete client port from list after DISCONNECT was processed
    m_portPool->removeClientPort(clientPortData);
}

void PortManager::handleClientPorts() noexcept
{
    // get requests for change of connection state of clients
    for (auto clientPortData : m_portPool->getClientPortDataList())
    {
        popo::ClientPortRouDi clientPort(*clientPortData);

        doDiscoveryForClientPort(clientPort);

        // check if we have to destroy this clinet port
        if (clientPort.toBeDestroyed())
        {
            destroyClientPort(clientPortData);
        }
    }
}

void PortManager::doDiscoveryForClientPort(popo::ClientPortRouDi& clientPort) noexcept
{
    clientPort.tryGetCaProMessage().and_then([this, &clientPort](auto caproMessage) {
        if ((capro::CaproMessageType::CONNECT == caproMessage.m_type)
            || (capro::CaproMessageType::DISCONNECT == caproMessage.m_type))
        {
            /// @todo iox-#1128 report to port introspection
            if (!this->sendToAllMatchingServerPorts(caproMessage, clientPort))
            {
                LogDebug() << "capro::CONNECT/DISCONNECT, no matching server for client from runtime '"
                           << clientPort.getRuntimeName() << "' and with service description '"
                           << caproMessage.m_serviceDescription << "'!";
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK, clientPort.getCaProServiceDescription());
                auto returnMessage = clientPort.dispatchCaProMessageAndGetPossibleResponse(nackMessage);
                // No response on NACK messages
                cxx::Ensures(!returnMessage.has_value());
            }
        }
        else
        {
            LogWarn() << "CaPro protocol error for client from runtime '" << clientPort.getRuntimeName()
                      << "' and with service description '" << clientPort.getCaProServiceDescription()
                      << "'! Cannot handle CaProMessageType '" << caproMessage.m_type << "'";
            errorHandler(PoshError::PORT_MANAGER__HANDLE_CLIENT_PORTS_INVALID_CAPRO_MESSAGE, iox::ErrorLevel::MODERATE);
            return;
        }
    });
}

void PortManager::makeAllServerPortsToStopOffer() noexcept
{
    for (auto port : m_portPool->getServerPortDataList())
    {
        port->m_offeringRequested.store(false, std::memory_order_relaxed);

        popo::ServerPortRouDi serverPort(*port);
        doDiscoveryForServerPort(serverPort);
    }
}

void PortManager::destroyServerPort(popo::ServerPortData* const serverPortData) noexcept
{
    cxx::Ensures(serverPortData != nullptr && "serverPortData must not be a nullptr");

    // create temporary server ports to orderly shut this server down
    popo::ServerPortRouDi serverPortRoudi{*serverPortData};
    popo::ServerPortUser serverPortUser{*serverPortData};

    serverPortUser.stopOffer();

    // process STOP_OFFER for this server in RouDi and distribute it
    serverPortRoudi.tryGetCaProMessage().and_then([this, &serverPortRoudi](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER);
        cxx::Ensures(caproMessage.m_serviceType == capro::CaproServiceType::SERVER);

        /// @todo iox-#1128 report to port introspection
        this->removeServerFromServiceRegistry(caproMessage.m_serviceDescription);
        this->sendToAllMatchingClientPorts(caproMessage, serverPortRoudi);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });

    serverPortRoudi.releaseAllChunks();

    /// @todo iox-#1128 remove from port introspection

    LogDebug() << "Destroy server port from runtime '" << serverPortData->m_runtimeName
               << "' and with service description '" << serverPortData->m_serviceDescription << "'";

    // delete server port from list after STOP_OFFER was processed
    m_portPool->removeServerPort(serverPortData);
}

void PortManager::handleServerPorts() noexcept
{
    // get the changes of server port offer state
    for (auto serverPortData : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi serverPort(*serverPortData);

        doDiscoveryForServerPort(serverPort);

        // check if we have to destroy this server port
        if (serverPort.toBeDestroyed())
        {
            destroyServerPort(serverPortData);
        }
    }
}

void PortManager::doDiscoveryForServerPort(popo::ServerPortRouDi& serverPort) noexcept
{
    serverPort.tryGetCaProMessage().and_then([this, &serverPort](auto caproMessage) {
        /// @todo iox-#1128 report to port instrospection

        if (capro::CaproMessageType::OFFER == caproMessage.m_type)
        {
            this->addServerToServiceRegistry(caproMessage.m_serviceDescription);
        }
        else if (capro::CaproMessageType::STOP_OFFER == caproMessage.m_type)
        {
            this->removeServerFromServiceRegistry(caproMessage.m_serviceDescription);
        }
        else
        {
            LogWarn() << "CaPro protocol error for server from runtime '" << serverPort.getRuntimeName()
                      << "' and with service description '" << serverPort.getCaProServiceDescription()
                      << "'! Cannot handle CaProMessageType '" << caproMessage.m_type << "'";
            errorHandler(PoshError::PORT_MANAGER__HANDLE_SERVER_PORTS_INVALID_CAPRO_MESSAGE, iox::ErrorLevel::MODERATE);
            return;
        }

        this->sendToAllMatchingClientPorts(caproMessage, serverPort);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });
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
            LogDebug() << "Destroy interface port from runtime '" << interfacePortData->m_runtimeName
                       << "' and with service description '" << interfacePortData->m_serviceDescription << "'";
            m_portPool->removeInterfacePort(interfacePortData);
        }
    }

    if (interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active publisher ports to all new interfaces
        capro::CaproMessage caproMessage;
        caproMessage.m_type = capro::CaproMessageType::OFFER;
        caproMessage.m_serviceType = capro::CaproServiceType::PUBLISHER;
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
        // provide offer information from all active server ports to all new interfaces
        caproMessage.m_serviceType = capro::CaproServiceType::SERVER;
        for (auto serverPortData : m_portPool->getServerPortDataList())
        {
            popo::ServerPortUser serverPort(*serverPortData);
            if (serverPort.isOffered())
            {
                caproMessage.m_serviceDescription = serverPort.getCaProServiceDescription();
                for (auto& interfacePortData : interfacePortsForInitialForwarding)
                {
                    auto interfacePort = popo::InterfacePort(interfacePortData);
                    // do not offer on same interface
                    if (serverPort.getCaProServiceDescription().getSourceInterface()
                        != interfacePort.getCaProServiceDescription().getSourceInterface())
                    {
                        interfacePort.dispatchCaProMessage(caproMessage);
                    }
                }
            }
        }
    }
}

void PortManager::handleNodes() noexcept
{
    /// @todo iox-#518 we have to update the introspection but node information is in process introspection which is not
    // accessible here. So currently nodes will be removed not before a process is removed
    // m_processIntrospection->removeNode(RuntimeName_t(process.c_str()),
    // NodeName_t(node.c_str()));

    for (auto nodeData : m_portPool->getNodeDataList())
    {
        if (nodeData->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            LogDebug() << "Destroy NodeData from runtime '" << nodeData->m_runtimeName << "' and node name '"
                       << nodeData->m_nodeName << "'";
            m_portPool->removeNodeData(nodeData);
        }
    }
}

void PortManager::handleConditionVariables() noexcept
{
    for (auto conditionVariableData : m_portPool->getConditionVariableDataList())
    {
        if (conditionVariableData->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            LogDebug() << "Destroy ConditionVariableData from runtime '" << conditionVariableData->m_runtimeName << "'";
            m_portPool->removeConditionVariableData(conditionVariableData);
        }
    }
}

bool PortManager::isCompatiblePubSub(const PublisherPortRouDiType& publisher,
                                     const SubscriberPortType& subscriber) const noexcept
{
    if (subscriber.getCaProServiceDescription() != publisher.getCaProServiceDescription())
    {
        return false;
    }

    auto& pubOpts = publisher.getOptions();
    auto& subOpts = subscriber.getOptions();

    const bool blockingPoliciesAreCompatible =
        !(pubOpts.subscriberTooSlowPolicy == popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA
          && subOpts.queueFullPolicy == popo::QueueFullPolicy::BLOCK_PRODUCER);

    const bool historyRequestIsCompatible = !subOpts.requiresPublisherHistorySupport || pubOpts.historyCapacity > 0;

    return blockingPoliciesAreCompatible && historyRequestIsCompatible;
}

bool PortManager::sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                                  SubscriberPortType& subscriberSource) noexcept
{
    bool publisherFound = false;
    for (auto publisherPortData : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(publisherPortData);

        auto messageInterface = message.m_serviceDescription.getSourceInterface();
        auto publisherInterface = publisherPort.getCaProServiceDescription().getSourceInterface();

        // internal publisher receive all messages all other publishers receive only messages if
        // they do not have the same interface otherwise we have cyclic connections in gateways
        if (publisherInterface != capro::Interfaces::INTERNAL && publisherInterface == messageInterface)
        {
            break;
        }

        if (isCompatiblePubSub(publisherPort, subscriberSource))
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
                m_portIntrospection.reportMessage(publisherResponse.value(), subscriberSource.getUniqueID());
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

        auto messageInterface = message.m_serviceDescription.getSourceInterface();
        auto subscriberInterface = subscriberPort.getCaProServiceDescription().getSourceInterface();

        // internal subscriber receive all messages all other subscribers receive only messages if
        // they do not have the same interface otherwise we have cyclic connections in gateways
        if (subscriberInterface != capro::Interfaces::INTERNAL && subscriberInterface == messageInterface)
        {
            break;
        }

        if (isCompatiblePubSub(publisherSource, subscriberPort))
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

bool PortManager::isCompatibleClientServer(const popo::ServerPortRouDi& server,
                                           const popo::ClientPortRouDi& client) const noexcept
{
    if (server.getCaProServiceDescription() != client.getCaProServiceDescription())
    {
        return false;
    }

    auto requestMatch = !(client.getServerTooSlowPolicy() == popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA
                          && server.getRequestQueueFullPolicy() == popo::QueueFullPolicy::BLOCK_PRODUCER);

    auto responseMatch = !(server.getClientTooSlowPolicy() == popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA
                           && client.getResponseQueueFullPolicy() == popo::QueueFullPolicy::BLOCK_PRODUCER);

    return requestMatch && responseMatch;
}

void PortManager::sendToAllMatchingClientPorts(const capro::CaproMessage& message,
                                               popo::ServerPortRouDi& serverSource) noexcept
{
    for (auto clientPortData : m_portPool->getClientPortDataList())
    {
        popo::ClientPortRouDi clientPort(*clientPortData);
        if (isCompatibleClientServer(serverSource, clientPort))
        {
            // send OFFER/STOP_OFFER to client
            auto clientResponse = clientPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the clients react on the change, process it immediately on server side
            if (clientResponse.has_value())
            {
                // we only expect reaction on CONNECT
                cxx::Expects(capro::CaproMessageType::CONNECT == clientResponse.value().m_type);

                /// @todo iox-#518 inform port introspection about client

                // send CONNECT to server
                auto serverResponse = serverSource.dispatchCaProMessageAndGetPossibleResponse(clientResponse.value());
                if (serverResponse.has_value())
                {
                    // sende responsee to client port
                    auto returnMessage = clientPort.dispatchCaProMessageAndGetPossibleResponse(serverResponse.value());

                    // ACK or NACK are sent back to the client port, no further response from this one expected
                    cxx::Ensures(!returnMessage.has_value());

                    /// @todo iox-#1128 inform port introspection about server
                }
            }
        }
    }
}

bool PortManager::sendToAllMatchingServerPorts(const capro::CaproMessage& message,
                                               popo::ClientPortRouDi& clientSource) noexcept
{
    bool serverFound = false;
    for (auto serverPortData : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi serverPort(*serverPortData);
        if (isCompatibleClientServer(serverPort, clientSource))
        {
            // send CONNECT/DISCONNECT to server
            auto serverResponse = serverPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the server react on the change, process it immediately on client side
            if (serverResponse.has_value())
            {
                // send response to client port
                auto returnMessage = clientSource.dispatchCaProMessageAndGetPossibleResponse(serverResponse.value());

                // ACK or NACK are sent back to the client port, no further response from this one expected
                cxx::Ensures(!returnMessage.has_value());

                /// @todo iox-#1128 inform port introspection about client
            }
            serverFound = true;
        }
    }
    return serverFound;
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

void PortManager::unblockProcessShutdown(const RuntimeName_t& runtimeName) noexcept
{
    for (auto port : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(port);
        if (runtimeName == publisherPort.getRuntimeName())
        {
            port->m_offeringRequested.store(false, std::memory_order_relaxed);
            doDiscoveryForPublisherPort(publisherPort);
        }
    }

    for (auto port : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi serverPort(*port);
        if (runtimeName == serverPort.getRuntimeName())
        {
            port->m_offeringRequested.store(false, std::memory_order_relaxed);
            doDiscoveryForServerPort(serverPort);
        }
    }
}

void PortManager::unblockRouDiShutdown() noexcept
{
    makeAllPublisherPortsToStopOffer();
    makeAllServerPortsToStopOffer();
}

void PortManager::makeAllPublisherPortsToStopOffer() noexcept
{
    for (auto port : m_portPool->getPublisherPortDataList())
    {
        port->m_offeringRequested.store(false, std::memory_order_relaxed);

        PublisherPortRouDiType publisherPort(port);
        doDiscoveryForPublisherPort(publisherPort);
    }
}

void PortManager::deletePortsOfProcess(const RuntimeName_t& runtimeName) noexcept
{
    // If we delete all ports from RouDi we need to reset the service registry publisher
    if (runtimeName == RuntimeName_t(iox::roudi::IPC_CHANNEL_ROUDI_NAME))
    {
        m_serviceRegistryPublisherPortData.reset();
    }
    for (auto port : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType sender(port);
        if (runtimeName == sender.getRuntimeName())
        {
            destroyPublisherPort(port);
        }
    }

    for (auto port : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortUserType subscriber(port);
        if (runtimeName == subscriber.getRuntimeName())
        {
            destroySubscriberPort(port);
        }
    }

    for (auto port : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi server(*port);
        if (runtimeName == server.getRuntimeName())
        {
            destroyServerPort(port);
        }
    }

    for (auto port : m_portPool->getClientPortDataList())
    {
        popo::ClientPortRouDi client(*port);
        if (runtimeName == client.getRuntimeName())
        {
            destroyClientPort(port);
        }
    }

    for (auto port : m_portPool->getInterfacePortDataList())
    {
        popo::InterfacePort interface(port);
        if (runtimeName == interface.getRuntimeName())
        {
            m_portPool->removeInterfacePort(port);
            LogDebug() << "Deleted Interface of application " << runtimeName;
        }
    }

    for (auto nodeData : m_portPool->getNodeDataList())
    {
        if (runtimeName == nodeData->m_runtimeName)
        {
            m_portPool->removeNodeData(nodeData);
            LogDebug() << "Deleted node of application " << runtimeName;
        }
    }

    for (auto conditionVariableData : m_portPool->getConditionVariableDataList())
    {
        if (runtimeName == conditionVariableData->m_runtimeName)
        {
            m_portPool->removeConditionVariableData(conditionVariableData);
            LogDebug() << "Deleted condition variable of application" << runtimeName;
        }
    }
}

void PortManager::destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept
{
    // create temporary publisher ports to orderly shut this publisher down
    PublisherPortRouDiType publisherPortRoudi{publisherPortData};
    PublisherPortUserType publisherPortUser{publisherPortData};

    publisherPortUser.stopOffer();

    // process STOP_OFFER for this publisher in RouDi and distribute it
    publisherPortRoudi.tryGetCaProMessage().and_then([this, &publisherPortRoudi](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER);

        m_portIntrospection.reportMessage(caproMessage);
        this->removePublisherFromServiceRegistry(caproMessage.m_serviceDescription);
        this->sendToAllMatchingSubscriberPorts(caproMessage, publisherPortRoudi);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });

    publisherPortRoudi.releaseAllChunks();

    m_portIntrospection.removePublisher(publisherPortUser);

    LogDebug() << "Destroy publisher port from runtime '" << publisherPortData->m_runtimeName
               << "' and with service description '" << publisherPortData->m_serviceDescription << "'";
    // delete publisher port from list after STOP_OFFER was processed
    m_portPool->removePublisherPort(publisherPortData);
}

void PortManager::destroySubscriberPort(SubscriberPortType::MemberType_t* const subscriberPortData) noexcept
{
    // create temporary subscriber ports to orderly shut this subscriber down
    SubscriberPortType subscriberPortRoudi(subscriberPortData);
    SubscriberPortUserType subscriberPortUser(subscriberPortData);

    subscriberPortUser.unsubscribe();

    // process UNSUB for this subscriber in RouDi and distribute it
    subscriberPortRoudi.tryGetCaProMessage().and_then([this, &subscriberPortRoudi](auto caproMessage) {
        cxx::Ensures(caproMessage.m_type == capro::CaproMessageType::UNSUB);

        m_portIntrospection.reportMessage(caproMessage);
        this->sendToAllMatchingPublisherPorts(caproMessage, subscriberPortRoudi);
    });

    subscriberPortRoudi.releaseAllChunks();

    m_portIntrospection.removeSubscriber(subscriberPortUser);

    LogDebug() << "Destroy subscriber port from runtime '" << subscriberPortData->m_runtimeName
               << "' and with service description '" << subscriberPortData->m_serviceDescription << "'";
    // delete subscriber port from list after UNSUB was processed
    m_portPool->removeSubscriberPort(subscriberPortData);
}

cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortManager::acquirePublisherPortData(const capro::ServiceDescription& service,
                                      const popo::PublisherOptions& publisherOptions,
                                      const RuntimeName_t& runtimeName,
                                      mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                      const PortConfigInfo& portConfigInfo) noexcept
{
    return acquirePublisherPortDataWithoutDiscovery(
               service, publisherOptions, runtimeName, payloadDataSegmentMemoryManager, portConfigInfo)
        .and_then([&](auto publisherPortData) {
            PublisherPortRouDiType port(publisherPortData);
            this->doDiscoveryForPublisherPort(port);
        });
}

cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortManager::acquirePublisherPortDataWithoutDiscovery(const capro::ServiceDescription& service,
                                                      const popo::PublisherOptions& publisherOptions,
                                                      const RuntimeName_t& runtimeName,
                                                      mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                                      const PortConfigInfo& portConfigInfo) noexcept
{
    if (doesViolateCommunicationPolicy<iox::build::CommunicationPolicy>(service).and_then(
            [&](const auto& usedByProcess) {
                LogWarn()
                    << "Process '" << runtimeName
                    << "' violates the communication policy by requesting a PublisherPort which is already used by '"
                    << usedByProcess << "' with service '" << service.operator cxx::Serialization().toString() << "'.";
            }))
    {
        errorHandler(PoshError::POSH__PORT_MANAGER_PUBLISHERPORT_NOT_UNIQUE, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS);
    }

    if (runtimeName == RuntimeName_t{IPC_CHANNEL_ROUDI_NAME})
    {
        m_internalServices.push_back(service);
    }
    else if (isInternal(service))
    {
        errorHandler(PoshError::POSH__PORT_MANAGER_INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN);
    }

    // we can create a new port
    auto maybePublisherPortData = m_portPool->addPublisherPort(
        service, payloadDataSegmentMemoryManager, runtimeName, publisherOptions, portConfigInfo.memoryInfo);

    if (!maybePublisherPortData.has_error())
    {
        auto publisherPortData = maybePublisherPortData.value();
        if (publisherPortData)
        {
            m_portIntrospection.addPublisher(*publisherPortData);
        }
    }

    return maybePublisherPortData;
}

PublisherPortRouDiType::MemberType_t*
PortManager::acquireInternalPublisherPortData(const capro::ServiceDescription& service,
                                              const popo::PublisherOptions& publisherOptions,
                                              mepoo::MemoryManager* const payloadDataSegmentMemoryManager) noexcept
{
    return acquirePublisherPortDataWithoutDiscovery(
               service, publisherOptions, IPC_CHANNEL_ROUDI_NAME, payloadDataSegmentMemoryManager, PortConfigInfo())
        .or_else([&service](auto&) {
            LogFatal() << "Could not create PublisherPort for internal service " << service;
            errorHandler(PoshError::PORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTERNAL_SERVICE, ErrorLevel::FATAL);
        })
        .and_then([&](auto publisherPortData) {
            // now the port to send registry information exists and can be used to publish service registry changes
            PublisherPortRouDiType port(publisherPortData);
            this->doDiscoveryForPublisherPort(port);
        })
        .value();
}

PublisherPortRouDiType::MemberType_t* PortManager::acquireInternalPublisherPortDataWithoutDiscovery(
    const capro::ServiceDescription& service,
    const popo::PublisherOptions& publisherOptions,
    mepoo::MemoryManager* const payloadDataSegmentMemoryManager) noexcept
{
    return acquirePublisherPortDataWithoutDiscovery(
               service, publisherOptions, IPC_CHANNEL_ROUDI_NAME, payloadDataSegmentMemoryManager, PortConfigInfo())
        .or_else([&service](auto&) {
            LogFatal() << "Could not create PublisherPort for internal service " << service;
            errorHandler(PoshError::PORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTERNAL_SERVICE, ErrorLevel::FATAL);
        })
        .value();
}

cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortManager::acquireSubscriberPortData(const capro::ServiceDescription& service,
                                       const popo::SubscriberOptions& subscriberOptions,
                                       const RuntimeName_t& runtimeName,
                                       const PortConfigInfo& portConfigInfo) noexcept
{
    auto maybeSubscriberPortData =
        m_portPool->addSubscriberPort(service, runtimeName, subscriberOptions, portConfigInfo.memoryInfo);
    if (!maybeSubscriberPortData.has_error())
    {
        auto subscriberPortData = maybeSubscriberPortData.value();
        if (subscriberPortData)
        {
            m_portIntrospection.addSubscriber(*subscriberPortData);

            // we do discovery here for trying to connect with publishers if subscribe on create is desired
            SubscriberPortType subscriberPort(subscriberPortData);
            doDiscoveryForSubscriberPort(subscriberPort);
        }
    }

    return maybeSubscriberPortData;
}

cxx::expected<popo::ClientPortData*, PortPoolError>
PortManager::acquireClientPortData(const capro::ServiceDescription& service,
                                   const popo::ClientOptions& clientOptions,
                                   const RuntimeName_t& runtimeName,
                                   mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                   const PortConfigInfo& portConfigInfo) noexcept
{
    // we can create a new port
    return m_portPool
        ->addClientPort(service, payloadDataSegmentMemoryManager, runtimeName, clientOptions, portConfigInfo.memoryInfo)
        .and_then([this](auto clientPortData) {
            /// @todo iox-#1128 add to port introspection

            // we do discovery here for trying to connect the client if offer on create is desired
            popo::ClientPortRouDi clientPort(*clientPortData);
            this->doDiscoveryForClientPort(clientPort);
        });
}

cxx::expected<popo::ServerPortData*, PortPoolError>
PortManager::acquireServerPortData(const capro::ServiceDescription& service,
                                   const popo::ServerOptions& serverOptions,
                                   const RuntimeName_t& runtimeName,
                                   mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                   const PortConfigInfo& portConfigInfo) noexcept
{
    // it is not allowed to have two servers with the same ServiceDescription;
    // check if the server is already in the list
    for (const auto serverPortData : m_portPool->getServerPortDataList())
    {
        if (service == serverPortData->m_serviceDescription)
        {
            if (serverPortData->m_toBeDestroyed)
            {
                destroyServerPort(serverPortData);
                continue;
            }
            LogWarn() << "Process '" << runtimeName
                      << "' violates the communication policy by requesting a ServerPort which is already used by '"
                      << serverPortData->m_runtimeName << "' with service '"
                      << service.operator cxx::Serialization().toString() << "'.";
            errorHandler(PoshError::POSH__PORT_MANAGER_SERVERPORT_NOT_UNIQUE, ErrorLevel::MODERATE);
            return cxx::error<PortPoolError>(PortPoolError::UNIQUE_SERVER_PORT_ALREADY_EXISTS);
        }
    }

    // we can create a new port
    return m_portPool
        ->addServerPort(service, payloadDataSegmentMemoryManager, runtimeName, serverOptions, portConfigInfo.memoryInfo)
        .and_then([this](auto serverPortData) {
            /// @todo iox-#1128 add to port introspection

            // we do discovery here for trying to connect the waiting client if offer on create is desired
            popo::ServerPortRouDi serverPort(*serverPortData);
            this->doDiscoveryForServerPort(serverPort);
        });
}

/// @todo iox-#518 return a cxx::expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces interface,
                                                               const RuntimeName_t& runtimeName,
                                                               const NodeName_t& /*node*/) noexcept
{
    auto result = m_portPool->addInterfacePort(runtimeName, interface);
    if (!result.has_error())
    {
        return result.value();
    }
    else
    {
        return nullptr;
    }
}

void PortManager::publishServiceRegistry() const noexcept
{
    if (!m_serviceRegistryPublisherPortData.has_value())
    {
        // should not happen (except during RouDi shutdown)
        // the port always exists, otherwise we would terminate during startup
        LogWarn() << "Could not publish service registry!";
        return;
    }
    PublisherPortUserType publisher(m_serviceRegistryPublisherPortData.value());
    publisher
        .tryAllocateChunk(sizeof(ServiceRegistry),
                          alignof(ServiceRegistry),
                          CHUNK_NO_USER_HEADER_SIZE,
                          CHUNK_NO_USER_HEADER_ALIGNMENT)
        .and_then([&](auto& chunk) {
            // It's ok to copy as the modifications happen in the same thread and not concurrently
            new (chunk->userPayload()) ServiceRegistry(m_serviceRegistry);

            publisher.sendChunk(chunk);
        })
        .or_else([](auto&) { LogWarn() << "Could not allocate a chunk for the service registry!"; });
}

const ServiceRegistry& PortManager::serviceRegistry() const noexcept
{
    return m_serviceRegistry;
}

void PortManager::addPublisherToServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.addPublisher(service).or_else([&](auto&) {
        LogWarn() << "Could not add publisher with service description '" << service << "' to service registry!";
        errorHandler(PoshError::POSH__PORT_MANAGER_COULD_NOT_ADD_SERVICE_TO_REGISTRY, ErrorLevel::MODERATE);
    });
    publishServiceRegistry();
}

void PortManager::removePublisherFromServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.removePublisher(service);
    publishServiceRegistry();
}

void PortManager::addServerToServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.addServer(service).or_else([&](auto&) {
        LogWarn() << "Could not add server with service description '" << service << "' to service registry!";
        errorHandler(PoshError::POSH__PORT_MANAGER_COULD_NOT_ADD_SERVICE_TO_REGISTRY, ErrorLevel::MODERATE);
    });
    publishServiceRegistry();
}

void PortManager::removeServerFromServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.removeServer(service);
    publishServiceRegistry();
}

cxx::expected<runtime::NodeData*, PortPoolError> PortManager::acquireNodeData(const RuntimeName_t& runtimeName,
                                                                              const NodeName_t& nodeName) noexcept
{
    return m_portPool->addNodeData(runtimeName, nodeName, 0);
}

cxx::expected<popo::ConditionVariableData*, PortPoolError>
PortManager::acquireConditionVariableData(const RuntimeName_t& runtimeName) noexcept
{
    return m_portPool->addConditionVariableData(runtimeName);
}

bool PortManager::isInternal(const capro::ServiceDescription& service) const noexcept
{
    for (auto& internalService : m_internalServices)
    {
        if (service == internalService)
        {
            return true;
        }
    }
    return false;
}


} // namespace roudi
} // namespace iox
