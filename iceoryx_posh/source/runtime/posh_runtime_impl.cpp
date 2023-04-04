// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"

#include "iceoryx_dust/cxx/convert.hpp"
#include "iox/variant.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iox/logging.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
PoshRuntimeImpl::PoshRuntimeImpl(optional<const RuntimeName_t*> name, const RuntimeLocation location) noexcept
    : PoshRuntime(name)
    , m_ipcChannelInterface(roudi::IPC_CHANNEL_ROUDI_NAME, *name.value(), runtime::PROCESS_WAITING_FOR_ROUDI_TIMEOUT)
    , m_ShmInterface([&] {
        // in case the runtime is located in the same process like RouDi the shm is already opened;
        // also in case of the RouDiEnvironment this would close the shm on destruction of the runstime which is also
        // not desired
        return location == RuntimeLocation::SAME_PROCESS_LIKE_ROUDI
                   ? nullopt
                   : optional<SharedMemoryUser>({m_ipcChannelInterface.getShmTopicSize(),
                                                 m_ipcChannelInterface.getSegmentId(),
                                                 m_ipcChannelInterface.getSegmentManagerAddressOffset()});
    }())
{
}

PoshRuntimeImpl::~PoshRuntimeImpl() noexcept
{
    // Inform RouDi that we're shutting down
    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::TERMINATION) << m_appName;
    IpcMessage receiveBuffer;

    if (m_ipcChannelInterface.sendRequestToRouDi(sendBuffer, receiveBuffer)
        && (1U == receiveBuffer.getNumberOfElements()))
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::TERMINATION_ACK)
        {
            IOX_LOG(TRACE) << "RouDi cleaned up resources of " << m_appName << ". Shutting down gracefully.";
        }
        else
        {
            IOX_LOG(ERROR) << "Got wrong response from IPC channel for IpcMessageType::TERMINATION:'"
                           << receiveBuffer.getMessage() << "'";
        }
    }
    else
    {
        IOX_LOG(ERROR) << "Sending IpcMessageType::TERMINATION to RouDi failed:'" << receiveBuffer.getMessage() << "'";
    }
}

