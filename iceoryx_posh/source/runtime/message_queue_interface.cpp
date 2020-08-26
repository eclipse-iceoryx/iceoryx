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

#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/version/version_info.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/internal/posix_wrapper/timespec.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include <thread>

namespace iox
{
namespace runtime
{
MqMessageType stringToMqMessageType(const char* str) noexcept
{
    std::underlying_type<MqMessageType>::type msg;
    bool noError = cxx::convert::stringIsNumber(str, cxx::convert::NumberType::INTEGER);
    noError &= noError ? (cxx::convert::fromString(str, msg)) : false;
    noError &= noError ? !(static_cast<std::underlying_type<MqMessageType>::type>(MqMessageType::BEGIN) >= msg
                           || static_cast<std::underlying_type<MqMessageType>::type>(MqMessageType::END) <= msg)
                       : false;
    return noError ? (static_cast<MqMessageType>(msg)) : MqMessageType::NOTYPE;
}

std::string mqMessageTypeToString(const MqMessageType msg) noexcept
{
    return std::to_string(static_cast<std::underlying_type<MqMessageType>::type>(msg));
}

MqMessageErrorType stringToMqMessageErrorType(const char* str) noexcept
{
    std::underlying_type<MqMessageErrorType>::type msg;
    bool noError = cxx::convert::stringIsNumber(str, cxx::convert::NumberType::INTEGER);
    noError &= noError ? (cxx::convert::fromString(str, msg)) : false;
    noError &= noError
                   ? !(static_cast<std::underlying_type<MqMessageErrorType>::type>(MqMessageErrorType::BEGIN) >= msg
                       || static_cast<std::underlying_type<MqMessageErrorType>::type>(MqMessageErrorType::END) <= msg)
                   : false;
    return noError ? (static_cast<MqMessageErrorType>(msg)) : MqMessageErrorType::NOTYPE;
}

std::string mqMessageErrorTypeToString(const MqMessageErrorType msg) noexcept
{
    return std::to_string(static_cast<std::underlying_type<MqMessageErrorType>::type>(msg));
}

MqBase::MqBase(const std::string& InterfaceName, const uint64_t maxMessages, const uint64_t messageSize) noexcept
    : m_interfaceName(InterfaceName)
{
    m_maxMessages = maxMessages;
    m_maxMessageSize = messageSize;
    if (m_maxMessageSize > posix::MessageQueue::MAX_MESSAGE_SIZE)
    {
        LogWarn() << "Message size too large, reducing from " << messageSize << " to "
                  << posix::MessageQueue::MAX_MESSAGE_SIZE;
        m_maxMessageSize = posix::MessageQueue::MAX_MESSAGE_SIZE;
    }
}

bool MqBase::receive(MqMessage& answer) const noexcept
{
    auto message = m_mq.receive();
    if (message.has_error())
    {
        return false;
    }

    return MqBase::setMessageFromString(message.get_value().c_str(), answer);
}

bool MqBase::timedReceive(const units::Duration timeout, MqMessage& answer) const noexcept
{
    return !m_mq.timedReceive(timeout)
                .and_then([&answer](std::string& message) { MqBase::setMessageFromString(message.c_str(), answer); })
                .has_error()
           && answer.isValid();
}

bool MqBase::setMessageFromString(const char* buffer, MqMessage& answer) noexcept
{
    answer.setMessage(buffer);
    if (!answer.isValid())
    {
        LogError() << "The received message " << answer.getMessage() << " is not valid";
        return false;
    }
    return true;
}

bool MqBase::send(const MqMessage& msg) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << "with mq_send() which "
                   << "does not follow the specified syntax.";
        return false;
    }

    auto logLengthError = [&msg](posix::IpcChannelError& error) {
        if (error == posix::IpcChannelError::MESSAGE_TOO_LONG)
        {
            const size_t messageSize =
                static_cast<size_t>(msg.getMessage().size()) + posix::MessageQueue::NULL_TERMINATOR_SIZE;
            LogError() << "msg size of " << messageSize << "bigger than configured max message size";
        }
    };
    return !m_mq.send(msg.getMessage()).or_else(logLengthError).has_error();
}

bool MqBase::timedSend(const MqMessage& msg, units::Duration timeout) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << " with mq_timedsend() which "
                   << "does not follow the specified syntax.";
        return false;
    }

    auto logLengthError = [&msg](posix::IpcChannelError& error) {
        if (error == posix::IpcChannelError::MESSAGE_TOO_LONG)
        {
            const size_t messageSize =
                static_cast<size_t>(msg.getMessage().size()) + posix::MessageQueue::NULL_TERMINATOR_SIZE;
            LogError() << "msg size of " << messageSize << "bigger than configured max message size";
        }
    };
    return !m_mq.timedSend(msg.getMessage(), timeout).or_else(logLengthError).has_error();
}

