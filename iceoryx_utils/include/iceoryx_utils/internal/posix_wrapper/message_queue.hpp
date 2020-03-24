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
#include "iceoryx_utils/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/mqueue.hpp"
#include "iceoryx_utils/platform/stat.hpp"


#include <iostream>

namespace iox
{
namespace posix
{
/// @brief Wrapper class for posix message queue
///
/// @tparam NON_BLOCKING specifies the type of message queue. A non-blocking message queue will immediately return from
/// a send/receive call if the queue is full/empty. A blocking message has member functions timedSend and timedReceive
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
class MessageQueue : public DesignPattern::Creation<MessageQueue, IpcChannelError>
{
  public:
    static constexpr mqd_t INVALID_DESCRIPTOR = -1;
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr size_t SHORTEST_VALID_QUEUE_NAME = 2;
    static constexpr size_t NULL_TERMINATOR_SIZE = 1;
    static constexpr size_t MAX_MESSAGE_SIZE = 4096;

    /// for calling private constructor in create method
    friend class DesignPattern::Creation<MessageQueue, IpcChannelError>;

    /// default constructor. The result is an invalid MessageQueue object which can be reassigned later by using the
    /// move constructor.
    MessageQueue();

    MessageQueue(const MessageQueue& other) = delete;
    MessageQueue(MessageQueue&& other);
    MessageQueue& operator=(const MessageQueue& other) = delete;
    MessageQueue& operator=(MessageQueue&& other);

    ~MessageQueue();

    static cxx::expected<bool, IpcChannelError> unlinkIfExists(const std::string& name);

    /// close and remove message queue.
    cxx::expected<IpcChannelError> destroy();

    /// @brief send a message to queue using std::string.
    /// @return true if sent without errors, false otherwise
    cxx::expected<IpcChannelError> send(const std::string& msg) const;

    /// @todo zero copy receive with receive(cxx::string&); cxx::string would be the buffer for mq_receive

    /// @brief receive message from queue using std::string.
    /// @return number of characters received. In case of an error, returns -1 and msg is empty.
    cxx::expected<std::string, IpcChannelError> receive() const;

    /// @brief try to receive message from queue for a given timeout duration using std::string. Only defined
    /// for NON_BLOCKING == false.
    /// @return optional containing the received string. In case of an error, nullopt type is returned.
    cxx::expected<std::string, IpcChannelError> timedReceive(const units::Duration& timeout) const;

    /// @brief try to send a message to the queue for a given timeout duration using std::string
    cxx::expected<IpcChannelError> timedSend(const std::string& msg, const units::Duration& timeout) const;

    cxx::expected<bool, IpcChannelError> isOutdated();

  private:
    MessageQueue(const std::string& name,
                 const IpcChannelMode mode,
                 const IpcChannelSide channelSide,
                 const size_t maxMsgSize = MAX_MESSAGE_SIZE,
                 const uint64_t maxMsgNumber = 10u);
    cxx::expected<int32_t, IpcChannelError>
    open(const std::string& name, const IpcChannelMode mode, const IpcChannelSide channelSide);

    cxx::expected<IpcChannelError> close();
    cxx::expected<IpcChannelError> unlink();
    cxx::error<IpcChannelError> createErrorFromErrnum(const int errnum) const;
    static cxx::error<IpcChannelError> createErrorFromErrnum(const std::string& name, const int errnum);

  private:
    std::string m_name;
    struct mq_attr m_attributes;
    mqd_t m_mqDescriptor = INVALID_DESCRIPTOR;
    IpcChannelSide m_channelSide;

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
