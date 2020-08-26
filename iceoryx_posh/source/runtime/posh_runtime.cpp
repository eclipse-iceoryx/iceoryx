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

#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_posh/runtime/runnable.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
std::function<PoshRuntime&(const std::string& name)> PoshRuntime::s_runtimeFactory = PoshRuntime::defaultRuntimeFactory;


PoshRuntime& PoshRuntime::defaultRuntimeFactory(const std::string& name) noexcept
{
    static PoshRuntime instance(name);
    return instance;
}

// singleton access
PoshRuntime& PoshRuntime::getInstance(const std::string& name) noexcept
{
    return PoshRuntime::s_runtimeFactory(name);
}

PoshRuntime::PoshRuntime(const std::string& name, const bool doMapSharedMemoryIntoThread) noexcept
    : m_appName(verifyInstanceName(name))
    , m_MqInterface(MQ_ROUDI_NAME, name, PROCESS_WAITING_FOR_ROUDI_TIMEOUT)
    , m_ShmInterface(doMapSharedMemoryIntoThread,
                     m_MqInterface.getShmTopicSize(),
                     m_MqInterface.getSegmentManagerAddr(),
                     m_MqInterface.getSegmentId())
    , m_applicationPort(getMiddlewareApplication())
{
    m_keepAliveTimer.start(posix::Timer::RunMode::PERIODIC, posix::Timer::CatchUpPolicy::IMMEDIATE);
    /// @todo here we could get the LogLevel and LogMode and set it on the LogManager
}

PoshRuntime::~PoshRuntime() noexcept
{
    if (m_applicationPort)
    {
        m_applicationPort.destroy();
    }
}


const std::string& PoshRuntime::verifyInstanceName(const std::string& name) noexcept
{
    if (name.empty())
    {
        LogError() << "Cannot initialize runtime. Application name must not be empty!";
        std::terminate();
    }
    else if (name.compare(DEFAULT_RUNTIME_INSTANCE_NAME) == 0)
    {
        LogError() << "Cannot initialize runtime. Application name has not been specified!";
        std::terminate();
    }
    else if (name.front() != '/')
    {
        LogError() << "Cannot initialize runtime. Application name " << name
                   << " does not have the required leading slash '/'";
        std::terminate();
    }
    else if (name.length() > MAX_PROCESS_NAME_LENGTH)
    {
        LogError() << "Application name has more than 100 characters, including null termination!";
        std::terminate();
    }

    return name;
}

std::string PoshRuntime::getInstanceName() const noexcept
{
    return m_appName;
}

const std::atomic<uint64_t>* PoshRuntime::getServiceRegistryChangeCounter() noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::SERVICE_REGISTRY_CHANGE_COUNTER) << m_appName;
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (2 == receiveBuffer.getNumberOfElements()))
    {
        RelativePointer::offset_t offset;
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(0).c_str(), offset);
        RelativePointer::id_t segmentId;
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), segmentId);
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