PublisherPortUserType::MemberType_t*
PoshRuntimeImpl::getMiddlewarePublisher(const capro::ServiceDescription& service,
                                        const popo::PublisherOptions& publisherOptions,
                                        const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_HISTORY_CAPACITY =
        PublisherPortUserType::MemberType_t::ChunkSenderData_t::ChunkDistributorDataProperties_t::MAX_HISTORY_CAPACITY;

    auto options = publisherOptions;
    if (options.historyCapacity > MAX_HISTORY_CAPACITY)
    {
        IOX_LOG(WARN) << "Requested history capacity " << options.historyCapacity
                      << " exceeds the maximum possible one for this publisher"
                      << ", limiting from " << publisherOptions.historyCapacity << " to " << MAX_HISTORY_CAPACITY;
        options.historyCapacity = MAX_HISTORY_CAPACITY;
    }

    if (options.nodeName.empty())
    {
        options.nodeName = m_appName;
    }

    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_PUBLISHER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << publisherOptions.serialize().toString()
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybePublisher = requestPublisherFromRoudi(sendBuffer);
    if (maybePublisher.has_error())
    {
        switch (maybePublisher.get_error())
        {
        case IpcMessageErrorType::NO_UNIQUE_CREATED:
            IOX_LOG(WARN) << "Service '" << service << "' already in use by another process.";
            errorHandler(PoshError::POSH__RUNTIME_PUBLISHER_PORT_NOT_UNIQUE, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN:
            IOX_LOG(WARN) << "Usage of internal service '" << service << "' is forbidden.";
            errorHandler(PoshError::POSH__RUNTIME_SERVICE_DESCRIPTION_FORBIDDEN, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::PUBLISHER_LIST_FULL:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created since we are out of memory for publishers.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_PUBLISHER_LIST_FULL, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_PUBLISHER_INVALID_RESPONSE:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created. Request publisher got invalid response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_PUBLISHER_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_PUBLISHER_WRONG_IPC_MESSAGE_RESPONSE:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created. Request publisher got wrong IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_PUBLISHER_WRONG_IPC_MESSAGE_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_PUBLISHER_NO_WRITABLE_SHM_SEGMENT:
            IOX_LOG(WARN)
                << "Service '" << service
                << "' could not be created. RouDi did not find a writable shared memory segment for the current "
                   "user. Try using another user or adapt RouDi's config.";
            errorHandler(PoshError::POSH__RUNTIME_NO_WRITABLE_SHM_SEGMENT, iox::ErrorLevel::SEVERE);
            break;
        default:
            IOX_LOG(WARN) << "Unknown error occurred while creating service '" << service << "'.";
            errorHandler(PoshError::POSH__RUNTIME_PUBLISHER_PORT_CREATION_UNKNOWN_ERROR, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybePublisher.value();
}

expected<PublisherPortUserType::MemberType_t*, IpcMessageErrorType>
PoshRuntimeImpl::requestPublisherFromRoudi(const IpcMessage& sendBuffer) noexcept
{
    IpcMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request publisher got invalid response!";
        return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_PUBLISHER_INVALID_RESPONSE);
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_PUBLISHER_ACK)

        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return success<PublisherPortUserType::MemberType_t*>(
                reinterpret_cast<PublisherPortUserType::MemberType_t*>(ptr));
        }
    }
    else if (receiveBuffer.getNumberOfElements() == 2U)
    {
        std::string IpcMessage1 = receiveBuffer.getElementAtIndex(0U);
        std::string IpcMessage2 = receiveBuffer.getElementAtIndex(1U);
        if (stringToIpcMessageType(IpcMessage1.c_str()) == IpcMessageType::ERROR)
        {
            IOX_LOG(ERROR) << "Request publisher received no valid publisher port from RouDi.";
            return error<IpcMessageErrorType>(stringToIpcMessageErrorType(IpcMessage2.c_str()));
        }
    }

    IOX_LOG(ERROR) << "Request publisher got wrong response from IPC channel :'" << receiveBuffer.getMessage() << "'";
    return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_PUBLISHER_WRONG_IPC_MESSAGE_RESPONSE);
}

SubscriberPortUserType::MemberType_t*
PoshRuntimeImpl::getMiddlewareSubscriber(const capro::ServiceDescription& service,
                                         const popo::SubscriberOptions& subscriberOptions,
                                         const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_QUEUE_CAPACITY = SubscriberPortUserType::MemberType_t::ChunkQueueData_t::MAX_CAPACITY;

    auto options = subscriberOptions;
    if (options.queueCapacity > MAX_QUEUE_CAPACITY)
    {
        IOX_LOG(WARN) << "Requested queue capacity " << options.queueCapacity
                      << " exceeds the maximum possible one for this subscriber"
                      << ", limiting from " << subscriberOptions.queueCapacity << " to " << MAX_QUEUE_CAPACITY;
        options.queueCapacity = MAX_QUEUE_CAPACITY;
    }
    else if (0U == options.queueCapacity)
    {
        IOX_LOG(WARN) << "Requested queue capacity of 0 doesn't make sense as no data would be received,"
                      << " the capacity is set to 1";
        options.queueCapacity = 1U;
    }

    if (subscriberOptions.historyRequest > subscriberOptions.queueCapacity)
    {
        IOX_LOG(WARN) << "Requested historyRequest for " << service
                      << " is larger than queueCapacity. Clamping historyRequest to queueCapacity!";
        options.historyRequest = subscriberOptions.queueCapacity;
    }

    if (options.nodeName.empty())
    {
        options.nodeName = m_appName;
    }

    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_SUBSCRIBER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << options.serialize().toString()
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybeSubscriber = requestSubscriberFromRoudi(sendBuffer);

    if (maybeSubscriber.has_error())
    {
        switch (maybeSubscriber.get_error())
        {
        case IpcMessageErrorType::SUBSCRIBER_LIST_FULL:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created since we are out of memory for subscribers.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_SUBSCRIBER_LIST_FULL, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_SUBSCRIBER_INVALID_RESPONSE:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created. Request subscriber got invalid response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_SUBSCRIBER_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_SUBSCRIBER_WRONG_IPC_MESSAGE_RESPONSE:
            IOX_LOG(WARN) << "Service '" << service
                          << "' could not be created. Request subscriber got wrong IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_SUBSCRIBER_WRONG_IPC_MESSAGE_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        default:
            IOX_LOG(WARN) << "Unknown error occurred while creating service '" << service << "'.";
            errorHandler(PoshError::POSH__RUNTIME_SUBSCRIBER_PORT_CREATION_UNKNOWN_ERROR, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeSubscriber.value();
}

expected<SubscriberPortUserType::MemberType_t*, IpcMessageErrorType>
PoshRuntimeImpl::requestSubscriberFromRoudi(const IpcMessage& sendBuffer) noexcept
{
    IpcMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request subscriber got invalid response!";
        return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_SUBSCRIBER_INVALID_RESPONSE);
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_SUBSCRIBER_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return success<SubscriberPortUserType::MemberType_t*>(
                reinterpret_cast<SubscriberPortUserType::MemberType_t*>(ptr));
        }
    }
    else if (receiveBuffer.getNumberOfElements() == 2U)
    {
        std::string IpcMessage1 = receiveBuffer.getElementAtIndex(0U);
        std::string IpcMessage2 = receiveBuffer.getElementAtIndex(1U);

        if (stringToIpcMessageType(IpcMessage1.c_str()) == IpcMessageType::ERROR)
        {
            IOX_LOG(ERROR) << "Request subscriber received no valid subscriber port from RouDi.";
            return error<IpcMessageErrorType>(stringToIpcMessageErrorType(IpcMessage2.c_str()));
        }
    }

    IOX_LOG(ERROR) << "Request subscriber got wrong response from IPC channel :'" << receiveBuffer.getMessage() << "'";
    return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_SUBSCRIBER_WRONG_IPC_MESSAGE_RESPONSE);
}

