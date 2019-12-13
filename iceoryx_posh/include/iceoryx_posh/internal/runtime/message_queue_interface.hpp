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

#pragma once

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
#include <process.h>
#endif

namespace iox
{
namespace runtime
{
enum class MqMessageType : int32_t
{
    BEGIN = -1,
    NOTYPE = 0,
    REG, // register app
    REG_ACK,
    IMPL_SENDER,
    IMPL_SENDER_ACK,
    IMPL_RECEIVER,
    IMPL_RECEIVER_ACK,
    IMPL_INTERFACE,
    IMPL_INTERFACE_ACK,
    IMPL_APPLICATION,
    IMPL_APPLICATION_ACK,
    CREATE_RUNNABLE,
    CREATE_RUNNABLE_ACK,
    REMOVE_RUNNABLE,
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


/// @brief Converts a string to the message type enumeration
/// @param[in] str string to convert
MqMessageType stringToMqMessageType(const char* str) noexcept;

/// @brief Converts a message type enumeration value into a string
/// @param[in] msg enum value to convert
std::string mqMessageTypeToString(const MqMessageType msg) noexcept;

class MqInterfaceUser;
class MqInterfaceCreator;

/// @brief Base-Class should never be used by the end-user.
///     Handles the common properties and methods for the childs but does
///     not call mq_open, mq_close, mq_link or mq_unlink. The handling of
///     the message queues must be done by the children.
class MqBase
{
  public:
    /// @brief Receives a message from the message queue and stores it in
    ///         answer.
    /// @param[out] answer If a message is received it is stored there.
    /// @return If the call to mq_received failed or an invalid message was
    ///             received it returns false, otherwise true.
    ///         One cause of a failed mq_received call can be a closed or
    ///             unlinked message queue
    bool receive(MqMessage& answer) const noexcept;

    /// @brief Tries to receive a message from the message queue within a
    ///         specified timeout. It stores the message in answer.
    /// @param[in] timeout_ms Timeout in milliseconds.
    /// @param[in] answer The answer of the message queue. If timedReceive
    ///         failed the content of answer is undefined.
    /// @return If a valid message was received before the timeout occures
    ///             it returns true, otherwise false.
    ///         It also returns false if clock_gettime() failed
    bool timedReceive(const uint32_t timeout_ms, MqMessage& answer) const noexcept;

    /// @brief Tries to send the message specified in msg.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @return If a valid message was send via mq_send it returns true,
    ///             otherwise if the message was invalid or mq_send returned
    ///             an error, it will return false.
    bool send(const MqMessage& msg) const noexcept;

    /// @brief Tries to send the message specified in msg to the message
    ///        queue within a specified timeout.
    /// @param[in] msg Must be a valid message, if its an invalid message
    ///                 send will return false
    /// @param[in] timeout specifies the duration to wait for sending.
    /// @return If a valid message was send via mq_send it returns true,
    ///             otherwise if the message was invalid or mq_send returned
    ///             an error, it will return false.
    bool timedSend(const MqMessage& msg, const units::Duration timeout) const noexcept;

    /// @brief Returns the interface name, the unique char string which
    ///         explicitly identifies the message queue.
    /// @return name of the message queue
    const std::string& getInterfaceName() const noexcept;

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
    static void cleanupOutdatedMessageQueue(const std::string& name) noexcept;

    friend class MqInterfaceUser;
    friend class MqInterfaceCreator;
    friend class MqRuntimeInterface;

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
    MqBase() = delete;
    // TODO: unique identifier problem, multiple MqBase objects with the
    //        same InterfaceName are using the same message queue
    MqBase(const std::string& InterfaceName, const long maxMessages, const long messageSize) noexcept;

    MqBase(const MqBase&) = default;
    MqBase(MqBase&&) = default;
    virtual ~MqBase() = default;

    MqBase& operator=(const MqBase&) = default;
    MqBase& operator=(MqBase&&) = default;

