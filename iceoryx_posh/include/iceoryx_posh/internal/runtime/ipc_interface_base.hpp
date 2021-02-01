// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP
#define IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_utils/cxx/deadline_timer.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/stat.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <cstdint>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
#include <process.h>
#endif

namespace iox
{
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
    CREATE_INTERFACE,
    CREATE_INTERFACE_ACK,
    CREATE_APPLICATION,
    CREATE_APPLICATION_ACK,
    CREATE_CONDITION_VARIABLE,
    CREATE_CONDITION_VARIABLE_ACK,
    CREATE_NODE,
    CREATE_NODE_ACK,
    FIND_SERVICE,
    KEEPALIVE,
    ERROR,
    APP_WAIT,
    WAKEUP_TRIGGER,
    REPLAY,
    SERVICE_REGISTRY_CHANGE_COUNTER,
    MESSAGE_NOT_SUPPORTED,
    // etc..
    END,
};

/// If IpcMessageType::ERROR, this is the sub type for details about the error
enum class IpcMessageErrorType : int32_t
{
    BEGIN = -1,
    NOTYPE = 0,
    /// A publisher could not be created unique
    NO_UNIQUE_CREATED,
    REQUEST_PUBLISHER_WRONG_MESSAGE_QUEUE_RESPONSE,
    REQUEST_SUBSCRIBER_WRONG_MESSAGE_QUEUE_RESPONSE,
    REQUEST_CONDITION_VARIABLE_WRONG_MESSAGE_QUEUE_RESPONSE,
    PUBLISHER_LIST_FULL,
    SUBSCRIBER_LIST_FULL,
    CONDITION_VARIABLE_LIST_FULL,
    NODE_DATA_LIST_FULL,
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

class IpcInterfaceUser;
class IpcInterfaceCreator;

/// @brief Base-Class should never be used by the end-user.
///     Handles the common properties and methods for the childs but does
///     not call mq_open, mq_close, mq_link or mq_unlink. The handling of
///     the message queues must be done by the children.
class IpcInterfaceBase
{
  public:
    /// @brief Receives a message from the message queue and stores it in
    ///         answer.
    /// @param[out] answer If a message is received it is stored there.
    /// @return If the call to mq_received failed or an invalid message was
    ///             received it returns false, otherwise true.
    ///         One cause of a failed mq_received call can be a closed or
    ///             unlinked message queue
    bool receive(IpcMessage& answer) const noexcept;

    /// @brief Tries to receive a message from the message queue within a
    ///         specified timeout. It stores the message in answer.
    /// @param[in] timeout for receiving a message.
    /// @param[in] answer The answer of the message queue. If timedReceive
    ///         failed the content of answer is undefined.
    /// @return If a valid message was received before the timeout occures
    ///             it returns true, otherwise false.
    ///         It also returns false if clock_gettime() failed
    bool timedReceive(const units::Duration timeout, IpcMessage& answer) const noexcept;

    /// @brief Tries to send the message specified in msg.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @return If a valid message was send via mq_send it returns true,
    ///             otherwise if the message was invalid or mq_send returned
    ///             an error, it will return false.
    bool send(const IpcMessage& msg) const noexcept;

    /// @brief Tries to send the message specified in msg to the message
    ///        queue within a specified timeout.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @param[in] timeout specifies the duration to wait for sending.
    /// @return If a valid message was send via mq_send it returns true,
    ///             otherwise if the message was invalid or mq_send returned
    ///             an error, it will return false.
    bool timedSend(const IpcMessage& msg, const units::Duration timeout) const noexcept;

    /// @brief Returns the interface name, the unique char string which
    ///         explicitly identifies the message queue.
    /// @return name of the message queue
    const ProcessName_t& getInterfaceName() const noexcept;

    /// @brief If the message queue could not be opened or linked in the
    ///         constructor it will return false, otherwise true. This is
    ///         needed since the constructor is not allowed to throw an
    ///         exception.
    ///         You should always check a message queue with isInitialized
    ///         before using it, since all other methods will fail and
    ///         return false if a message could not be successfully
    ///         initialized.
    /// @return initialization state
    bool isInitialized() const noexcept;

    /// @brief Since there might be an outdated message queue due to an unclean temination
    ///        this function closes the message queue if it's existing.
    /// @param[in] name of the message queue to clean up
    static void cleanupOutdatedMessageQueue(const ProcessName_t& name) noexcept;

    friend class IpcInterfaceUser;
    friend class IpcInterfaceCreator;
    friend class IpcRuntimeInterface;

  protected:
    /// @brief Closes and opens an existing message queue using the same parameters as before.
    ///        If the queue was not open, it is just openened.
    /// @return true if successfully reopened, false if not
    bool reopen() noexcept;

    /// @brief Checks if the mqd_t still has its counterpart in the file system
    /// @return If the message queue, which corresponds to the mqd_t descriptor
    ///         is still availabe in the file system it returns true,
    ///         otherwise it was deleted or the mqueue was not open and returns false
    bool mqMapsToFile() noexcept;

    /// @brief The default constructor is explicitly deleted since every
    ///         message queue needs a unique string to be identified with.
    IpcInterfaceBase() = delete;
    // TODO: unique identifier problem, multiple IpcInterfaceBase objects with the
    //        same InterfaceName are using the same message queue
    IpcInterfaceBase(const ProcessName_t& InterfaceName, const uint64_t maxMessages, const uint64_t messageSize) noexcept;
    virtual ~IpcInterfaceBase() = default;

    /// @brief delete copy and move ctor and assignment since they are not needed
    IpcInterfaceBase(const IpcInterfaceBase&) = delete;
    IpcInterfaceBase(IpcInterfaceBase&&) = delete;
    IpcInterfaceBase& operator=(const IpcInterfaceBase&) = delete;
    IpcInterfaceBase& operator=(IpcInterfaceBase&&) = delete;

    /// @brief Set the content of answer from buffer.
    /// @param[in] buffer Raw message as char pointer
    /// @param[out] answer Raw message is setting this IpcMessage
    /// @return answer.isValid()
    static bool setMessageFromString(const char* buffer, IpcMessage& answer) noexcept;

    /// @brief Opens a message queue with mq_open and default permissions
    ///         stored in m_perms and stores the descriptor in m_roudiMq
    /// @param[in] channelSide of the queue. SERVER will also destroy the message queue in the dTor, while CLIENT
    /// keeps the message queue in the file system after the dTor is called
    /// @return Returns true if a message queue could be opened, otherwise
    ///             false.
    bool openMessageQueue(const posix::IpcChannelSide channelSide) noexcept;

    /// @brief Closes a message queue with mq_close
    /// @return Returns true if the message queue could be closed, otherwise
    ///             false.
    bool closeMessageQueue() noexcept;

    /// @brief If a message queue was moved then m_interfaceName was cleared
    ///         and this object gave up the control of that specific
    ///         message queue and therefore shouldnt unlink or close it.
    ///         Otherwise the object which it was moved to can end up with
    ///         an invalid message queue descriptor.
    /// @return Returns true if the message queue is closable,
    ///             otherwise false.
    bool hasClosableMessageQueue() const noexcept;

  protected:
    ProcessName_t m_interfaceName;
    uint64_t m_maxMessageSize{0U};
    uint64_t m_maxMessages{0U};
    iox::posix::IpcChannelSide m_channelSide{posix::IpcChannelSide::CLIENT};
    IpcChannelType m_mq;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_INTERFACE_BASE_HPP
