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
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iox/assertions.hpp"
#include "iox/logging.hpp"
#include "iox/vector.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToCaProInterface(const capro::IdString_t& str) noexcept
{
    auto result = convert::from_string<int32_t>(str.c_str());
    if (!result.has_value())
    {
        IOX_LOG(WARN, "conversion failure");
        return capro::Interfaces::INTERNAL;
    }

    const auto i = result.value();

    if (i >= static_cast<int32_t>(capro::Interfaces::INTERFACE_END))
    {
        IOX_LOG(WARN, "invalid enum (out of range: " << i << ")");
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
        IOX_LOG(FATAL, "Could not get PortPool!");
        IOX_REPORT_FATAL(PoshError::PORT_MANAGER__PORT_POOL_UNAVAILABLE);
    }
    m_portPool = maybePortPool.value();

    auto maybeDiscoveryMemoryManager = m_roudiMemoryInterface->discoveryMemoryManager();
    if (!maybeDiscoveryMemoryManager.has_value())
    {
        IOX_LOG(FATAL, "Could not get MemoryManager for discovery!");
        IOX_REPORT_FATAL(PoshError::PORT_MANAGER__DISCOVERY_MEMORY_MANAGER_UNAVAILABLE);
    }
    auto& discoveryMemoryManager = maybeDiscoveryMemoryManager.value();

    popo::PublisherOptions registryPortOptions;
    registryPortOptions.historyCapacity = 1U;
    registryPortOptions.nodeName = iox::NodeName_t("Service Registry");
    registryPortOptions.offerOnCreate = true;

    // we cannot (fully) perform discovery without this port
    m_serviceRegistryPublisherPortData = acquireInternalPublisherPortDataWithoutDiscovery(
        {SERVICE_DISCOVERY_SERVICE_NAME, SERVICE_DISCOVERY_INSTANCE_NAME, SERVICE_DISCOVERY_EVENT_NAME},
        registryPortOptions,
        discoveryMemoryManager);

    // if we arrive here, the port for service discovery exists and we perform the discovery
    PublisherPortRouDiType serviceRegistryPort(*m_serviceRegistryPublisherPortData);
    doDiscoveryForPublisherPort(serviceRegistryPort);

    auto maybeIntrospectionMemoryManager = m_roudiMemoryInterface->introspectionMemoryManager();
    if (!maybeIntrospectionMemoryManager.has_value())
    {
        IOX_LOG(FATAL, "Could not get MemoryManager for introspection!");
        IOX_REPORT_FATAL(PoshError::PORT_MANAGER__INTROSPECTION_MEMORY_MANAGER_UNAVAILABLE);
    }
    auto& introspectionMemoryManager = maybeIntrospectionMemoryManager.value();

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

    handleConditionVariables();

    publishServiceRegistry();
}

void PortManager::handlePublisherPorts() noexcept
{
    // get the changes of publisher port offer state
    auto& publisherPorts = m_portPool->getPublisherPortDataList();
    auto port = publisherPorts.begin();
    while (port != publisherPorts.end())
    {
        auto currentPort = port++;
        PublisherPortRouDiType publisherPort(currentPort.to_ptr());

        doDiscoveryForPublisherPort(publisherPort);

        // check if we have to destroy this publisher port
        if (publisherPort.toBeDestroyed())
        {
            destroyPublisherPort(currentPort.to_ptr());
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
            IOX_LOG(WARN,
                    "CaPro protocol error for publisher from runtime '"
                        << publisherPort.getRuntimeName() << "' and with service description '"
                        << publisherPort.getCaProServiceDescription() << "'! Cannot handle CaProMessageType '"
                        << caproMessage.m_type << "'");
            IOX_REPORT(PoshError::PORT_MANAGER__HANDLE_PUBLISHER_PORTS_INVALID_CAPRO_MESSAGE, iox::er::RUNTIME_ERROR);
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
    auto& subscriberPorts = m_portPool->getSubscriberPortDataList();
    auto port = subscriberPorts.begin();
    while (port != subscriberPorts.end())
    {
        auto currentPort = port++;
        SubscriberPortType subscriberPort(currentPort.to_ptr());

        doDiscoveryForSubscriberPort(subscriberPort);

        // check if we have to destroy this subscriber port
        if (subscriberPort.toBeDestroyed())
        {
            destroySubscriberPort(currentPort.to_ptr());
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
                IOX_LOG(DEBUG,
                        "capro::SUB/UNSUB, no matching publisher for subscriber from runtime '"
                            << subscriberPort.getRuntimeName() << "' and with service description '"
                            << caproMessage.m_serviceDescription << "'!");
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK,
                                                subscriberPort.getCaProServiceDescription());
                subscriberPort.dispatchCaProMessageAndGetPossibleResponse(nackMessage).and_then([](auto& response) {
                    IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                    IOX_PANIC("Expected no response on NACK messages");
                });
            }
        }
        else
        {
            IOX_LOG(WARN,
                    "CaPro protocol error for subscriber from runtime '"
                        << subscriberPort.getRuntimeName() << "' and with service description '"
                        << subscriberPort.getCaProServiceDescription() << "'! Cannot handle CaProMessageType '"
                        << caproMessage.m_type << "'");
            IOX_REPORT(PoshError::PORT_MANAGER__HANDLE_SUBSCRIBER_PORTS_INVALID_CAPRO_MESSAGE, iox::er::RUNTIME_ERROR);
            return;
        }
    });
}