const std::string& MqBase::getInterfaceName() const noexcept
{
    return m_interfaceName;
}

bool MqBase::isInitialized() const noexcept
{
    return m_mq.isInitialized();
}

bool MqBase::openMessageQueue(const posix::IpcChannelSide channelSide) noexcept
{
    m_mq.destroy();

    m_channelSide = channelSide;
    IpcChannelType::create(
        m_interfaceName, posix::IpcChannelMode::BLOCKING, m_channelSide, m_maxMessageSize, m_maxMessages)
        .and_then([this](IpcChannelType& mq) { this->m_mq = std::move(mq); });

    return m_mq.isInitialized();
}

bool MqBase::closeMessageQueue() noexcept
{
    return !m_mq.destroy().has_error();
}

bool MqBase::reopen() noexcept
{
    return openMessageQueue(m_channelSide);
}

bool MqBase::mqMapsToFile() noexcept
{
    return !m_mq.isOutdated().get_value_or(true);
}

bool MqBase::hasClosableMessageQueue() const noexcept
{
    return m_mq.isInitialized();
}

void MqBase::cleanupOutdatedMessageQueue(const std::string& name) noexcept
{
    if (posix::MessageQueue::unlinkIfExists(name).get_value_or(false))
    {
        LogWarn() << "MQ still there, doing an unlink of " << name;
    }
}

MqInterfaceUser::MqInterfaceUser(const std::string& name, const int64_t maxMessages, const int64_t messageSize) noexcept
    : MqBase(name, maxMessages, messageSize)
{
    openMessageQueue(posix::IpcChannelSide::CLIENT);
}

MqInterfaceCreator::MqInterfaceCreator(const std::string& name,
                                       const int64_t maxMessages,
                                       const int64_t messageSize) noexcept
    : MqBase(name, maxMessages, messageSize)
{
    // check if the mq is still there (e.g. because of no proper termination
    // of the process)
    cleanupOutdatedMessageQueue(name);

    openMessageQueue(posix::IpcChannelSide::SERVER);
}

void MqInterfaceCreator::cleanupResource()
{
    m_mq.destroy();
}

MqRuntimeInterface::MqRuntimeInterface(const std::string& roudiName,
                                       const std::string& appName,
                                       const units::Duration roudiWaitingTimeout) noexcept
    : m_appName(appName)
    , m_AppMqInterface(appName)
    , m_RoudiMqInterface(roudiName)
{
    posix::Timer timer(roudiWaitingTimeout);

    enum class RegState
    {
        WAIT_FOR_ROUDI,
        SEND_REGISTER_REQUEST,
        WAIT_FOR_REGISTER_ACK,
        FINISHED
    };

    int64_t transmissionTimestamp{0};
    auto regState = RegState::WAIT_FOR_ROUDI;
    while (!timer.hasExpiredComparedToCreationTime() && regState != RegState::FINISHED)
    {
        if (!m_RoudiMqInterface.isInitialized() || !m_RoudiMqInterface.mqMapsToFile())
        {
            LogDebug() << "reopen RouDi mqueue!";
            m_RoudiMqInterface.reopen();
            regState = RegState::WAIT_FOR_ROUDI;
        }

        switch (regState)
        {
        case RegState::WAIT_FOR_ROUDI:
        {
            waitForRoudi(timer);
            if (m_RoudiMqInterface.isInitialized())
            {
                regState = RegState::SEND_REGISTER_REQUEST;
            }
            break;
        }
        case RegState::SEND_REGISTER_REQUEST:
        {
            using namespace units;
            using namespace std::chrono;
            auto timestamp = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
            while (transmissionTimestamp == timestamp)
            {
                timestamp = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
            }
            transmissionTimestamp = timestamp;

            // send MqMessageType::REG to RouDi

            MqMessage sendBuffer;
            sendBuffer << mqMessageTypeToString(MqMessageType::REG) << m_appName << std::to_string(getpid())
                       << std::to_string(posix::PosixUser::getUserOfCurrentProcess().getID())
                       << std::to_string(transmissionTimestamp)
                       << static_cast<cxx::Serialization>(version::VersionInfo::getCurrentVersion()).toString();

            bool successfullySent = m_RoudiMqInterface.timedSend(sendBuffer, 100_ms);

            if (successfullySent)
            {
                regState = RegState::WAIT_FOR_REGISTER_ACK;
            }
            else
            {
                regState = RegState::WAIT_FOR_ROUDI;
            }
            break;
        }
        case RegState::WAIT_FOR_REGISTER_ACK:
            if (waitForRegAck(transmissionTimestamp) == RegAckResult::SUCCESS)
            {
                regState = RegState::FINISHED;
            }
            else
            {
                regState = RegState::WAIT_FOR_ROUDI;
            }
            break;
        case RegState::FINISHED:
            // nothing to do, move along
            break;
        }
    }

    if (regState != RegState::FINISHED)
    {
        m_AppMqInterface.cleanupResource();
    }
    switch (regState)
    {
    case RegState::WAIT_FOR_ROUDI:
        errorHandler(Error::kMQ_INTERFACE__REG_ROUDI_NOT_AVAILABLE);
        break;
    case RegState::SEND_REGISTER_REQUEST:
        errorHandler(Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ);
        break;
    case RegState::WAIT_FOR_REGISTER_ACK:
        errorHandler(Error::kMQ_INTERFACE__REG_ACK_NO_RESPONSE);
        break;
    case RegState::FINISHED:
        // nothing to do, move along
        break;
    }
}

