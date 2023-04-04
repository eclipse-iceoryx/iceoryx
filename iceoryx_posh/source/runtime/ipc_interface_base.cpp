// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_dust/cxx/convert.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iox/logging.hpp"

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
    return cxx::convert::toString(static_cast<std::underlying_type<IpcMessageType>::type>(msg));
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
    return cxx::convert::toString(static_cast<std::underlying_type<IpcMessageErrorType>::type>(msg));
}

template <typename IpcChannelType>
IpcInterface<IpcChannelType>::IpcInterface(const RuntimeName_t& runtimeName,
                                           const uint64_t maxMessages,
                                           const uint64_t messageSize) noexcept
    : m_runtimeName(runtimeName)
{
    m_maxMessages = maxMessages;
    m_maxMessageSize = messageSize;
    if (m_maxMessageSize > platform::IoxIpcChannelType::MAX_MESSAGE_SIZE)
    {
        IOX_LOG(WARN) << "Message size too large, reducing from " << messageSize << " to "
                      << platform::IoxIpcChannelType::MAX_MESSAGE_SIZE;
        m_maxMessageSize = platform::IoxIpcChannelType::MAX_MESSAGE_SIZE;
    }
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::receive(IpcMessage& answer) const noexcept
{
    auto message = m_ipcChannel.receive();
    if (message.has_error())
    {
        return false;
    }

    return IpcInterface<IpcChannelType>::setMessageFromString(message.value().c_str(), answer);
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::timedReceive(const units::Duration timeout, IpcMessage& answer) const noexcept
{
    return !m_ipcChannel.timedReceive(timeout)
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
        IOX_LOG(ERROR) << "The received message " << answer.getMessage() << " is not valid";
        return false;
    }
    return true;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::send(const IpcMessage& msg) const noexcept
{
    if (!msg.isValid())
    {
        IOX_LOG(ERROR) << "Trying to send the message " << msg.getMessage() << " which "
                       << "does not follow the specified syntax.";
        return false;
    }

    auto logLengthError = [&msg](posix::IpcChannelError& error) {
        if (error == posix::IpcChannelError::MESSAGE_TOO_LONG)
        {
            const uint64_t messageSize = msg.getMessage().size() + platform::IoxIpcChannelType::NULL_TERMINATOR_SIZE;
            IOX_LOG(ERROR) << "msg size of " << messageSize << " bigger than configured max message size";
        }
    };
    return !m_ipcChannel.send(msg.getMessage()).or_else(logLengthError).has_error();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::timedSend(const IpcMessage& msg, units::Duration timeout) const noexcept
{
    if (!msg.isValid())
    {
        IOX_LOG(ERROR) << "Trying to send the message " << msg.getMessage() << " which "
                       << "does not follow the specified syntax.";
        return false;
    }

    auto logLengthError = [&msg](posix::IpcChannelError& error) {
        if (error == posix::IpcChannelError::MESSAGE_TOO_LONG)
        {
            const uint64_t messageSize = msg.getMessage().size() + platform::IoxIpcChannelType::NULL_TERMINATOR_SIZE;
            IOX_LOG(ERROR) << "msg size of " << messageSize << " bigger than configured max message size";
        }
    };
    return !m_ipcChannel.timedSend(msg.getMessage(), timeout).or_else(logLengthError).has_error();
}

template <typename IpcChannelType>
const RuntimeName_t& IpcInterface<IpcChannelType>::getRuntimeName() const noexcept
{
    return m_runtimeName;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::isInitialized() const noexcept
{
    return m_ipcChannel.isInitialized();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::openIpcChannel(const posix::IpcChannelSide channelSide) noexcept
{
    m_channelSide = channelSide;
    IpcChannelType::create(m_runtimeName, m_channelSide, m_maxMessageSize, m_maxMessages)
        .and_then([this](auto& ipcChannel) { this->m_ipcChannel = std::move(ipcChannel); })
        .or_else([](auto& err) {
            IOX_LOG(ERROR) << "unable to create ipc channel with error code: " << static_cast<uint8_t>(err);
        });

    return m_ipcChannel.isInitialized();
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::reopen() noexcept
{
    return openIpcChannel(m_channelSide);
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::ipcChannelMapsToFile() noexcept
{
    return !m_ipcChannel.isOutdated().value_or(true);
}

template <>
bool IpcInterface<posix::UnixDomainSocket>::ipcChannelMapsToFile() noexcept
{
    return true;
}

template <>
bool IpcInterface<posix::NamedPipe>::ipcChannelMapsToFile() noexcept
{
    return true;
}

template <typename IpcChannelType>
bool IpcInterface<IpcChannelType>::hasClosableIpcChannel() const noexcept
{
    return m_ipcChannel.isInitialized();
}

template <typename IpcChannelType>
void IpcInterface<IpcChannelType>::cleanupOutdatedIpcChannel(const RuntimeName_t& name) noexcept
{
    if (platform::IoxIpcChannelType::unlinkIfExists(name).value_or(false))
    {
        IOX_LOG(WARN) << "IPC channel still there, doing an unlink of " << name;
    }
}

template class IpcInterface<posix::UnixDomainSocket>;
template class IpcInterface<posix::NamedPipe>;
template class IpcInterface<posix::MessageQueue>;

} // namespace runtime
} // namespace iox