void PortManager::destroyClientPort(popo::ClientPortData* const clientPortData) noexcept
{
    IOX_ENFORCE(clientPortData != nullptr, "clientPortData must not be a nullptr");

    // create temporary client ports to orderly shut this client down
    popo::ClientPortRouDi clientPortRoudi(*clientPortData);
    popo::ClientPortUser clientPortUser(*clientPortData);

    clientPortUser.disconnect();

    // process DISCONNECT for this client in RouDi and distribute it
    clientPortRoudi.tryGetCaProMessage().and_then([this, &clientPortRoudi](auto caproMessage) {
        IOX_ENFORCE(caproMessage.m_type == capro::CaproMessageType::DISCONNECT, "Received wrong 'CaproMessageType'!");

        /// @todo iox-#1128 report to port introspection
        this->sendToAllMatchingServerPorts(caproMessage, clientPortRoudi);
    });

    clientPortRoudi.releaseAllChunks();

    /// @todo iox-#1128 remove from to port introspection

    IOX_LOG(DEBUG,
            "Destroy client port from runtime '" << clientPortData->m_runtimeName << "' and with service description '"
                                                 << clientPortData->m_serviceDescription << "'");

    // delete client port from list after DISCONNECT was processed
    m_portPool->removeClientPort(clientPortData);
}

void PortManager::handleClientPorts() noexcept
{
    // get requests for change of connection state of clients
    auto& clientPorts = m_portPool->getClientPortDataList();
    auto port = clientPorts.begin();
    while (port != clientPorts.end())
    {
        auto currentPort = port++;
        popo::ClientPortRouDi clientPort(*currentPort);

        doDiscoveryForClientPort(clientPort);

        // check if we have to destroy this clinet port
        if (clientPort.toBeDestroyed())
        {
            destroyClientPort(currentPort.to_ptr());
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
                IOX_LOG(DEBUG,
                        "capro::CONNECT/DISCONNECT, no matching server for client from runtime '"
                            << clientPort.getRuntimeName() << "' and with service description '"
                            << caproMessage.m_serviceDescription << "'!");
                capro::CaproMessage nackMessage(capro::CaproMessageType::NACK, clientPort.getCaProServiceDescription());
                clientPort.dispatchCaProMessageAndGetPossibleResponse(nackMessage).and_then([](auto& response) {
                    IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                    IOX_PANIC("Expected no response on NACK messages");
                });
            }
        }
        else
        {
            IOX_LOG(WARN,
                    "CaPro protocol error for client from runtime '"
                        << clientPort.getRuntimeName() << "' and with service description '"
                        << clientPort.getCaProServiceDescription() << "'! Cannot handle CaProMessageType '"
                        << caproMessage.m_type << "'");
            IOX_REPORT(PoshError::PORT_MANAGER__HANDLE_CLIENT_PORTS_INVALID_CAPRO_MESSAGE, iox::er::RUNTIME_ERROR);
            return;
        }
    });
}