popo::ClientPortUser::MemberType_t* PoshRuntimeImpl::getMiddlewareClient(const capro::ServiceDescription& service,
                                                                         const popo::ClientOptions& clientOptions,
                                                                         const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::ClientChunkQueueConfig::MAX_QUEUE_CAPACITY;
    auto options = clientOptions;
    if (options.responseQueueCapacity > MAX_QUEUE_CAPACITY)
    {
        IOX_LOG(WARN) << "Requested response queue capacity " << options.responseQueueCapacity
                      << " exceeds the maximum possible one for this client"
                      << ", limiting from " << options.responseQueueCapacity << " to " << MAX_QUEUE_CAPACITY;
        options.responseQueueCapacity = MAX_QUEUE_CAPACITY;
    }
    else if (options.responseQueueCapacity == 0U)
    {
        IOX_LOG(WARN) << "Requested response queue capacity of 0 doesn't make sense as no data would be received,"
                      << " the capacity is set to 1";
        options.responseQueueCapacity = 1U;
    }

    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_CLIENT) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << options.serialize().toString()
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybeClient = requestClientFromRoudi(sendBuffer);
    if (maybeClient.has_error())
    {
        switch (maybeClient.get_error())
        {
        case IpcMessageErrorType::CLIENT_LIST_FULL:
            IOX_LOG(WARN) << "Could not create client with service description '" << service
                          << "' as we are out of memory for clients.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_OUT_OF_CLIENTS, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_CLIENT_INVALID_RESPONSE:
            IOX_LOG(WARN) << "Could not create client with service description '" << service
                          << "'; received invalid response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_CLIENT_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_CLIENT_WRONG_IPC_MESSAGE_RESPONSE:
            IOX_LOG(WARN) << "Could not create client with service description '" << service
                          << "'; received wrong IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_CLIENT_WRONG_IPC_MESSAGE_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_CLIENT_NO_WRITABLE_SHM_SEGMENT:
            IOX_LOG(WARN)
                << "Service '" << service
                << "' could not be created. RouDi did not find a writable shared memory segment for the current "
                   "user. Try using another user or adapt RouDi's config.";
            errorHandler(PoshError::POSH__RUNTIME_NO_WRITABLE_SHM_SEGMENT, iox::ErrorLevel::SEVERE);
            break;
        default:
            IOX_LOG(WARN) << "Unknown error occurred while creating client with service description '" << service
                          << "'";
            errorHandler(PoshError::POSH__RUNTIME_CLIENT_PORT_CREATION_UNKNOWN_ERROR, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeClient.value();
}

expected<popo::ClientPortUser::MemberType_t*, IpcMessageErrorType>
PoshRuntimeImpl::requestClientFromRoudi(const IpcMessage& sendBuffer) noexcept
{
    IpcMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request client got invalid response!";
        return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_CLIENT_INVALID_RESPONSE);
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_CLIENT_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return success<popo::ClientPortUser::MemberType_t*>(
                reinterpret_cast<popo::ClientPortUser::MemberType_t*>(ptr));
        }
    }
    else if (receiveBuffer.getNumberOfElements() == 2U)
    {
        std::string IpcMessage1 = receiveBuffer.getElementAtIndex(0U);
        std::string IpcMessage2 = receiveBuffer.getElementAtIndex(1U);
        if (stringToIpcMessageType(IpcMessage1.c_str()) == IpcMessageType::ERROR)
        {
            IOX_LOG(ERROR) << "Request client received no valid client port from RouDi.";
            return error<IpcMessageErrorType>(stringToIpcMessageErrorType(IpcMessage2.c_str()));
        }
    }


    IOX_LOG(ERROR) << "Request client got wrong response from IPC channel :'" << receiveBuffer.getMessage() << "'";
    return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_CLIENT_WRONG_IPC_MESSAGE_RESPONSE);
}

popo::ServerPortUser::MemberType_t* PoshRuntimeImpl::getMiddlewareServer(const capro::ServiceDescription& service,
                                                                         const popo::ServerOptions& serverOptions,
                                                                         const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::ServerChunkQueueConfig::MAX_QUEUE_CAPACITY;
    auto options = serverOptions;
    if (options.requestQueueCapacity > MAX_QUEUE_CAPACITY)
    {
        IOX_LOG(WARN) << "Requested request queue capacity " << options.requestQueueCapacity
                      << " exceeds the maximum possible one for this server"
                      << ", limiting from " << options.requestQueueCapacity << " to " << MAX_QUEUE_CAPACITY;
        options.requestQueueCapacity = MAX_QUEUE_CAPACITY;
    }
    else if (options.requestQueueCapacity == 0U)
    {
        IOX_LOG(WARN) << "Requested request queue capacity of 0 doesn't make sense as no data would be received,"
                      << " the capacity is set to 1";
        options.requestQueueCapacity = 1U;
    }

    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_SERVER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << options.serialize().toString()
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybeServer = requestServerFromRoudi(sendBuffer);
    if (maybeServer.has_error())
    {
        switch (maybeServer.get_error())
        {
        case IpcMessageErrorType::SERVER_LIST_FULL:
            IOX_LOG(WARN) << "Could not create server with service description '" << service
                          << "' as we are out of memory for servers.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_OUT_OF_SERVERS, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_SERVER_INVALID_RESPONSE:
            IOX_LOG(WARN) << "Could not create server with service description '" << service
                          << "'; received invalid response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_SERVER_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_SERVER_WRONG_IPC_MESSAGE_RESPONSE:
            IOX_LOG(WARN) << "Could not create server with service description '" << service
                          << "'; received wrong IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_SERVER_WRONG_IPC_MESSAGE_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_SERVER_NO_WRITABLE_SHM_SEGMENT:
            IOX_LOG(WARN)
                << "Service '" << service
                << "' could not be created. RouDi did not find a writable shared memory segment for the current "
                   "user. Try using another user or adapt RouDi's config.";
            errorHandler(PoshError::POSH__RUNTIME_NO_WRITABLE_SHM_SEGMENT, iox::ErrorLevel::SEVERE);
            break;
        default:
            IOX_LOG(WARN) << "Unknown error occurred while creating server with service description '" << service
                          << "'";
            errorHandler(PoshError::POSH__RUNTIME_SERVER_PORT_CREATION_UNKNOWN_ERROR, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeServer.value();
}

expected<popo::ServerPortUser::MemberType_t*, IpcMessageErrorType>
PoshRuntimeImpl::requestServerFromRoudi(const IpcMessage& sendBuffer) noexcept
{
    IpcMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request server got invalid response!";
        return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_SERVER_INVALID_RESPONSE);
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_SERVER_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return success<popo::ServerPortUser::MemberType_t*>(
                reinterpret_cast<popo::ServerPortUser::MemberType_t*>(ptr));
        }
    }
    else if (receiveBuffer.getNumberOfElements() == 2U)
    {
        std::string IpcMessage1 = receiveBuffer.getElementAtIndex(0U);
        std::string IpcMessage2 = receiveBuffer.getElementAtIndex(1U);
        if (stringToIpcMessageType(IpcMessage1.c_str()) == IpcMessageType::ERROR)
        {
            IOX_LOG(ERROR) << "Request server received no valid server port from RouDi.";
            return error<IpcMessageErrorType>(stringToIpcMessageErrorType(IpcMessage2.c_str()));
        }
    }

    IOX_LOG(ERROR) << "Request server got wrong response from IPC channel :'" << receiveBuffer.getMessage() << "'";
    return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_SERVER_WRONG_IPC_MESSAGE_RESPONSE);
}