SenderPortType::MemberType_t* PoshRuntime::getMiddlewareSender(const capro::ServiceDescription& service,
                                                               const cxx::CString100& runnableName,
                                                               const PortConfigInfo& portConfigInfo) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_SENDER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << runnableName
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    auto requestedSenderPort = requestSenderFromRoudi(sendBuffer);
    if (requestedSenderPort.has_error())
    {
        switch (requestedSenderPort.get_error())
        {
        case MqMessageErrorType::NO_UNIQUE_CREATED:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' already in use by another process.";
            errorHandler(Error::kPOSH__RUNTIME_SENDERPORT_NOT_UNIQUE, nullptr, iox::ErrorLevel::MODERATE);
            break;
        case MqMessageErrorType::SENDERLIST_FULL:
            LogWarn() << "Service '" << service.operator cxx::Serialization().toString()
                      << "' could not be created since we are out of memory.";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_SENDERLIST_FULL, nullptr, iox::ErrorLevel::SEVERE);
            break;
        default:
            LogWarn() << "Undefined behavior occurred while creating service '"
                      << service.operator cxx::Serialization().toString() << "'.";
            errorHandler(
                Error::kPOSH__RUNTIME_SENDERPORT_CREATION_UNDEFINED_BEHAVIOR, nullptr, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return requestedSenderPort.get_value();
}

cxx::expected<SenderPortType::MemberType_t*, MqMessageErrorType>
PoshRuntime::requestSenderFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_SENDER_ACK)

        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return cxx::success<SenderPortType::MemberType_t*>(reinterpret_cast<SenderPortType::MemberType_t*>(ptr));
        }
        else
        {
            LogError() << "Wrong response from message queue" << mqMessage;
            assert(false);
            return cxx::success<SenderPortType::MemberType_t*>(nullptr);
        }
    }
    else
    {
        if (receiveBuffer.getNumberOfElements() == 2)
        {
            std::string mqMessage1 = receiveBuffer.getElementAtIndex(0);
            std::string mqMessage2 = receiveBuffer.getElementAtIndex(1);
            if (stringToMqMessageType(mqMessage1.c_str()) == MqMessageType::ERROR)
            {
                LogError() << "No valid sender port received from RouDi.";
                return cxx::error<MqMessageErrorType>(stringToMqMessageErrorType(mqMessage2.c_str()));
            }
        }

        LogError() << "Wrong response from message queue :'" << receiveBuffer.getMessage() << "'";
        assert(false);
        return cxx::success<SenderPortType::MemberType_t*>(nullptr);
    }
}

ReceiverPortType::MemberType_t* PoshRuntime::getMiddlewareReceiver(const capro::ServiceDescription& service,
                                                                   const cxx::CString100& runnableName,
                                                                   const PortConfigInfo& portConfigInfo) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_RECEIVER) << m_appName
               << static_cast<cxx::Serialization>(service).toString() << runnableName
               << static_cast<cxx::Serialization>(portConfigInfo).toString();

    return requestReceiverFromRoudi(sendBuffer);
}

ReceiverPortType::MemberType_t* PoshRuntime::requestReceiverFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_RECEIVER_ACK)
        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<ReceiverPortType::MemberType_t*>(ptr);
        }
        else
        {
            LogError() << "Wrong response from message queue " << mqMessage;
            errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
            return nullptr;
        }
    }
    else
    {
        LogError() << "Wrong response from message queue";
        errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
}

popo::InterfacePortData* PoshRuntime::getMiddlewareInterface(const capro::Interfaces interface,
                                                             const cxx::CString100& runnableName) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_INTERFACE) << m_appName
               << static_cast<uint32_t>(interface) << runnableName;

    MqMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_INTERFACE_ACK)
        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<popo::InterfacePortData*>(ptr);
        }
        else
        {
            LogError() << "Wrong response from message queue " << mqMessage;
            errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
            return nullptr;
        }
    }
    else
    {
        LogError() << "Wrong response from message queue";
        errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
}

RunnableData* PoshRuntime::createRunnable(const RunnableProperty& runnableProperty) noexcept
{
    MqMessage sendBuffer;
    sendBuffer << mqMessageTypeToString(MqMessageType::CREATE_RUNNABLE) << m_appName
               << static_cast<std::string>(runnableProperty);

    MqMessage receiveBuffer;

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_RUNNABLE_ACK)
        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<RunnableData*>(ptr);
        }
        else
        {
            LogError() << "Wrong response from message queue " << mqMessage;
            errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
            return nullptr;
        }
    }
    else
    {
        LogError() << "Wrong response from message queue";
        errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
}

cxx::expected<Error> PoshRuntime::findService(const capro::ServiceDescription& serviceDescription,
                                              InstanceContainer& instanceContainer) noexcept
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

    uint32_t numberOfElements = requestResponse.getNumberOfElements();
    uint32_t capacity = static_cast<uint32_t>(instanceContainer.capacity());

    // Limit the instances (max value is the capacity of instanceContainer)
    uint32_t numberOfInstances = ((numberOfElements > capacity) ? capacity : numberOfElements);
    for (uint32_t i = 0; i < numberOfInstances; ++i)
    {
        IdString instance(iox::cxx::TruncateToCapacity, requestResponse.getElementAtIndex(i).c_str());
        instanceContainer.push_back(instance);
    }

    if (numberOfElements > capacity)
    {
        LogWarn() << numberOfElements << " instances found for service \"" << serviceDescription.getServiceIDString()
                  << "\" which is more than supported number of instances(" << MAX_NUMBER_OF_INSTANCES << "\n";
        errorHandler(Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<Error>(Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW);
    }
    return {cxx::success<>()};
}