void PortManager::makeAllServerPortsToStopOffer() noexcept
{
    for (auto& port : m_portPool->getServerPortDataList())
    {
        port.m_offeringRequested.store(false, std::memory_order_relaxed);

        popo::ServerPortRouDi serverPort(port);
        doDiscoveryForServerPort(serverPort);
    }
}

void PortManager::destroyServerPort(popo::ServerPortData* const serverPortData) noexcept
{
    IOX_ENFORCE(serverPortData != nullptr, "serverPortData must not be a nullptr");

    // create temporary server ports to orderly shut this server down
    popo::ServerPortRouDi serverPortRoudi{*serverPortData};
    popo::ServerPortUser serverPortUser{*serverPortData};

    serverPortUser.stopOffer();

    // process STOP_OFFER for this server in RouDi and distribute it
    serverPortRoudi.tryGetCaProMessage().and_then([this, &serverPortRoudi](auto caproMessage) {
        IOX_ENFORCE(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER, "Received wrong 'CaproMessageType'!");
        IOX_ENFORCE(caproMessage.m_serviceType == capro::CaproServiceType::SERVER,
                    "Received wrong 'CaproServiceType'!");

        /// @todo iox-#1128 report to port introspection
        this->removeServerFromServiceRegistry(caproMessage.m_serviceDescription);
        this->sendToAllMatchingClientPorts(caproMessage, serverPortRoudi);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });

    serverPortRoudi.releaseAllChunks();

    /// @todo iox-#1128 remove from port introspection

    IOX_LOG(DEBUG,
            "Destroy server port from runtime '" << serverPortData->m_runtimeName << "' and with service description '"
                                                 << serverPortData->m_serviceDescription << "'");

    // delete server port from list after STOP_OFFER was processed
    m_portPool->removeServerPort(serverPortData);
}

void PortManager::handleServerPorts() noexcept
{
    // get the changes of server port offer state
    auto& serverPorts = m_portPool->getServerPortDataList();
    auto port = serverPorts.begin();
    while (port != serverPorts.end())
    {
        auto currentPort = port++;
        popo::ServerPortRouDi serverPort(*currentPort);

        doDiscoveryForServerPort(serverPort);

        // check if we have to destroy this server port
        if (serverPort.toBeDestroyed())
        {
            destroyServerPort(currentPort.to_ptr());
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
            IOX_LOG(WARN,
                    "CaPro protocol error for server from runtime '"
                        << serverPort.getRuntimeName() << "' and with service description '"
                        << serverPort.getCaProServiceDescription() << "'! Cannot handle CaProMessageType '"
                        << caproMessage.m_type << "'");
            IOX_REPORT(PoshError::PORT_MANAGER__HANDLE_SERVER_PORTS_INVALID_CAPRO_MESSAGE, iox::er::RUNTIME_ERROR);
            return;
        }

        this->sendToAllMatchingClientPorts(caproMessage, serverPort);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });
}

