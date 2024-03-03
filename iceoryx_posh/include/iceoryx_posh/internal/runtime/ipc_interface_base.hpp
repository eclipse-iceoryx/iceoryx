// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#ifndef IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP
#define IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP

#include "iceoryx_platform/errno.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/time.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/duration.hpp"
#include "iox/optional.hpp"
#include "iox/relative_pointer.hpp"

#include "iox/message_queue.hpp"
#include "iox/named_pipe.hpp"
#include "iox/unix_domain_socket.hpp"

#include <cstdint>
#include <cstdlib>
#include <string>

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
#include <process.h>
#endif


namespace iox
{
namespace platform
{
#if defined(_WIN32)
using IoxIpcChannelType = iox::NamedPipe;
#elif defined(__FREERTOS__)
using IoxIpcChannelType = iox::NamedPipe;
#else
using IoxIpcChannelType = iox::UnixDomainSocket;
#endif
} // namespace platform
namespace runtime
{
enum class IpcMessageType : int32_t
{
    BEGIN = -1,
    NOTYPE = 0,
    REG, // register app
    REG_ACK,
    CREATE_PUBLISHER,
    CREATE_PUBLISHER_ACK,
    CREATE_SUBSCRIBER,
    CREATE_SUBSCRIBER_ACK,
    CREATE_CLIENT,
    CREATE_CLIENT_ACK,
    CREATE_SERVER,
    CREATE_SERVER_ACK,
    CREATE_INTERFACE,
    CREATE_INTERFACE_ACK,
    CREATE_CONDITION_VARIABLE,
    CREATE_CONDITION_VARIABLE_ACK,
    TERMINATION,
    TERMINATION_ACK,
    PREPARE_APP_TERMINATION,
    PREPARE_APP_TERMINATION_ACK,
    ERROR,
    APP_WAIT,
    WAKEUP_TRIGGER,
    REPLAY,
    MESSAGE_NOT_SUPPORTED,
    // etc..
    END,
};

/// If IpcMessageType::ERROR, this is the sub type for details about the error
enum class IpcMessageErrorType : int32_t
{
    BEGIN,
    NOTYPE,
    /// A publisher could not be created unique
    NO_UNIQUE_CREATED,
    INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN,
    REQUEST_PUBLISHER_INVALID_RESPONSE,
    REQUEST_PUBLISHER_WRONG_IPC_MESSAGE_RESPONSE,
    REQUEST_PUBLISHER_NO_WRITABLE_SHM_SEGMENT,
    REQUEST_SUBSCRIBER_INVALID_RESPONSE,
    REQUEST_SUBSCRIBER_WRONG_IPC_MESSAGE_RESPONSE,
    REQUEST_CLIENT_INVALID_RESPONSE,
    REQUEST_CLIENT_WRONG_IPC_MESSAGE_RESPONSE,
    REQUEST_CLIENT_NO_WRITABLE_SHM_SEGMENT,
    REQUEST_SERVER_INVALID_RESPONSE,
    REQUEST_SERVER_WRONG_IPC_MESSAGE_RESPONSE,
    REQUEST_SERVER_NO_WRITABLE_SHM_SEGMENT,
    REQUEST_CONDITION_VARIABLE_INVALID_RESPONSE,
    REQUEST_CONDITION_VARIABLE_WRONG_IPC_MESSAGE_RESPONSE,
    PUBLISHER_LIST_FULL,
    SUBSCRIBER_LIST_FULL,
    CLIENT_LIST_FULL,
    SERVER_LIST_FULL,
    CONDITION_VARIABLE_LIST_FULL,
    EVENT_VARIABLE_LIST_FULL,
    NODE_DATA_LIST_FULL,
    SEGMENT_ID_CONVERSION_FAILURE,
    OFFSET_CONVERSION_FAILURE,
    END,
};


/// @brief Converts a string to the message type enumeration
/// @param[in] str string to convert
IpcMessageType stringToIpcMessageType(const char* str) noexcept;

/// @brief Converts a message type enumeration value into a string
/// @param[in] msg enum value to convert
std::string IpcMessageTypeToString(const IpcMessageType msg) noexcept;

/// @brief Converts a string to the message error type enumeration
/// @param[in] str string to convert
IpcMessageErrorType stringToIpcMessageErrorType(const char* str) noexcept;
/// @brief Converts a message error type enumeration value into a string
/// @param[in] msg enum value to convert
std::string IpcMessageErrorTypeToString(const IpcMessageErrorType msg) noexcept;

using InterfaceName_t = string<MAX_IPC_CHANNEL_NAME_LENGTH>;
/// @brief Transforms an IPC channel name to a prefixed interface name
/// @param[in] channelName the name of the channel without the 'iox1_#_' prefix
/// @param[in] domainId to tie the interface to
/// @param[in] resourceType to be used for the resource prefix
/// @return the interface name with the 'iox1_#_' prefix
InterfaceName_t ipcChannelNameToInterfaceName(RuntimeName_t channelName, DomainId domainId, ResourceType resourceType);

class IpcInterfaceUser;
class IpcInterfaceCreator;

/// @brief Class should never be used by the end-user.
///     Handles the common properties and methods for the IpcChannelType. The handling of
///     the IPC channels must be done by the children.
/// @tparam IpcChannelType the type of ipc channel, supported types are MessageQueue, NamedPipe and UnixDomainSocket
/// @note This class won't uniquely identify if another object is using the same IPC channel
template <typename IpcChannelType>
class IpcInterface
{
  public:
    static constexpr uint64_t MAX_MESSAGE_SIZE = IpcChannelType::MAX_MESSAGE_SIZE;

