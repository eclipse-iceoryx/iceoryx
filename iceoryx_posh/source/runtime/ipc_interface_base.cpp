// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"

#include <thread>

namespace iox
{
namespace runtime
{
IpcMessageType stringToIpcMessageType(const char* str) noexcept
{
    using UnderlyingType = std::underlying_type<IpcMessageType>::type;
    UnderlyingType msg;
    auto result = convert::from_string<UnderlyingType>(str);

    if (!result.has_value())
    {
        return IpcMessageType::NOTYPE;
    }

    msg = result.value();

    if (static_cast<UnderlyingType>(IpcMessageType::BEGIN) >= msg
        || static_cast<UnderlyingType>(IpcMessageType::END) <= msg)
    {
        return IpcMessageType::NOTYPE;
    }

    return static_cast<IpcMessageType>(msg);
}

std::string IpcMessageTypeToString(const IpcMessageType msg) noexcept
{
    using UnderlyingType = std::underlying_type<IpcMessageType>::type;
    return convert::toString(static_cast<UnderlyingType>(msg));
}

IpcMessageErrorType stringToIpcMessageErrorType(const char* str) noexcept
{
    using UnderlyingType = std::underlying_type<IpcMessageErrorType>::type;
    UnderlyingType msg;
    auto result = convert::from_string<UnderlyingType>(str);

    msg = result.value();

    if (static_cast<UnderlyingType>(IpcMessageErrorType::BEGIN) >= msg
        || static_cast<UnderlyingType>(IpcMessageErrorType::END) <= msg)
    {
        return IpcMessageErrorType::NOTYPE;
    }

    return static_cast<IpcMessageErrorType>(msg);
}

std::string IpcMessageErrorTypeToString(const IpcMessageErrorType msg) noexcept
{
    using UnderlyingType = std::underlying_type<IpcMessageErrorType>::type;
    return convert::toString(static_cast<UnderlyingType>(msg));
}

InterfaceName_t ipcChannelNameToInterfaceName(RuntimeName_t channelName, DomainId domainId, ResourceType resourceType)
{
    return concatenate(iceoryxResourcePrefix(domainId, resourceType), channelName);
}

template <typename IpcChannelType>
IpcInterface<IpcChannelType>::IpcInterface(const RuntimeName_t& runtimeName,
                                           const DomainId domainId,
                                           const ResourceType resourceType,
                                           const uint64_t maxMessages,
                                           const uint64_t messageSize) noexcept
{
    if (runtimeName.empty())
    {
        IOX_PANIC("Then runtime name must not be empty");
    }
    for (const auto s : platform::IOX_PATH_SEPARATORS)
    {
        const char separator[2]{s};
        if (runtimeName.find(separator).has_value())
        {
            IOX_LOG(FATAL, "The runtime name '" << runtimeName << "' contains path separators");
            IOX_PANIC("Invalid characters for runtime name");
        }
    }

    m_interfaceName = ipcChannelNameToInterfaceName(runtimeName, domainId, resourceType);
    m_runtimeName = runtimeName;
    m_maxMessages = maxMessages;
    m_maxMessageSize = messageSize;
    if (m_maxMessageSize > platform::IoxIpcChannelType::MAX_MESSAGE_SIZE)
    {
        IOX_LOG(WARN,
                "Message size too large, reducing from " << messageSize << " to "
                                                         << platform::IoxIpcChannelType::MAX_MESSAGE_SIZE);
        m_maxMessageSize = platform::IoxIpcChannelType::MAX_MESSAGE_SIZE;
    }
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::receive(IpcMessage& answer) const noexcept
{
    if (!m_ipcChannel.has_value())
    {
        IOX_LOG(WARN,
                "Trying to receive data on an non-initialized IPC interface! Interface name: " << m_interfaceName);
        return false;
    }

    auto message = m_ipcChannel->receive();
    if (message.has_error())
    {
        return false;
    }

    return IpcInterface<IpcChannelType>::setMessageFromString(message.value().c_str(), answer);
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::timedReceive(const units::Duration timeout, IpcMessage& answer) const noexcept
{
    if (!m_ipcChannel.has_value())
    {
        IOX_LOG(WARN,
                "Trying to receive data on an non-initialized IPC interface! Interface name: " << m_interfaceName);
        return false;
    }

    return !m_ipcChannel->timedReceive(timeout)
                .and_then([&answer](auto& message) {
                    IpcInterface<IpcChannelType>::setMessageFromString(message.c_str(), answer);
                })
                .has_error()
           && answer.isValid();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::setMessageFromString(const char* buffer, IpcMessage& answer) noexcept
{
    answer.setMessage(buffer);
    if (!answer.isValid())
    {
        IOX_LOG(ERROR, "The received message " << answer.getMessage() << " is not valid");
        return false;
    }
    return true;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::send(const IpcMessage& msg) const noexcept
{
    if (!m_ipcChannel.has_value())
    {
        IOX_LOG(WARN, "Trying to send data on an non-initialized IPC interface! Interface name: " << m_interfaceName);
        return false;
    }

    if (!msg.isValid())
    {
        IOX_LOG(ERROR,
                "Trying to send the message " << msg.getMessage() << " which "
                                              << "does not follow the specified syntax.");
        return false;
    }

    auto logLengthError = [&msg](PosixIpcChannelError& error) {
        if (error == PosixIpcChannelError::MESSAGE_TOO_LONG)
        {
            const uint64_t messageSize = msg.getMessage().size() + platform::IoxIpcChannelType::NULL_TERMINATOR_SIZE;
            IOX_LOG(ERROR, "msg size of " << messageSize << " bigger than configured max message size");
        }
    };
    return !m_ipcChannel->send(msg.getMessage()).or_else(logLengthError).has_error();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::timedSend(const IpcMessage& msg, units::Duration timeout) const noexcept
{
    if (!m_ipcChannel.has_value())
    {
        IOX_LOG(WARN, "Trying to send data on an non-initialized IPC interface! Interface name: " << m_interfaceName);
        return false;
    }

    if (!msg.isValid())
    {
        IOX_LOG(ERROR,
                "Trying to send the message " << msg.getMessage() << " which "
                                              << "does not follow the specified syntax.");
        return false;
    }

    auto logLengthError = [&msg](PosixIpcChannelError& error) {
        if (error == PosixIpcChannelError::MESSAGE_TOO_LONG)
        {
            const uint64_t messageSize = msg.getMessage().size() + platform::IoxIpcChannelType::NULL_TERMINATOR_SIZE;
            IOX_LOG(ERROR, "msg size of " << messageSize << " bigger than configured max message size");
        }
    };
    return !m_ipcChannel->timedSend(msg.getMessage(), timeout).or_else(logLengthError).has_error();
}

template <typename IpcChannelType>
const RuntimeName_t& IpcInterface<IpcChannelType>::getRuntimeName() const noexcept
{
    return m_runtimeName;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::isInitialized() const noexcept
{
    return m_ipcChannel.has_value();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::openIpcChannel(const PosixIpcChannelSide channelSide) noexcept
{
    m_channelSide = channelSide;

    using IpcChannelBuilder_t = typename IpcChannelType::Builder_t;
    IpcChannelBuilder_t()
        .name(m_interfaceName)
        .channelSide(m_channelSide)
        .maxMsgSize(m_maxMessageSize)
        .maxMsgNumber(m_maxMessages)
        .create()
        .and_then([this](auto& ipcChannel) { this->m_ipcChannel.emplace(std::move(ipcChannel)); })
        .or_else([this](auto& err) {
            if (this->m_channelSide == PosixIpcChannelSide::SERVER)
            {
                IOX_LOG(ERROR,
                        "Unable to create ipc channel '" << this->m_interfaceName
                                                         << "'. Error code: " << static_cast<uint8_t>(err));
            }
            else
            {
                // the client opens the channel and tries to do this in a loop when the channel is not available,
                // therefore resulting in a wall of error messages on the console which leads to missing the important
                // one that roudi is not running if this would be LogLevel::ERROR instead of LogLevel::TRACE
                IOX_LOG(TRACE,
                        "Unable to open ipc channel '" << this->m_interfaceName
                                                       << "'. Error code: " << static_cast<uint8_t>(err));
            }
        });

    return isInitialized();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::reopen() noexcept
{
    return openIpcChannel(m_channelSide);
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::ipcChannelMapsToFile() noexcept
{
    return m_ipcChannel.has_value() && !m_ipcChannel->isOutdated().value_or(true);
}

template <>
bool IpcInterface<UnixDomainSocket>::ipcChannelMapsToFile() noexcept
{
    return true;
}

template <>
bool IpcInterface<NamedPipe>::ipcChannelMapsToFile() noexcept
{
    return true;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::hasClosableIpcChannel() const noexcept
{
    return isInitialized();
}

template <typename IpcChannelType>
void IpcInterface<IpcChannelType>::cleanupOutdatedIpcChannel(const InterfaceName_t& name) noexcept
{
    if (platform::IoxIpcChannelType::unlinkIfExists(name).value_or(false))
    {
        IOX_LOG(WARN, "IPC channel still there, doing an unlink of '" << name << "'");
    }
}

template class IpcInterface<UnixDomainSocket>;
template class IpcInterface<NamedPipe>;
template class IpcInterface<MessageQueue>;

} // namespace runtime
} // namespace iox