    /// @brief Set the content of answer from buffer.
    /// @param[in] buffer Raw message as char pointer
    /// @param[out] answer Raw message is setting this MqMessage
    /// @return answer.isValid()
    static bool setMessageFromString(const char* buffer, MqMessage& answer) noexcept;

    /// @brief Opens a message queue with mq_open and default permissions
    ///         stored in m_perms and stores the descriptor in m_roudiMq
    /// @param[in] name Unique identifier of the message queue
    /// @param[in] oflag Flags that control the operation of the call.
    ///                 They are defined in fcntl.h
    /// @return Returns true if a message queue could be opened, otherwise
    ///             false.
    bool openMessageQueue(const std::string& name, const int oflag) noexcept;

    /// @brief Closes a message queue with mq_close
    /// @return Returns true if the message queue could be closed, otherwise
    ///             false.
    bool closeMessageQueue() const noexcept;

    /// @brief Unlinks a message queue with mq_unlink
    /// @return Returns true if the message queue could be unlinked,
    ///             otherwise false.
    bool unlinkMessageQueue() const noexcept;

    /// @brief If a message queue was moved then m_interfaceName was cleared
    ///         and this object gave up the control of that specific
    ///         message queue and therefore shouldnt unlink or close it.
    ///         Otherwise the object which it was moved to can end up with
    ///         an invalid message queue descriptor.
    /// @return Returns true if the message queue is closable,
    ///             otherwise false.
    bool hasClosableMessageQueue() const noexcept;

  protected:
    static constexpr long MQ_FLAGS = 0;    // ignored by mq_open
    static constexpr long MQ_CUR_MSGS = 0; // ignored by mq_open
    static constexpr long MAX_MESSAGE_LENGTH = 4096;
    static constexpr int NULL_TERMINATOR_SIZE = 1;
    /// @todo in QNX there are two addtional fields, mq_sendwait and mq_recvwait
    /// mq_sendwait: number of processes waiting to send
    /// mq_recvwait: number of processes waiting to receive

    bool m_isInitialized{true};

    // dont initialize m_attr with actual values here,
    // fields have a different order in QNX,
    // so we need to initialize by name (see ctor of MqBase)
    struct mq_attr m_attr; // = {};  removed since they are initialized explicitly in MqBase (see above)
    int m_perms{S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH};

    int m_oflag{O_RDONLY};
    mqd_t m_roudiMq{-1};
    std::string m_interfaceName;
};

/// @brief Class for handling a message queue via mq_open and mq_close.
class MqInterfaceUser : public MqBase
{
  public:
    /// @brief Constructs a MqInterfaceUser and opens a message queue with
    ///         mq_open. If mq_open fails the method isInitialized returns
    ///         false. Therefore, isInitialized should always be called
    ///         before using this class.
    /// @param[in] name Unique identifier of the message queue which
    ///         is used for mq_open
    /// @param[in] maxMessages maximum number of queued messages
    /// @param[in] message size maximum message size
    MqInterfaceUser(const std::string& name,
                    const long maxMessages = APP_MAX_MESSAGES,
                    const long messageSize = APP_MESSAGE_SIZE) noexcept;

    /// @brief The copy constructor and assignment operator are deleted since
    ///         this class manages a resource (message queue) which cannot
    ///         be copied. Theoretically, we can create a new message queue
    ///         but since every message queue needs a unique string
    ///         identifier which is undefined when creating a copy, this
    ///         makes no sense.
    MqInterfaceUser(const MqInterfaceUser&) = delete;
    MqInterfaceUser& operator=(const MqInterfaceUser&) = delete;

    /// @brief Since this object manages a system resource (message queue)
    ///         only the move constructor and assignment operator is
    ///         defined.
    MqInterfaceUser(MqInterfaceUser&&) noexcept;
    MqInterfaceUser& operator=(MqInterfaceUser&&) noexcept;

