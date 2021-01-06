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

#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_posh/runtime/node.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
PoshRuntime::factory_t& PoshRuntime::getRuntimeFactory() noexcept
{
    static factory_t runtimeFactory = PoshRuntime::defaultRuntimeFactory;
    return runtimeFactory;
}

void PoshRuntime::setRuntimeFactory(const factory_t& factory) noexcept
{
    if (factory)
    {
        PoshRuntime::getRuntimeFactory() = factory;
    }
    else
    {
        LogFatal() << "Cannot set runtime factory. Passed factory must not be empty!";
        errorHandler(Error::kPOSH__RUNTIME_FACTORY_IS_NOT_SET);
    }
}

PoshRuntime& PoshRuntime::defaultRuntimeFactory(cxx::optional<const ProcessName_t*> name) noexcept
{
    static PoshRuntime instance(name);
    return instance;
}

// singleton access
PoshRuntime& PoshRuntime::getInstance() noexcept
{
    return getInstance(cxx::nullopt);
}

PoshRuntime& PoshRuntime::initRuntime(const ProcessName_t& name) noexcept
{
    return getInstance(cxx::make_optional<const ProcessName_t*>(&name));
}

PoshRuntime& PoshRuntime::getInstance(cxx::optional<const ProcessName_t*> name) noexcept
{
    return getRuntimeFactory()(name);
}

PoshRuntime::PoshRuntime(cxx::optional<const ProcessName_t*> name, const bool doMapSharedMemoryIntoThread) noexcept
    : m_appName(verifyInstanceName(name))
    , m_MqInterface(roudi::MQ_ROUDI_NAME, *name.value(), runtime::PROCESS_WAITING_FOR_ROUDI_TIMEOUT)
    , m_ShmInterface(doMapSharedMemoryIntoThread,
                     m_MqInterface.getShmTopicSize(),
                     m_MqInterface.getSegmentId(),
                     m_MqInterface.getSegmentManagerAddressOffset())
    , m_applicationPort(getMiddlewareApplication())
{
    /// @todo here we could get the LogLevel and LogMode and set it on the LogManager
}

PoshRuntime::~PoshRuntime() noexcept
{
    if (m_applicationPort)
    {
        m_applicationPort.destroy();
    }
}


const ProcessName_t& PoshRuntime::verifyInstanceName(cxx::optional<const ProcessName_t*> name) noexcept
{
    if (!name.has_value())
    {
        LogError() << "Cannot initialize runtime. Application name has not been specified!";
        std::terminate();
    }
    else if (name.value()->empty())
    {
        LogError() << "Cannot initialize runtime. Application name must not be empty!";
        std::terminate();
    }
    else if (name.value()->c_str()[0] == '/')
    {
        LogError() << "Cannot initialize runtime. Please remove leading slash from Application name " << *name.value();
        std::terminate();
    }

    return *name.value();
}

ProcessName_t PoshRuntime::getInstanceName() const noexcept
{
    return m_appName;
}

const std::atomic<uint64_t>* PoshRuntime::getServiceRegistryChangeCounter() noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::SERVICE_REGISTRY_CHANGE_COUNTER) << m_appName;
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (2U == receiveBuffer.getNumberOfElements()))
    {
        RelativePointer::offset_t offset{0U};
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(0U).c_str(), offset);
        RelativePointer::id_t segmentId{0U};
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), segmentId);
        auto ptr = RelativePointer::getPtr(segmentId, offset);

        return reinterpret_cast<std::atomic<uint64_t>*>(ptr);
    }
    else
    {
        LogError() << "unable to request service registry change counter caused by wrong response from RouDi: \""
                   << receiveBuffer.getMessage() << "\" with request: \"" << sendBuffer.getMessage() << "\"";
        return nullptr;
    }
}

PublisherPortUserType::MemberType_t* PoshRuntime::getMiddlewarePublisher(const capro::ServiceDescription& service,
                                                                         const popo::PublisherOptions& publisherOptions,
                                                                         const NodeName_t& nodeName,
                                                                         const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_HISTORY_CAPACITY =
        PublisherPortUserType::MemberType_t::ChunkSenderData_t::ChunkDistributorDataProperties_t::MAX_HISTORY_CAPACITY;

    auto options = publisherOptions;
    if (options.historyCapacity > MAX_HISTORY_CAPACITY)
    {
        LogWarn() << "Requested history capacity " << options.historyCapacity
                  << " exceeds the maximum possible one for this publisher"
                  << ", limiting from " << publisherOptions.historyCapacity << " to " << MAX_HISTORY_CAPACITY;
        options.historyCapacity = MAX_HISTORY_CAPACITY;
    }
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_PUBLISHER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << std::to_string(options.historyCapacity)
               << nodeName << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybePublisher = requestPublisherFromRoudi(sendBuffer);
    if (maybePublisher.has_error())
    {
        switch (maybePublisher.get_error())
        {
        case MqMessageErrorType::NO_UNIQUE_CREATED:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' already in use by another process.";
            errorHandler(Error::kPOSH__RUNTIME_PUBLISHER_PORT_NOT_UNIQUE, nullptr, iox::ErrorLevel::SEVERE);
            break;
        case MqMessageErrorType::PUBLISHER_LIST_FULL:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' could not be created since we are out of memory for publishers.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_PUBLISHER_LIST_FULL, nullptr, iox::ErrorLevel::SEVERE);
            break;
        case MqMessageErrorType::REQUEST_PUBLISHER_WRONG_MESSAGE_QUEUE_RESPONSE:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' could not be created. Request publisher got wrong message queue response.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_REQUEST_PUBLISHER_WRONG_MESSAGE_QUEUE_RESPONSE,
                         nullptr,
                         iox::ErrorLevel::SEVERE);
            break;
        default:
            LogWarn() << "Undefined behavior occurred while creating service '"
                      << service.operator cxx::Serialization().toString() << "'.";
            errorHandler(
                Error::kPOSH__RUNTIME_PUBLISHER_PORT_CREATION_UNDEFINED_BEHAVIOR, nullptr, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybePublisher.value();
}

cxx::expected<PublisherPortUserType::MemberType_t*, MqMessageErrorType>
PoshRuntime::requestPublisherFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_PUBLISHER_ACK)

        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return cxx::success<PublisherPortUserType::MemberType_t*>(
                reinterpret_cast<PublisherPortUserType::MemberType_t*>(ptr));
        }
    }
    else
    {
        if (receiveBuffer.getNumberOfElements() == 2U)
        {
            std::string mqMessage1 = receiveBuffer.getElementAtIndex(0U);
            std::string mqMessage2 = receiveBuffer.getElementAtIndex(1U);
            if (stringToMqMessageType(mqMessage1.c_str()) == MqMessageType::ERROR)
            {
                LogError() << "Request publisher received no valid publisher port from RouDi.";
                return cxx::error<MqMessageErrorType>(stringToMqMessageErrorType(mqMessage2.c_str()));
            }
        }
    }

    LogError() << "Request publisher got wrong response from message queue :'" << receiveBuffer.getMessage() << "'";
    return cxx::error<MqMessageErrorType>(MqMessageErrorType::REQUEST_PUBLISHER_WRONG_MESSAGE_QUEUE_RESPONSE);
}