void PortManager::handleInterfaces() noexcept
{
    // check if there are new interfaces that must get an initial offer information
    vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> interfacePortsForInitialForwarding;


    auto& interfacePorts = m_portPool->getInterfacePortDataList();
    auto port = interfacePorts.begin();
    while (port != interfacePorts.end())
    {
        auto currentPort = port++;
        if (currentPort->m_doInitialOfferForward)
        {
            interfacePortsForInitialForwarding.push_back(currentPort.to_ptr());
            currentPort->m_doInitialOfferForward = false;
        }

        // check if we have to destroy this interface port
        if (currentPort->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            IOX_LOG(DEBUG,
                    "Destroy interface port from runtime '" << currentPort->m_runtimeName
                                                            << "' and with service description '"
                                                            << currentPort->m_serviceDescription << "'");
            m_portPool->removeInterfacePort(currentPort.to_ptr());
        }
    }

    if (interfacePortsForInitialForwarding.size() > 0)
    {
        // provide offer information from all active publisher ports to all new interfaces
        capro::CaproMessage caproMessage;
        caproMessage.m_type = capro::CaproMessageType::OFFER;
        caproMessage.m_serviceType = capro::CaproServiceType::PUBLISHER;
        for (auto& publisherPortData : m_portPool->getPublisherPortDataList())
        {
            PublisherPortUserType publisherPort(&publisherPortData);
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
        for (auto& serverPortData : m_portPool->getServerPortDataList())
        {
            popo::ServerPortUser serverPort(serverPortData);
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

void PortManager::handleConditionVariables() noexcept
{
    auto& condVars = m_portPool->getConditionVariableDataList();
    auto condVar = condVars.begin();
    while (condVar != condVars.end())
    {
        auto currentCondVar = condVar++;
        if (currentCondVar->m_toBeDestroyed.load(std::memory_order_relaxed))
        {
            IOX_LOG(DEBUG, "Destroy ConditionVariableData from runtime '" << currentCondVar->m_runtimeName << "'");
            m_portPool->removeConditionVariableData(currentCondVar.to_ptr());
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
    for (auto& publisherPortData : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(&publisherPortData);

        auto messageInterface = message.m_serviceDescription.getSourceInterface();
        auto publisherInterface = publisherPort.getCaProServiceDescription().getSourceInterface();

        // internal publisher receive all messages all other publishers receive only messages if
        // they do not have the same interface otherwise we have cyclic connections in gateways
        if (publisherInterface != capro::Interfaces::INTERNAL && publisherInterface == messageInterface)
        {
            // iox-#1908
            continue;
        }

        if (isCompatiblePubSub(publisherPort, subscriberSource))
        {
            auto publisherResponse = publisherPort.dispatchCaProMessageAndGetPossibleResponse(message);
            if (publisherResponse.has_value())
            {
                // send response to subscriber port
                subscriberSource.dispatchCaProMessageAndGetPossibleResponse(publisherResponse.value())
                    .and_then([](auto& response) {
                        IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                        IOX_PANIC("Expected no response on ACK or NACK messages");
                    });

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
    for (auto& subscriberPortData : m_portPool->getSubscriberPortDataList())
    {
        SubscriberPortType subscriberPort(&subscriberPortData);

        auto messageInterface = message.m_serviceDescription.getSourceInterface();
        auto subscriberInterface = subscriberPort.getCaProServiceDescription().getSourceInterface();

        // internal subscriber receive all messages all other subscribers receive only messages if
        // they do not have the same interface otherwise we have cyclic connections in gateways
        if (subscriberInterface != capro::Interfaces::INTERNAL && subscriberInterface == messageInterface)
        {
            // iox-#1908
            continue;
        }

        if (isCompatiblePubSub(publisherSource, subscriberPort))
        {
            auto subscriberResponse = subscriberPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the subscribers react on the change, process it immediately on publisher side
            if (subscriberResponse.has_value())
            {
                // we only expect reaction on OFFER
                IOX_ENFORCE(capro::CaproMessageType::OFFER == message.m_type, "Received wrong 'CaproMessageType'!");

                // inform introspection
                m_portIntrospection.reportMessage(subscriberResponse.value());

                auto publisherResponse =
                    publisherSource.dispatchCaProMessageAndGetPossibleResponse(subscriberResponse.value());
                if (publisherResponse.has_value())
                {
                    // sende responsee to subscriber port
                    subscriberPort.dispatchCaProMessageAndGetPossibleResponse(publisherResponse.value())
                        .and_then([](auto& response) {
                            IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                            IOX_PANIC("Expected no response on ACK or NACK messages");
                        });

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
    for (auto& clientPortData : m_portPool->getClientPortDataList())
    {
        popo::ClientPortRouDi clientPort(clientPortData);
        if (isCompatibleClientServer(serverSource, clientPort))
        {
            // send OFFER/STOP_OFFER to client
            auto clientResponse = clientPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the clients react on the change, process it immediately on server side
            if (clientResponse.has_value())
            {
                // we only expect reaction on CONNECT
                IOX_ENFORCE(capro::CaproMessageType::CONNECT == clientResponse.value().m_type,
                            "Received wrong 'CaproMessageType'!");

                /// @todo iox-#518 inform port introspection about client

                // send CONNECT to server
                auto serverResponse = serverSource.dispatchCaProMessageAndGetPossibleResponse(clientResponse.value());
                if (serverResponse.has_value())
                {
                    // send response to client port
                    clientPort.dispatchCaProMessageAndGetPossibleResponse(serverResponse.value())
                        .and_then([](auto& response) {
                            IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                            IOX_PANIC("Expected no response on ACK or NACK messages");
                        });

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
    for (auto& serverPortData : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi serverPort(serverPortData);
        if (isCompatibleClientServer(serverPort, clientSource))
        {
            // send CONNECT/DISCONNECT to server
            auto serverResponse = serverPort.dispatchCaProMessageAndGetPossibleResponse(message);

            // if the server react on the change, process it immediately on client side
            if (serverResponse.has_value())
            {
                // send response to client port
                clientSource.dispatchCaProMessageAndGetPossibleResponse(serverResponse.value())
                    .and_then([](auto& response) {
                        IOX_LOG(FATAL, "Got response '" << response.m_type << "'");
                        IOX_PANIC("Expected no response on ACK or NACK messages");
                    });

                /// @todo iox-#1128 inform port introspection about client
            }
            serverFound = true;
        }
    }
    return serverFound;
}

void PortManager::sendToAllMatchingInterfacePorts(const capro::CaproMessage& message) noexcept
{
    for (auto& interfacePortData : m_portPool->getInterfacePortDataList())
    {
        iox::popo::InterfacePort interfacePort(&interfacePortData);
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
    for (auto& port : m_portPool->getPublisherPortDataList())
    {
        PublisherPortRouDiType publisherPort(&port);
        if (runtimeName == publisherPort.getRuntimeName())
        {
            port.m_offeringRequested.store(false, std::memory_order_relaxed);
            doDiscoveryForPublisherPort(publisherPort);
        }
    }

    for (auto& port : m_portPool->getServerPortDataList())
    {
        popo::ServerPortRouDi serverPort(port);
        if (runtimeName == serverPort.getRuntimeName())
        {
            port.m_offeringRequested.store(false, std::memory_order_relaxed);
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
    for (auto& port : m_portPool->getPublisherPortDataList())
    {
        port.m_offeringRequested.store(false, std::memory_order_relaxed);

        PublisherPortRouDiType publisherPort(&port);
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
    auto& publisherPorts = m_portPool->getPublisherPortDataList();
    auto publisherPort = publisherPorts.begin();
    while (publisherPort != publisherPorts.end())
    {
        auto currentPort = publisherPort++;
        PublisherPortRouDiType sender(currentPort.to_ptr());
        if (runtimeName == sender.getRuntimeName())
        {
            destroyPublisherPort(currentPort.to_ptr());
        }
    }

    auto& subscriberPorts = m_portPool->getSubscriberPortDataList();
    auto subscriberPort = subscriberPorts.begin();
    while (subscriberPort != subscriberPorts.end())
    {
        auto currentPort = subscriberPort++;
        SubscriberPortUserType subscriber(currentPort.to_ptr());
        if (runtimeName == subscriber.getRuntimeName())
        {
            destroySubscriberPort(currentPort.to_ptr());
        }
    }

    auto& serverPorts = m_portPool->getServerPortDataList();
    auto serverPort = serverPorts.begin();
    while (serverPort != serverPorts.end())
    {
        auto currentPort = serverPort++;
        popo::ServerPortRouDi server(*currentPort);
        if (runtimeName == server.getRuntimeName())
        {
            destroyServerPort(currentPort.to_ptr());
        }
    }

    auto& clientPorts = m_portPool->getClientPortDataList();
    auto clientPort = clientPorts.begin();
    while (clientPort != clientPorts.end())
    {
        auto currentPort = clientPort++;
        popo::ClientPortRouDi client(*currentPort);
        if (runtimeName == client.getRuntimeName())
        {
            destroyClientPort(currentPort.to_ptr());
        }
    }

    auto& interfacePorts = m_portPool->getInterfacePortDataList();
    auto interfacePort = interfacePorts.begin();
    while (interfacePort != interfacePorts.end())
    {
        auto currentPort = interfacePort++;
        popo::InterfacePort interface(currentPort.to_ptr());
        if (runtimeName == interface.getRuntimeName())
        {
            IOX_LOG(DEBUG, "Deleted Interface of application " << runtimeName);
            m_portPool->removeInterfacePort(currentPort.to_ptr());
        }
    }

    auto& condVars = m_portPool->getConditionVariableDataList();
    auto condVar = condVars.begin();
    while (condVar != condVars.end())
    {
        auto currentCondVar = condVar++;
        if (runtimeName == currentCondVar->m_runtimeName)
        {
            IOX_LOG(DEBUG, "Deleted condition variable of application" << runtimeName);
            m_portPool->removeConditionVariableData(currentCondVar.to_ptr());
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
        IOX_ENFORCE(caproMessage.m_type == capro::CaproMessageType::STOP_OFFER, "Received wrong 'CaproMessageType'!");

        m_portIntrospection.reportMessage(caproMessage);
        this->removePublisherFromServiceRegistry(caproMessage.m_serviceDescription);
        this->sendToAllMatchingSubscriberPorts(caproMessage, publisherPortRoudi);
        this->sendToAllMatchingInterfacePorts(caproMessage);
    });

    publisherPortRoudi.releaseAllChunks();

    m_portIntrospection.removePublisher(publisherPortUser);

    IOX_LOG(DEBUG,
            "Destroy publisher port from runtime '" << publisherPortData->m_runtimeName
                                                    << "' and with service description '"
                                                    << publisherPortData->m_serviceDescription << "'");
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
        IOX_ENFORCE(caproMessage.m_type == capro::CaproMessageType::UNSUB, "Received wrong 'CaproMessageType'!");

        m_portIntrospection.reportMessage(caproMessage);
        this->sendToAllMatchingPublisherPorts(caproMessage, subscriberPortRoudi);
    });

    subscriberPortRoudi.releaseAllChunks();

    m_portIntrospection.removeSubscriber(subscriberPortUser);

    IOX_LOG(DEBUG,
            "Destroy subscriber port from runtime '" << subscriberPortData->m_runtimeName
                                                     << "' and with service description '"
                                                     << subscriberPortData->m_serviceDescription << "'");
    // delete subscriber port from list after UNSUB was processed
    m_portPool->removeSubscriberPort(subscriberPortData);
}

expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
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

expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortManager::acquirePublisherPortDataWithoutDiscovery(const capro::ServiceDescription& service,
                                                      const popo::PublisherOptions& publisherOptions,
                                                      const RuntimeName_t& runtimeName,
                                                      mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                                      const PortConfigInfo& portConfigInfo) noexcept
{
    if (doesViolateCommunicationPolicy<iox::build::CommunicationPolicy>(service).and_then([&](const auto&
                                                                                                  usedByProcess) {
            IOX_LOG(
                WARN,
                "Process '"
                    << runtimeName
                    << "' violates the communication policy by requesting a PublisherPort which is already used by '"
                    << usedByProcess << "' with service '" << service.operator Serialization().toString() << "'.");
        }))
    {
        IOX_REPORT(PoshError::POSH__PORT_MANAGER_PUBLISHERPORT_NOT_UNIQUE, iox::er::RUNTIME_ERROR);
        return err(PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS);
    }

    if (runtimeName == RuntimeName_t{IPC_CHANNEL_ROUDI_NAME})
    {
        m_internalServices.push_back(service);
    }
    else if (isInternal(service))
    {
        IOX_REPORT(PoshError::POSH__PORT_MANAGER_INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN, iox::er::RUNTIME_ERROR);
        return err(PortPoolError::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN);
    }

    // we can create a new port
    auto maybePublisherPortData = m_portPool->addPublisherPort(
        service, payloadDataSegmentMemoryManager, runtimeName, publisherOptions, portConfigInfo.memoryInfo);

    if (maybePublisherPortData.has_value())
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
            IOX_LOG(FATAL, "Could not create PublisherPort for internal service " << service);
            IOX_REPORT_FATAL(PoshError::PORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTERNAL_SERVICE);
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
            IOX_LOG(FATAL, "Could not create PublisherPort for internal service " << service);
            IOX_REPORT_FATAL(PoshError::PORT_MANAGER__NO_PUBLISHER_PORT_FOR_INTERNAL_SERVICE);
        })
        .value();
}

expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortManager::acquireSubscriberPortData(const capro::ServiceDescription& service,
                                       const popo::SubscriberOptions& subscriberOptions,
                                       const RuntimeName_t& runtimeName,
                                       const PortConfigInfo& portConfigInfo) noexcept
{
    auto maybeSubscriberPortData =
        m_portPool->addSubscriberPort(service, runtimeName, subscriberOptions, portConfigInfo.memoryInfo);
    if (maybeSubscriberPortData.has_value())
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

expected<popo::ClientPortData*, PortPoolError>
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

expected<popo::ServerPortData*, PortPoolError>
PortManager::acquireServerPortData(const capro::ServiceDescription& service,
                                   const popo::ServerOptions& serverOptions,
                                   const RuntimeName_t& runtimeName,
                                   mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                   const PortConfigInfo& portConfigInfo) noexcept
{
    // it is not allowed to have two servers with the same ServiceDescription;
    // check if the server is already in the list
    auto& serverPorts = m_portPool->getServerPortDataList();
    auto port = serverPorts.begin();
    while (port != serverPorts.end())
    {
        auto currentPort = port++;

        if (service == currentPort->m_serviceDescription)
        {
            if (currentPort->m_toBeDestroyed)
            {
                destroyServerPort(currentPort.to_ptr());
                continue;
            }
            IOX_LOG(WARN,
                    "Process '"
                        << runtimeName
                        << "' violates the communication policy by requesting a ServerPort which is already used by '"
                        << currentPort->m_runtimeName << "' with service '"
                        << service.operator Serialization().toString() << "'.");
            IOX_REPORT(PoshError::POSH__PORT_MANAGER_SERVERPORT_NOT_UNIQUE, iox::er::RUNTIME_ERROR);
            return err(PortPoolError::UNIQUE_SERVER_PORT_ALREADY_EXISTS);
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

/// @todo iox-#518 return a expected
popo::InterfacePortData* PortManager::acquireInterfacePortData(capro::Interfaces interface,
                                                               const RuntimeName_t& runtimeName) noexcept
{
    auto result = m_portPool->addInterfacePort(runtimeName, interface);
    if (result.has_value())
    {
        return result.value();
    }
    else
    {
        return nullptr;
    }
}

void PortManager::publishServiceRegistry() noexcept
{
    if (!m_serviceRegistry.hasDataChangedSinceLastCall())
    {
        return;
    }

    if (!m_serviceRegistryPublisherPortData.has_value())
    {
        // should not happen (except during RouDi shutdown)
        // the port always exists, otherwise we would terminate during startup
        IOX_LOG(WARN, "Could not publish service registry!");
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
        .or_else([](auto&) { IOX_LOG(WARN, "Could not allocate a chunk for the service registry!"); });
}

const ServiceRegistry& PortManager::serviceRegistry() const noexcept
{
    return m_serviceRegistry;
}

void PortManager::addPublisherToServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.addPublisher(service).or_else([&](auto&) {
        IOX_LOG(WARN, "Could not add publisher with service description '" << service << "' to service registry!");
        IOX_REPORT(PoshError::POSH__PORT_MANAGER_COULD_NOT_ADD_SERVICE_TO_REGISTRY, iox::er::RUNTIME_ERROR);
    });
}

void PortManager::removePublisherFromServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.removePublisher(service);
}

void PortManager::addServerToServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.addServer(service).or_else([&](auto&) {
        IOX_LOG(WARN, "Could not add server with service description '" << service << "' to service registry!");
        IOX_REPORT(PoshError::POSH__PORT_MANAGER_COULD_NOT_ADD_SERVICE_TO_REGISTRY, iox::er::RUNTIME_ERROR);
    });
}

void PortManager::removeServerFromServiceRegistry(const capro::ServiceDescription& service) noexcept
{
    m_serviceRegistry.removeServer(service);
}

expected<popo::ConditionVariableData*, PortPoolError>
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
