// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/ipc_interface_base.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"

#include <thread>

namespace iox
{
namespace runtime
{
IpcMessageType stringToIpcMessageType(const char* str) noexcept
{
    std::underlying_type<IpcMessageType>::type msg;
    bool noError = cxx::convert::stringIsNumber(str, cxx::convert::NumberType::INTEGER);
    noError &= noError ? (cxx::convert::fromString(str, msg)) : false;
    noError &= noError ? !(static_cast<std::underlying_type<IpcMessageType>::type>(IpcMessageType::BEGIN) >= msg
                           || static_cast<std::underlying_type<IpcMessageType>::type>(IpcMessageType::END) <= msg)
                       : false;
    return noError ? (static_cast<IpcMessageType>(msg)) : IpcMessageType::NOTYPE;
}

std::string IpcMessageTypeToString(const IpcMessageType msg) noexcept
{
    return std::to_string(static_cast<std::underlying_type<IpcMessageType>::type>(msg));
}

IpcMessageErrorType stringToIpcMessageErrorType(const char* str) noexcept
{
    std::underlying_type<IpcMessageErrorType>::type msg;
    bool noError = cxx::convert::stringIsNumber(str, cxx::convert::NumberType::INTEGER);
    noError &= noError ? (cxx::convert::fromString(str, msg)) : false;
    noError &= noError
                   ? !(static_cast<std::underlying_type<IpcMessageErrorType>::type>(IpcMessageErrorType::BEGIN) >= msg
                       || static_cast<std::underlying_type<IpcMessageErrorType>::type>(IpcMessageErrorType::END) <= msg)
                   : false;
    return noError ? (static_cast<IpcMessageErrorType>(msg)) : IpcMessageErrorType::NOTYPE;
}

std::string IpcMessageErrorTypeToString(const IpcMessageErrorType msg) noexcept
{
    return std::to_string(static_cast<std::underlying_type<IpcMessageErrorType>::type>(msg));
}

IpcInterfaceBase::IpcInterfaceBase(const ProcessName_t& InterfaceName,
                                   const uint64_t maxMessages,
                                   const uint64_t messageSize) noexcept
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

bool IpcInterfaceBase::receive(IpcMessage& answer) const noexcept
{
    auto message = m_ipcChannel.receive();
    if (message.has_error())
    {
        return false;
    }

    return IpcInterfaceBase::setMessageFromString(message.value().c_str(), answer);
}

bool IpcInterfaceBase::timedReceive(const units::Duration timeout, IpcMessage& answer) const noexcept
{
    return !m_ipcChannel.timedReceive(timeout)
                .and_then([&answer](auto& message) { IpcInterfaceBase::setMessageFromString(message.c_str(), answer); })
                .has_error()
           && answer.isValid();
}

bool IpcInterfaceBase::setMessageFromString(const char* buffer, IpcMessage& answer) noexcept
{
    answer.setMessage(buffer);
    if (!answer.isValid())
    {
        LogError() << "The received message " << answer.getMessage() << " is not valid";
        return false;
    }
    return true;
}

bool IpcInterfaceBase::send(const IpcMessage& msg) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << " which "
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
    return !m_ipcChannel.send(msg.getMessage()).or_else(logLengthError).has_error();
}

bool IpcInterfaceBase::timedSend(const IpcMessage& msg, units::Duration timeout) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << " which "
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
    return !m_ipcChannel.timedSend(msg.getMessage(), timeout).or_else(logLengthError).has_error();
}

const ProcessName_t& IpcInterfaceBase::getInterfaceName() const noexcept
{
    return m_interfaceName;
}

bool IpcInterfaceBase::isInitialized() const noexcept
{
    return m_ipcChannel.isInitialized();
}

bool IpcInterfaceBase::openIpcChannel(const posix::IpcChannelSide channelSide) noexcept
{
    m_ipcChannel.destroy();

    m_channelSide = channelSide;
    IpcChannelType::create(
        m_interfaceName, posix::IpcChannelMode::BLOCKING, m_channelSide, m_maxMessageSize, m_maxMessages)
        .and_then([this](auto& ipcChannel) { this->m_ipcChannel = std::move(ipcChannel); });

    return m_ipcChannel.isInitialized();
}

bool IpcInterfaceBase::closeIpcChannel() noexcept
{
    return !m_ipcChannel.destroy().has_error();
}

bool IpcInterfaceBase::reopen() noexcept
{
    return openIpcChannel(m_channelSide);
}

bool IpcInterfaceBase::ipcChannelMapsToFile() noexcept
{
    return !m_ipcChannel.isOutdated().value_or(true);
}

bool IpcInterfaceBase::hasClosableIpcChannel() const noexcept
{
    return m_ipcChannel.isInitialized();
}

void IpcInterfaceBase::cleanupOutdatedIpcChannel(const ProcessName_t& name) noexcept
{
    if (posix::MessageQueue::unlinkIfExists(name).value_or(false))
    {
        LogWarn() << "IPC channel still there, doing an unlink of " << name;
    }
}

} // namespace runtime
} // namespace iox