SubscriberPortUserType::MemberType_t*
PoshRuntime::getMiddlewareSubscriber(const capro::ServiceDescription& service,
                                     const popo::SubscriberOptions& subscriberOptions,
                                     const NodeName_t& nodeName,
                                     const PortConfigInfo& portConfigInfo) noexcept
{
    constexpr uint64_t MAX_QUEUE_CAPACITY = SubscriberPortUserType::MemberType_t::ChunkQueueData_t::MAX_CAPACITY;

    auto options = subscriberOptions;
    if (options.queueCapacity > MAX_QUEUE_CAPACITY)
    {
        LogWarn() << "Requested queue capacity " << options.queueCapacity
                  << " exceeds the maximum possible one for this subscriber"
                  << ", limiting from " << subscriberOptions.queueCapacity << " to " << MAX_QUEUE_CAPACITY;
        options.queueCapacity = MAX_QUEUE_CAPACITY;
    }

    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_SUBSCRIBER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << std::to_string(options.historyRequest)
               << std::to_string(options.queueCapacity) << nodeName
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto maybeSubscriber = requestSubscriberFromRoudi(sendBuffer);

    if (maybeSubscriber.has_error())
    {
        switch (maybeSubscriber.get_error())
        {
        case MqMessageErrorType::SUBSCRIBER_LIST_FULL:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' could not be created since we are out of memory for subscribers.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_SUBSCRIBER_LIST_FULL, nullptr, iox::ErrorLevel::SEVERE);
            break;
        case MqMessageErrorType::REQUEST_SUBSCRIBER_WRONG_MESSAGE_QUEUE_RESPONSE:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' could not be created. Request subscriber got wrong message queue response.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_REQUEST_SUBSCRIBER_WRONG_MESSAGE_QUEUE_RESPONSE,
                         nullptr,
                         iox::ErrorLevel::SEVERE);
            break;
        default:
            LogWarn() << "Undefined behavior occurred while creating service '"
                      << service.operator cxx::Serialization().toString() << "'.";
            errorHandler(
                Error::kPOSH__RUNTIME_SUBSCRIBER_PORT_CREATION_UNDEFINED_BEHAVIOR, nullptr, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeSubscriber.value();
}

cxx::expected<SubscriberPortUserType::MemberType_t*, MqMessageErrorType>
PoshRuntime::requestSubscriberFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_SUBSCRIBER_ACK)
        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return cxx::success<SubscriberPortUserType::MemberType_t*>(
                reinterpret_cast<SubscriberPortUserType::MemberType_t*>(ptr));
        }
    }
    else
    {
        if (receiveBuffer.getNumberOfElements() == 2U)
        {
            std::string mqMessage1 = receiveBuffer.getElementAtIndex(0U);
            std::string mqMessage2 = receiveBuffer.getElementAtIndex(1U);

            if (stringToMqMessageType(mqMessage1.c_str()) == MqMessageType::ERROR)
            {
                LogError() << "Request subscriber received no valid subscriber port from RouDi.";
                return cxx::error<MqMessageErrorType>(stringToMqMessageErrorType(mqMessage2.c_str()));
            }
        }
    }

    LogError() << "Request subscriber got wrong response from message queue :'" << receiveBuffer.getMessage() << "'";
    return cxx::error<MqMessageErrorType>(MqMessageErrorType::REQUEST_SUBSCRIBER_WRONG_MESSAGE_QUEUE_RESPONSE);
}

popo::InterfacePortData* PoshRuntime::getMiddlewareInterface(const capro::Interfaces interface,
                                                             const NodeName_t& nodeName) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_INTERFACE) << m_appName
               << static_cast<uint32_t>(interface) << nodeName;

    MqMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_INTERFACE_ACK)
        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<popo::InterfacePortData*>(ptr);
        }
    }

    LogError() << "Get mw interface got wrong response from message queue :'" << receiveBuffer.getMessage() << "'";
    errorHandler(
        Error::kPOSH__RUNTIME_ROUDI_GET_MW_INTERFACE_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
    return nullptr;
}

NodeData* PoshRuntime::createNode(const NodeProperty& nodeProperty) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_NODE) << m_appName
               << static_cast<cxx::Serialization>(nodeProperty).toString();

    MqMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_NODE_ACK)
        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<NodeData*>(ptr);
        }
    }

    LogError() << "Got wrong response from RouDi while creating node:'" << receiveBuffer.getMessage() << "'";
    errorHandler(
        Error::kPOSH__RUNTIME_ROUDI_CREATE_NODE_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
    return nullptr;
}

cxx::expected<InstanceContainer, Error>
PoshRuntime::findService(const capro::ServiceDescription& serviceDescription) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::FIND_SERVICE) << m_appName
               << static_cast<cxx::Serialization>(serviceDescription).toString();

    MqMessage requestResponse;

    if (!sendRequestToRouDi(sendBuffer, requestResponse))
    {
        LogError() << "Could not send FIND_SERVICE request to RouDi\n";
        errorHandler(Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ, nullptr, ErrorLevel::MODERATE);
        return cxx::error<Error>(Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ);
    }

    InstanceContainer instanceContainer;
    uint32_t numberOfElements = requestResponse.getNumberOfElements();
    uint32_t capacity = static_cast<uint32_t>(instanceContainer.capacity());

    // Limit the instances (max value is the capacity of instanceContainer)
    uint32_t numberOfInstances = ((numberOfElements > capacity) ? capacity : numberOfElements);
    for (uint32_t i = 0; i < numberOfInstances; ++i)
    {
        capro::IdString_t instance(iox::cxx::TruncateToCapacity, requestResponse.getElementAtIndex(i).c_str());
        instanceContainer.push_back(instance);
    }

    if (numberOfElements > capacity)
    {
        LogWarn() << numberOfElements << " instances found for service \"" << serviceDescription.getServiceIDString()
                  << "\" which is more than supported number of instances(" << MAX_NUMBER_OF_INSTANCES << "\n";
        errorHandler(Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<Error>(Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW);
    }
    return {cxx::success<InstanceContainer>(instanceContainer)};
}


