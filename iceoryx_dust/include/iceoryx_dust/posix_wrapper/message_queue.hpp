// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_DUST_POSIX_WRAPPER_MESSAGE_QUEUE_HPP
#define IOX_DUST_POSIX_WRAPPER_MESSAGE_QUEUE_HPP

#include "iceoryx_dust/design/creation.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mqueue.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iox/duration.hpp"
#include "iox/optional.hpp"

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
    static constexpr mqd_t INVALID_DESCRIPTOR = std::numeric_limits<mqd_t>::max();
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr uint64_t SHORTEST_VALID_QUEUE_NAME = 2;
    static constexpr uint64_t NULL_TERMINATOR_SIZE = 1;
    static constexpr uint64_t MAX_MESSAGE_SIZE = 4096;

    /// for calling private constructor in create method
    friend class DesignPattern::Creation<MessageQueue, IpcChannelError>;

    /// default constructor. The result is an invalid MessageQueue object which can be reassigned later by using the
    /// move constructor.
    MessageQueue() noexcept;

    MessageQueue(const MessageQueue& other) = delete;
    MessageQueue(MessageQueue&& other) noexcept;
    MessageQueue& operator=(const MessageQueue& other) = delete;
    MessageQueue& operator=(MessageQueue&& other) noexcept;

    ~MessageQueue() noexcept;

    static expected<bool, IpcChannelError> unlinkIfExists(const IpcChannelName_t& name) noexcept;

    /// @brief send a message to queue using std::string.
    /// @return true if sent without errors, false otherwise
    expected<void, IpcChannelError> send(const std::string& msg) const noexcept;

    /// @todo iox-#1693 zero copy receive with receive(iox::string&); iox::string would be the buffer for mq_receive

    /// @brief receive message from queue using std::string.
    /// @return number of characters received. In case of an error, returns -1 and msg is empty.
    expected<std::string, IpcChannelError> receive() const noexcept;

    /// @brief try to receive message from queue for a given timeout duration using std::string. Only defined
    /// for NON_BLOCKING == false.
    /// @return optional containing the received string. In case of an error, nullopt type is returned.
    expected<std::string, IpcChannelError> timedReceive(const units::Duration& timeout) const noexcept;

    /// @brief try to send a message to the queue for a given timeout duration using std::string
    expected<void, IpcChannelError> timedSend(const std::string& msg, const units::Duration& timeout) const noexcept;

    static expected<bool, IpcChannelError> isOutdated() noexcept;

  private:
    MessageQueue(const IpcChannelName_t& name,
                 const IpcChannelSide channelSide,
                 const size_t maxMsgSize = MAX_MESSAGE_SIZE,
                 const uint64_t maxMsgNumber = 10U) noexcept;

    expected<mqd_t, IpcChannelError> open(const IpcChannelName_t& name, const IpcChannelSide channelSide) noexcept;

    expected<void, IpcChannelError> close() noexcept;
    expected<void, IpcChannelError> unlink() noexcept;
    IpcChannelError errnoToEnum(const int32_t errnum) const noexcept;
    static IpcChannelError errnoToEnum(const IpcChannelName_t& name, const int32_t errnum) noexcept;
    static expected<IpcChannelName_t, IpcChannelError> sanitizeIpcChannelName(const IpcChannelName_t& name) noexcept;
    expected<void, IpcChannelError> destroy() noexcept;

  private:
    IpcChannelName_t m_name;
    mq_attr m_attributes{};
    mqd_t m_mqDescriptor = INVALID_DESCRIPTOR;
    IpcChannelSide m_channelSide = IpcChannelSide::CLIENT;

#ifdef __QNX__
    static constexpr int TIMEOUT_ERRNO = EINTR;
#else
    static constexpr int TIMEOUT_ERRNO = ETIMEDOUT;
#endif
    // read/write permissions
    /// NOLINTJUSTIFICATION used inside the wrapper so that the user does not have to use this
    ///                     construct from outside
    /// NOLINTNEXTLINE(hicpp-signed-bitwise)
    static constexpr mode_t m_filemode{S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH};
};
} // namespace posix
} // namespace iox

#endif // IOX_DUST_POSIX_WRAPPER_MESSAGE_QUEUE_HPP