popo::InterfacePortData* PoshRuntimeImpl::getMiddlewareInterface(const capro::Interfaces interface,
                                                                 const NodeName_t& nodeName) noexcept
{
    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_INTERFACE) << m_appName
               << static_cast<uint32_t>(interface) << nodeName;

    IpcMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request interface got invalid response!";
        errorHandler(PoshError::POSH__RUNTIME_ROUDI_GET_MW_INTERFACE_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_INTERFACE_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return reinterpret_cast<popo::InterfacePortData*>(ptr);
        }
    }

    IOX_LOG(ERROR) << "Get mw interface got wrong response from IPC channel :'" << receiveBuffer.getMessage() << "'";
    errorHandler(PoshError::POSH__RUNTIME_ROUDI_GET_MW_INTERFACE_WRONG_IPC_MESSAGE_RESPONSE, iox::ErrorLevel::SEVERE);
    return nullptr;
}

NodeData* PoshRuntimeImpl::createNode(const NodeProperty& nodeProperty) noexcept
{
    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_NODE) << m_appName
               << static_cast<cxx::Serialization>(nodeProperty).toString();

    IpcMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request node got invalid response!";
        errorHandler(PoshError::POSH__RUNTIME_ROUDI_CREATE_NODE_INVALID_RESPONSE, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_NODE_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return reinterpret_cast<NodeData*>(ptr);
        }
    }

    IOX_LOG(ERROR) << "Got wrong response from RouDi while creating node:'" << receiveBuffer.getMessage() << "'";
    errorHandler(PoshError::POSH__RUNTIME_ROUDI_CREATE_NODE_WRONG_IPC_MESSAGE_RESPONSE, iox::ErrorLevel::SEVERE);
    return nullptr;
}

expected<popo::ConditionVariableData*, IpcMessageErrorType>
PoshRuntimeImpl::requestConditionVariableFromRoudi(const IpcMessage& sendBuffer) noexcept
{
    IpcMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) == false)
    {
        IOX_LOG(ERROR) << "Request condition variable got invalid response!";
        return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_CONDITION_VARIABLE_INVALID_RESPONSE);
    }
    else if (receiveBuffer.getNumberOfElements() == 3U)
    {
        std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::CREATE_CONDITION_VARIABLE_ACK)
        {
            segment_id_underlying_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            UntypedRelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = UntypedRelativePointer::getPtr(segment_id_t{segmentId}, offset);
            return success<popo::ConditionVariableData*>(reinterpret_cast<popo::ConditionVariableData*>(ptr));
        }
    }
    else if (receiveBuffer.getNumberOfElements() == 2U)
    {
        std::string IpcMessage1 = receiveBuffer.getElementAtIndex(0U);
        std::string IpcMessage2 = receiveBuffer.getElementAtIndex(1U);
        if (stringToIpcMessageType(IpcMessage1.c_str()) == IpcMessageType::ERROR)
        {
            IOX_LOG(ERROR) << "Request condition variable received no valid condition variable port from RouDi.";
            return error<IpcMessageErrorType>(stringToIpcMessageErrorType(IpcMessage2.c_str()));
        }
    }

    IOX_LOG(ERROR) << "Request condition variable got wrong response from IPC channel :'" << receiveBuffer.getMessage()
                   << "'";
    return error<IpcMessageErrorType>(IpcMessageErrorType::REQUEST_CONDITION_VARIABLE_WRONG_IPC_MESSAGE_RESPONSE);
}