bool PoshRuntime::offerService(const capro::ServiceDescription& serviceDescription) noexcept
{
    if (serviceDescription.isValid())
    {
        capro::CaproMessage msg(
            capro::CaproMessageType::OFFER, serviceDescription, capro::CaproMessageSubType::SERVICE);
        m_applicationPort.dispatchCaProMessage(msg);
        return true;
    }
    LogWarn() << "Could not offer service " << serviceDescription.getServiceIDString() << ","
              << " ServiceDescription is invalid\n";
    return false;
}

void PoshRuntime::stopOfferService(const capro::ServiceDescription& serviceDescription) noexcept
{
    capro::CaproMessage msg(
        capro::CaproMessageType::STOP_OFFER, serviceDescription, capro::CaproMessageSubType::SERVICE);
    m_applicationPort.dispatchCaProMessage(msg);
}

popo::ApplicationPortData* PoshRuntime::getMiddlewareApplication() noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_APPLICATION) << m_appName;

    MqMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_APPLICATION_ACK)
        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<popo::ApplicationPortData*>(ptr);
        }
    }

    LogError() << "Get mw application got wrong response from message queue :'" << receiveBuffer.getMessage() << "'";
    errorHandler(
        Error::kPOSH__RUNTIME_ROUDI_GET_MW_APPLICATION_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
    return nullptr;
}

cxx::expected<popo::ConditionVariableData*, MqMessageErrorType>
PoshRuntime::requestConditionVariableFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3U == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0U);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_CONDITION_VARIABLE_ACK)
        {
            RelativePointer::id_t segmentId{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2U).c_str(), segmentId);
            RelativePointer::offset_t offset{0U};
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return cxx::success<popo::ConditionVariableData*>(reinterpret_cast<popo::ConditionVariableData*>(ptr));
        }
    }
    else
    {
        if (receiveBuffer.getNumberOfElements() == 2U)
        {
            std::string mqMessage1 = receiveBuffer.getElementAtIndex(0U);
            std::string mqMessage2 = receiveBuffer.getElementAtIndex(1U);
            if (stringToMqMessageType(mqMessage1.c_str()) == MqMessageType::ERROR)
            {
                LogError() << "Request condition variable received no valid condition variable port from RouDi.";
                return cxx::error<MqMessageErrorType>(stringToMqMessageErrorType(mqMessage2.c_str()));
            }
        }
    }

    LogError() << "Request condition variable got wrong response from message queue :'" << receiveBuffer.getMessage()
               << "'";
    return cxx::error<MqMessageErrorType>(MqMessageErrorType::REQUEST_CONDITION_VARIABLE_WRONG_MESSAGE_QUEUE_RESPONSE);
}

popo::ConditionVariableData* PoshRuntime::getMiddlewareConditionVariable() noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_CONDITION_VARIABLE) << m_appName;

    auto maybeConditionVariable = requestConditionVariableFromRoudi(sendBuffer);
    if (maybeConditionVariable.has_error())
    {
        switch (maybeConditionVariable.get_error())
        {
        case MqMessageErrorType::CONDITION_VARIABLE_LIST_FULL:
            LogWarn() << "Could not create condition variable as we are out of memory for condition variables.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_CONDITION_VARIABLE_LIST_FULL, nullptr, iox::ErrorLevel::SEVERE);
            break;
        case MqMessageErrorType::REQUEST_CONDITION_VARIABLE_WRONG_MESSAGE_QUEUE_RESPONSE:
            LogWarn() << "Could not create condition variables; received wrong message queue response.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_REQUEST_CONDITION_VARIABLE_WRONG_MESSAGE_QUEUE_RESPONSE,
                         nullptr,
                         iox::ErrorLevel::SEVERE);
            break;
        default:
            LogWarn() << "Undefined behavior occurred while creating condition variable";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_CONDITION_VARIABLE_CREATION_UNDEFINED_BEHAVIOR,
                         nullptr,
                         iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeConditionVariable.value();
}

bool PoshRuntime::sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept
{
    // runtime must be thread safe
    std::lock_guard<std::mutex> g(m_appMqRequestMutex);
    return m_MqInterface.sendRequestToRouDi(msg, answer);
}

// this is the callback for the m_keepAliveTimer
void PoshRuntime::sendKeepAlive() noexcept
{
    if (!m_MqInterface.sendKeepalive())
    {
        LogWarn() << "Error in sending keep alive";
    }
}

} // namespace runtime
} // namespace iox