bool MqRuntimeInterface::sendKeepalive() noexcept
{
    return m_RoudiMqInterface.send({mqMessageTypeToString(MqMessageType::KEEPALIVE), m_appName});
}

std::string MqRuntimeInterface::getSegmentManagerAddr() const noexcept
{
    return m_segmentManager;
}

bool MqRuntimeInterface::sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept
{
    if (!m_RoudiMqInterface.send(msg))
    {
        LogError() << "Could not send request via RouDi messagequeue interface.\n";
        return false;
    }

    if (!m_AppMqInterface.receive(answer))
    {
        LogError() << "Could not receive request via App messagequeue interface.\n";
        return false;
    }

    return true;
}

bool MqRuntimeInterface::sendMessageToRouDi(const MqMessage& msg) noexcept
{
    if (!m_RoudiMqInterface.send(msg))
    {
        LogError() << "Could not send message via RouDi messagequeue interface.\n";
        return false;
    }
    return true;
}

size_t MqRuntimeInterface::getShmTopicSize() noexcept
{
    return m_shmTopicSize;
}

void MqRuntimeInterface::waitForRoudi(posix::Timer& timer) noexcept
{
    bool printWaitingWarning = true;
    bool printFoundMessage = false;
    while (!timer.hasExpiredComparedToCreationTime() && !m_RoudiMqInterface.isInitialized())
    {
        m_RoudiMqInterface.reopen();

        if (m_RoudiMqInterface.isInitialized())
        {
            LogDebug() << "RouDi mqueue found!";
            break;
        }

        if (printWaitingWarning)
        {
            LogWarn() << "RouDi not found - waiting ...";
            printWaitingWarning = false;
            printFoundMessage = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (printFoundMessage && m_RoudiMqInterface.isInitialized())
    {
        LogWarn() << "... RouDi found.";
    }
}

MqRuntimeInterface::RegAckResult MqRuntimeInterface::waitForRegAck(int64_t transmissionTimestamp) noexcept
{
    // wait for the register ack from the RouDi daemon. If we receive another response we do a retry
    // @todo if message queues are properly setup and cleaned up, always the expected REG_ACK should be received here,
    // so if not this issue should be passed to the error handling later
    constexpr size_t MAX_RETRY_COUNT = 3;
    size_t retryCounter = 0;
    while (retryCounter++ < MAX_RETRY_COUNT)
    {
        MqMessage receiveBuffer;
        // wait for MqMessageType::REG_ACK from RouDi for 1 seconds
        if (m_AppMqInterface.timedReceive(1_s, receiveBuffer))
        {
            std::string cmd = receiveBuffer.getElementAtIndex(0);

            if (stringToMqMessageType(cmd.c_str()) == MqMessageType::REG_ACK)
            {
                constexpr uint32_t REGISTER_ACK_PARAMETERS = 5;
                if (receiveBuffer.getNumberOfElements() != REGISTER_ACK_PARAMETERS)
                {
                    errorHandler(Error::kMQ_INTERFACE__REG_ACK_INVALIG_NUMBER_OF_PARAMS);
                }

                // read out the shared memory base address and save it
                m_shmTopicSize = strtoull(receiveBuffer.getElementAtIndex(1).c_str(), nullptr, 10);
                m_segmentManager = receiveBuffer.getElementAtIndex(2);

                int64_t receivedTimestamp;
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(3).c_str(), receivedTimestamp);
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(4).c_str(), m_segmentId);
                if (transmissionTimestamp == receivedTimestamp)
                {
                    return RegAckResult::SUCCESS;
                }
                else
                {
                    LogWarn() << "Received a REG_ACK with an outdated timestamp!";
                }
            }
            else
            {
                LogError() << "Wrong response received " << receiveBuffer.getMessage();
            }
        }
    }

    return RegAckResult::TIMEOUT;
}

uint64_t MqRuntimeInterface::getSegmentId() const noexcept
{
    return m_segmentId;
}

} // namespace runtime
} // namespace iox