popo::ConditionVariableData* PoshRuntimeImpl::getMiddlewareConditionVariable() noexcept
{
    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_CONDITION_VARIABLE) << m_appName;

    auto maybeConditionVariable = requestConditionVariableFromRoudi(sendBuffer);
    if (maybeConditionVariable.has_error())
    {
        switch (maybeConditionVariable.get_error())
        {
        case IpcMessageErrorType::CONDITION_VARIABLE_LIST_FULL:
            IOX_LOG(WARN) << "Could not create condition variable as we are out of memory for condition variables.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_CONDITION_VARIABLE_LIST_FULL, iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_CONDITION_VARIABLE_INVALID_RESPONSE:
            IOX_LOG(WARN) << "Could not create condition variables; received invalid IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_CONDITION_VARIABLE_INVALID_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        case IpcMessageErrorType::REQUEST_CONDITION_VARIABLE_WRONG_IPC_MESSAGE_RESPONSE:
            IOX_LOG(WARN) << "Could not create condition variables; received wrong IPC channel response.";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_REQUEST_CONDITION_VARIABLE_WRONG_IPC_MESSAGE_RESPONSE,
                         iox::ErrorLevel::SEVERE);
            break;
        default:
            IOX_LOG(WARN) << "Unknown error occurred while creating condition variable";
            errorHandler(PoshError::POSH__RUNTIME_ROUDI_CONDITION_VARIABLE_CREATION_UNKNOWN_ERROR,
                         iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeConditionVariable.value();
}

bool PoshRuntimeImpl::sendRequestToRouDi(const IpcMessage& msg, IpcMessage& answer) noexcept
{
    // runtime must be thread safe
    std::lock_guard<posix::mutex> g(m_appIpcRequestMutex);
    return m_ipcChannelInterface.sendRequestToRouDi(msg, answer);
}

// this is the callback for the m_keepAliveTimer
void PoshRuntimeImpl::sendKeepAliveAndHandleShutdownPreparation() noexcept
{
    if (!m_ipcChannelInterface.sendKeepalive())
    {
        IOX_LOG(WARN) << "Error in sending keep alive";
    }

    // this is not the nicest solution, but we cannot send this in the signal handler where m_shutdownRequested is
    // usually set; luckily the runtime already has a thread running and therefore this thread is used to unblock the
    // application shutdown from a potentially blocking publisher with the
    // ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER option set
    if (m_shutdownRequested.exchange(false, std::memory_order_relaxed))
    {
        // Inform RouDi to prepare for app shutdown
        IpcMessage sendBuffer;
        sendBuffer << IpcMessageTypeToString(IpcMessageType::PREPARE_APP_TERMINATION) << m_appName;
        IpcMessage receiveBuffer;

        if (m_ipcChannelInterface.sendRequestToRouDi(sendBuffer, receiveBuffer)
            && (1U == receiveBuffer.getNumberOfElements()))
        {
            std::string IpcMessage = receiveBuffer.getElementAtIndex(0U);

            if (stringToIpcMessageType(IpcMessage.c_str()) == IpcMessageType::PREPARE_APP_TERMINATION_ACK)
            {
                IOX_LOG(TRACE) << "RouDi unblocked shutdown of " << m_appName << ".";
            }
            else
            {
                IOX_LOG(ERROR) << "Got wrong response from IPC channel for PREPARE_APP_TERMINATION:'"
                               << receiveBuffer.getMessage() << "'";
            }
        }
        else
        {
            IOX_LOG(ERROR) << "Sending IpcMessageType::PREPARE_APP_TERMINATION to RouDi failed:'"
                           << receiveBuffer.getMessage() << "'";
        }
    }
}

} // namespace runtime
} // namespace iox