    virtual ~IpcInterface() noexcept = default;

    /// @brief Receives a message from the IPC channel and stores it in
    ///         answer.
    /// @param[out] answer If a message is received it is stored there.
    /// @return If the call failed or an invalid message was
    ///             received it returns false, otherwise true.
    bool receive(IpcMessage& answer) const noexcept;

    /// @brief Tries to receive a message from the IPC channel within a
    ///         specified timeout. It stores the message in answer.
    /// @param[in] timeout for receiving a message.
    /// @param[in] answer The answer of the IPC channel. If timedReceive
    ///         failed the content of answer is undefined.
    /// @return If a valid message was received before the timeout occures
    ///             it returns true, otherwise false.
    ///         It also returns false if clock_gettime() failed
    bool timedReceive(const units::Duration timeout, IpcMessage& answer) const noexcept;

    /// @brief Tries to send the message specified in msg.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @return If a valid message was send it returns true,
    ///             otherwise if the message was invalid it will return false.
    bool send(const IpcMessage& msg) const noexcept;

    /// @brief Tries to send the message specified in msg to the message
    ///        queue within a specified timeout.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @param[in] timeout specifies the duration to wait for sending.
    /// @return If a valid message was send it returns true,
    ///             otherwise if the message was invalid it will return false.
    bool timedSend(const IpcMessage& msg, const units::Duration timeout) const noexcept;

    /// @brief Returns the interface name, the unique char string which
    ///         explicitly identifies the IPC channel.
    /// @return name of the IPC channel
    const RuntimeName_t& getRuntimeName() const noexcept;

    /// @brief If the IPC channel could not be opened or linked in the
    ///         constructor it will return false, otherwise true. This is
    ///         needed since the constructor is not allowed to throw an
    ///         exception.
    ///         You should always check a IPC channel with isInitialized
    ///         before using it, since all other methods will fail and
    ///         return false if a message could not be successfully
    ///         initialized.
    /// @return initialization state
    bool isInitialized() const noexcept;

    /// @brief Since there might be an outdated IPC channel due to an unclean temination
    ///        this function closes the IPC channel if it's existing.
    /// @param[in] name of the IPC channel to clean up
    static void cleanupOutdatedIpcChannel(const InterfaceName_t& name) noexcept;

    friend class IpcInterfaceUser;
    friend class IpcInterfaceCreator;
    friend class IpcRuntimeInterface;

  protected:
    /// @brief Closes and opens an existing IPC channel using the same parameters as before.
    ///        If the queue was not open, it is just openened.
    /// @return true if successfully reopened, false if not
    bool reopen() noexcept;

    /// @brief Checks if the IPC channel has its counterpart in the file system
    /// @return If the IPC channel, which corresponds to a descriptor,
    ///         is still availabe in the file system it returns true,
    ///         otherwise it was deleted or the IPC channel was not open and returns false
    bool ipcChannelMapsToFile() noexcept;

    /// @brief The default constructor is explicitly deleted since every
    ///         IPC channel needs a unique string to be identified with.
    IpcInterface() = delete;

    IpcInterface(const RuntimeName_t& runtimeName,
                 const DomainId domainId,
                 const ResourceType resourceType,
                 const uint64_t maxMessages,
                 const uint64_t messageSize) noexcept;

    IpcInterface(IpcInterface&&) noexcept = default;
    IpcInterface& operator=(IpcInterface&&) noexcept = default;

    /// @brief delete unneeded ctors and assignment operators
    IpcInterface(const IpcInterface&) = delete;
    IpcInterface& operator=(const IpcInterface&) = delete;

    /// @brief Set the content of answer from buffer.
    /// @param[in] buffer Raw message as char pointer
    /// @param[out] answer Raw message is setting this IpcMessage
    /// @return answer.isValid()
    static bool setMessageFromString(const char* buffer, IpcMessage& answer) noexcept;

    /// @brief Opens a IPC channel and default permissions
    ///         stored in m_perms and stores the descriptor
    /// @param[in] channelSide of the queue. SERVER will also destroy the IPC channel in the dTor, while CLIENT
    /// keeps the IPC channel in the file system after the dTor is called
    /// @return Returns true if a IPC channel could be opened, otherwise
    ///             false.
    bool openIpcChannel(const PosixIpcChannelSide channelSide) noexcept;

    /// @brief If a IPC channel was moved then m_runtimeName was cleared
    ///         and this object gave up the control of that specific
    ///         IPC channel and therefore shouldnt unlink or close it.
    ///         Otherwise the object which it was moved to can end up with
    ///         an invalid IPC channel descriptor.
    /// @return Returns true if the IPC channel is closable,
    ///             otherwise false.
    bool hasClosableIpcChannel() const noexcept;

  protected:
    InterfaceName_t m_interfaceName;
    RuntimeName_t m_runtimeName;
    uint64_t m_maxMessageSize{0U};
    uint64_t m_maxMessages{0U};
    PosixIpcChannelSide m_channelSide{PosixIpcChannelSide::CLIENT};
    optional<IpcChannelType> m_ipcChannel;
};

using IpcInterfaceBase = IpcInterface<platform::IoxIpcChannelType>;

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP
