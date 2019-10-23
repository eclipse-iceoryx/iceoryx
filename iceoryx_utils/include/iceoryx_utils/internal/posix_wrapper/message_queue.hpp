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

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <fcntl.h>
#include <iostream>
#include <mqueue.h>
#include <sys/stat.h>

namespace iox
{
namespace posix
{
enum class MessageQueueError
{
    MqNotInitialized,
    AccessDenied,
    NoSuchMessageQueue,
    InternalLogicError,
    MessageQueueAlreadyExists,
    InvalidArguments,
    MessageTooLong,
    MessageQueueIsFull,
    InvalidMessageQueueName,
    Timeout,
    Undefined
};

enum class MessageQueueMode
{
    NonBlocking,
    Blocking
};

enum class MessageQueueOwnership
{
    OpenExisting,
    CreateNew
};

/// @brief Wrapper class for posix message queue
///
/// @tparam NonBlocking specifies the type of message queue. A non-blocking message queue will immediately return from a
/// send/receive call if the queue is full/empty. A blocking message has member functions timedSend and timedReceive
/// which allow to specify a maximum timeout duration.
/// @code
///     auto mq = posix::MessageQueue<true>::CreateMessageQueue("/MqName123");
///     if (mq.has_value())
///     {
///         mq->send("important message, bla.");
///         // ...
///         std::string str;
///         mq->receive(str);
///     }
/// @endcode
class MessageQueue : public DesignPattern::Creation<MessageQueue, MessageQueueError>
{
  public:
    static constexpr mqd_t InvalidDescriptor = -1;
    static constexpr int32_t ErrorCode = -1;
    static constexpr size_t MaxMsgSize = 512;
    static constexpr int64_t MaxMsgNumber = 10;

    /// for calling private constructor in create method
    friend class DesignPattern::Creation<MessageQueue, MessageQueueError>;

    /// default constructor. The result is an invalid MessageQueue object which can be reassigned later by using the
    /// move constructor.
    MessageQueue();

    MessageQueue(const MessageQueue& other) = delete;
    MessageQueue(MessageQueue&& other);
    MessageQueue& operator=(const MessageQueue& other) = delete;
    MessageQueue& operator=(MessageQueue&& other);

    ~MessageQueue();

    /// close and remove message queue.
    cxx::expected<MessageQueueError> destroy();

    /// @brief send a message to queue using std::string.
    /// @return true if sent without errors, false otherwise
    cxx::expected<MessageQueueError> send(const std::string& f_msg);

    /// @brief receive message from queue using std::string.
    /// @return number of characters received. In case of an error, returns -1 and f_msg is empty.
    cxx::expected<std::string, MessageQueueError> receive();

    /// @brief try to receive message from queue for a given timeout duration using std::string. Only defined
    /// for NonBlocking == false.
    /// @return optional containing the received string. In case of an error, nullopt type is returned.
    cxx::expected<std::string, MessageQueueError> timedReceive(const units::Duration& f_timeout);

    /// @brief try to send a message to the queue for a given timeout duration using std::string
    cxx::expected<MessageQueueError> timedSend(const std::string& f_msg, const units::Duration& f_timeout);

  private:
    MessageQueue(const std::string& f_name, const MessageQueueMode f_mode, const MessageQueueOwnership f_ownerShip);
    cxx::expected<int32_t, MessageQueueError>
    open(const std::string& f_name, const MessageQueueMode f_mode, const MessageQueueOwnership f_ownerShip);

    cxx::expected<MessageQueueError> close();
    cxx::expected<MessageQueueError> unlink();
    cxx::error<MessageQueueError> createErrorFromErrnum(const int errnum);

  private:
    std::string m_name;
    struct mq_attr m_attributes;
    mqd_t m_mqDescriptor = InvalidDescriptor;
    MessageQueueOwnership m_ownerShip;

#ifdef __QNX__
    static constexpr int TIMEOUT_ERRNO = EINTR;
#else
    static constexpr int TIMEOUT_ERRNO = ETIMEDOUT;
#endif
    // read/write permissions
    static constexpr mode_t m_filemode{S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH};
};
} // namespace posix
} // namespace iox