void PoshRuntime::offerService(const capro::ServiceDescription& serviceDescription) noexcept
{
    capro::CaproMessage msg(capro::CaproMessageType::OFFER, serviceDescription, capro::CaproMessageSubType::SERVICE);
    m_applicationPort.dispatchCaProMessage(msg);
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

    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_APPLICATION_ACK)
        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return reinterpret_cast<popo::ApplicationPortData*>(ptr);
        }
        else
        {
            LogError() << "Wrong response from message queue" << mqMessage;
            assert(false);
            return nullptr;
        }
    }
    else
    {
        LogError() << "Wrong response from message queue";
        errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
        return nullptr;
    }
}

cxx::expected<popo::ConditionVariableData*, MqMessageErrorType>
PoshRuntime::requestConditionVariableFromRoudi(const MqMessage& sendBuffer) noexcept
{
    MqMessage receiveBuffer;
    if (sendRequestToRouDi(sendBuffer, receiveBuffer) && (3 == receiveBuffer.getNumberOfElements()))
    {
        std::string mqMessage = receiveBuffer.getElementAtIndex(0);

        if (stringToMqMessageType(mqMessage.c_str()) == MqMessageType::CREATE_CONDITION_VARIABLE_ACK)
        {
            RelativePointer::id_t segmentId;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(2).c_str(), segmentId);
            RelativePointer::offset_t offset;
            cxx::convert::fromString(receiveBuffer.getElementAtIndex(1).c_str(), offset);
            auto ptr = RelativePointer::getPtr(segmentId, offset);
            return cxx::success<popo::ConditionVariableData*>(reinterpret_cast<popo::ConditionVariableData*>(ptr));
        }
        else
        {
            LogError() << "Wrong response from message queue " << mqMessage;
            errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
            return cxx::success<popo::ConditionVariableData*>(nullptr);
        }
    }
    else
    {
        if (receiveBuffer.getNumberOfElements() == 2)
        {
            std::string mqMessage1 = receiveBuffer.getElementAtIndex(0);
            std::string mqMessage2 = receiveBuffer.getElementAtIndex(1);
            if (stringToMqMessageType(mqMessage1.c_str()) == MqMessageType::ERROR)
            {
                LogError() << "No valid condition variable received from RouDi.";
                errorHandler(
                    Error::kPOSH__RUNTIME_NO_VALID_CONDITION_VARIABLE_RECEIVED, nullptr, iox::ErrorLevel::MODERATE);
                return cxx::error<MqMessageErrorType>(stringToMqMessageErrorType(mqMessage2.c_str()));
            }
        }

        LogError() << "Wrong response from message queue";
        errorHandler(Error::kPOSH__RUNTIME_WRONG_MESSAGE_QUEUE_RESPONSE, nullptr, iox::ErrorLevel::SEVERE);
        return cxx::success<popo::ConditionVariableData*>(nullptr);
    }
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
            LogWarn() << "Could not create another condition variable as we are out of memory";
            errorHandler(Error::kPOSH__RUNTIME_ROUDI_CONDITION_VARIABLE_LIST_FULL, nullptr, iox::ErrorLevel::SEVERE);
            break;
        default:
            LogWarn() << "Undefined behavior occurred while creating condition variable";
            errorHandler(
                Error::kPOSH__RUNTIME_CONDITION_VARIABLE_CREATION_UNDEFINED_BEHAVIOR, nullptr, iox::ErrorLevel::SEVERE);
            break;
        }
        return nullptr;
    }
    return maybeConditionVariable.get_value();
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