    ~MqInterfaceUser() noexcept;
};

/// @brief Class for handling a message queue via mq_open and mq_unlink
///             and mq_close
class MqInterfaceCreator : public MqBase
{
  public:
    /// @brief Constructs a MqInterfaceCreator and opens a new message
    ///         queue with mq_open. If mq_open fails it tries to mq_unlink
    ///         the message queue and tries mq_open again. If it still fails
    ///         isInitialized will return false. Therefore, isInitialized
    ///         should always be called before using this class.
    /// @param[in] name Unique identifier of the message queue which is
    ///         is used for mq_open
    /// @param[in] maxMessages maximum number of queued messages
    /// @param[in] message size maximum message size
    MqInterfaceCreator(const std::string& name,
                       const long maxMessages = ROUDI_MAX_MESSAGES,
                       const long messageSize = ROUDI_MESSAGE_SIZE) noexcept;

    /// @brief The copy constructor and assignment operator is deleted since
    ///         this class manages a resource (message queue) which cannot
    ///         be copied. Theoretically, we can create a new message queue
    ///         but since every message queue needs a unique string
    ///         identifier which is undefined when creating a copy, this
    ///         makes no sense.
    MqInterfaceCreator(const MqInterfaceCreator&) = delete;
    MqInterfaceCreator& operator=(const MqInterfaceCreator&) = delete;

    /// @brief Since this object manages a system resource (message queue)
    ///         only the move constructor and assignment operator is
    ///         defined.
    MqInterfaceCreator(MqInterfaceCreator&&) noexcept;
    MqInterfaceCreator& operator=(MqInterfaceCreator&&) noexcept;

    ~MqInterfaceCreator() noexcept;
};

class MqRuntimeInterface
{
  public:
    /// @brief Runtime Interface for the own message queue and the one to the RouDi daemon
    /// @param[in] roudiName name of the RouDi message queue
    /// @param[in] appName name of the appplication and its message queue
    /// @param[in] roudiWaitingTimeout timeout for searching the RouDi message queue
    MqRuntimeInterface(const std::string& roudiName,
                       const std::string& appName,
                       const units::Duration roudiWaitingTimeout) noexcept;

    MqRuntimeInterface(const MqRuntimeInterface&) = delete;
    MqRuntimeInterface& operator=(const MqRuntimeInterface&) = delete;
    MqRuntimeInterface(MqRuntimeInterface&&) = default;
    MqRuntimeInterface& operator=(MqRuntimeInterface&&) = default;
    ~MqRuntimeInterface() = default;

    /// @brief sends the keep alive trigger to the RouDi daemon
    /// @return true if sending was successful, false if not
    bool sendKeepalive() noexcept;

    /// @brief send a request to the RouDi daemon
    /// @param[in] msg request to RouDi
    /// @param[out] answer response from RouDi
    /// @return true if communication was successful, false if not
    bool sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept;

    /// @brief send a message to the RouDi daemon
    /// @param[in] msg message which will be send to RouDi
    /// @return true if communication was successful, otherwise false
    bool sendMessageToRouDi(const MqMessage& msg) noexcept;

    /// @brief get the base address of the management shared memory segment
    /// @return address as string
    std::string getShmBaseAddr() const noexcept;

    /// @brief get the adress of the segment manager
    /// @return address as string
    std::string getSegmentManagerAddr() const noexcept;

    /// @brief get the size of the management shared memory object
    /// @return size in bytes
    size_t getShmTopicSize() noexcept;

    /// @brief get the segment id of the shared memory object
    /// @return segment id
    uint64_t getSegmentId() const noexcept;

  private:
    enum class RegAckResult
    {
        SUCCESS,
        TIMEOUT
    };

    /// @brief
    /// @return
    void waitForRoudi(posix::Timer& timer) noexcept;

    /// @brief
    /// @return
    RegAckResult waitForRegAck(int64_t transmissionTimestamp) noexcept;

  private:
    std::string m_appName;
    std::string m_shmBaseAddr;
    std::string m_segmentManager;
    MqInterfaceCreator m_AppMqInterface;
    MqInterfaceUser m_RoudiMqInterface;
    size_t m_shmTopicSize{0};
    uint64_t m_segmentId{0};
};
} // namespace runtime
} // namespace iox
