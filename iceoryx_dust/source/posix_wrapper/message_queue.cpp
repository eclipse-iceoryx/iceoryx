// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iceoryx_dust/posix_wrapper/message_queue.hpp"
#include "iceoryx_dust/cxx/std_string_support.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iox/logging.hpp"

#include <chrono>
#include <string>

namespace iox
{
namespace posix
{
expected<MessageQueue, IpcChannelError> MessageQueueBuilder::create() const noexcept
{
    auto sanitzedNameResult = MessageQueue::sanitizeIpcChannelName(m_name);
    if (sanitzedNameResult.has_error())
    {
        return err(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    auto& sanitizedName = sanitzedNameResult.value();
    IOX_MAYBE_UNUSED std::false_type m_name; // m_name shall not be used anymore but only sanitizedName

    if (m_maxMsgSize > MessageQueue::MAX_MESSAGE_SIZE)
    {
        return err(IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED);
    }

    if (m_channelSide == IpcChannelSide::SERVER)
    {
        posixCall(mq_unlink)(sanitizedName.c_str())
            .failureReturnValue(MessageQueue::ERROR_CODE)
            .ignoreErrnos(ENOENT)
            .evaluate()
            .and_then([&sanitizedName](auto& r) {
                if (r.errnum != ENOENT)
                {
                    IOX_LOG(DEBUG) << "MQ still there, doing an unlink of '" << sanitizedName << "'";
                }
            });
    }

    // fields have a different order in QNX, so we need to initialize by name
    mq_attr attributes;
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = static_cast<decltype(attributes.mq_maxmsg)>(m_maxMsgNumber);
    attributes.mq_msgsize = static_cast<decltype(attributes.mq_msgsize)>(m_maxMsgSize);
    attributes.mq_curmsgs = 0L;
#ifdef __QNX__
    attributes.mq_recvwait = 0L;
    attributes.mq_sendwait = 0L;
#endif

    auto openResult = MessageQueue::open(sanitizedName, attributes, m_channelSide);
    if (openResult.has_error())
    {
        return err(openResult.error());
    }
    const auto mqDescriptor = openResult.value();

    return ok(MessageQueue{std::move(sanitizedName), attributes, mqDescriptor, m_channelSide});
}

MessageQueue::MessageQueue(const IpcChannelName_t& name,
                           const mq_attr attributes,
                           mqd_t mqDescriptor,
                           const IpcChannelSide channelSide) noexcept
    : m_name(name)
    , m_attributes(attributes)
    , m_mqDescriptor(mqDescriptor)
    , m_channelSide(channelSide)
{
}

MessageQueue::MessageQueue(MessageQueue&& other) noexcept
{
    *this = std::move(other);
}

MessageQueue::~MessageQueue() noexcept
{
    if (destroy().has_error())
    {
        IOX_LOG(ERROR) << "unable to cleanup message queue '" << m_name << "' in the destructor";
    }
}

MessageQueue& MessageQueue::operator=(MessageQueue&& other) noexcept
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            IOX_LOG(ERROR) << "unable to cleanup message queue '" << m_name
                           << "' during move operation - resource leaks are possible!";
        }

        m_name = std::move(other.m_name);
        m_attributes = other.m_attributes;
        m_mqDescriptor = other.m_mqDescriptor;
        m_channelSide = other.m_channelSide;
        other.m_mqDescriptor = INVALID_DESCRIPTOR;
    }

    return *this;
}

expected<bool, IpcChannelError> MessageQueue::unlinkIfExists(const IpcChannelName_t& name) noexcept
{
    auto sanitizedIpcChannelName = sanitizeIpcChannelName(name);
    if (sanitizedIpcChannelName.has_error())
    {
        return err(IpcChannelError::INVALID_CHANNEL_NAME);
    }


    auto mqCall = posixCall(mq_unlink)(sanitizedIpcChannelName->c_str())
                      .failureReturnValue(ERROR_CODE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(*sanitizedIpcChannelName, mqCall.error().errnum));
    }
    return ok(mqCall->errnum != ENOENT);
}

expected<void, IpcChannelError> MessageQueue::destroy() noexcept
{
    if (m_mqDescriptor != INVALID_DESCRIPTOR)
    {
        auto closeCall = close();
        if (closeCall.has_error())
        {
            m_mqDescriptor = INVALID_DESCRIPTOR;
            return closeCall;
        }
        auto unlinkCall = unlink();
        if (unlinkCall.has_error())
        {
            m_mqDescriptor = INVALID_DESCRIPTOR;
            return unlinkCall;
        }
    }

    m_mqDescriptor = INVALID_DESCRIPTOR;
    return ok();
}

expected<void, IpcChannelError> MessageQueue::send(const std::string& msg) const noexcept
{
    const uint64_t messageSize = msg.size() + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        return err(IpcChannelError::MESSAGE_TOO_LONG);
    }

    auto mqCall =
        posixCall(mq_send)(m_mqDescriptor, msg.c_str(), messageSize, 1U).failureReturnValue(ERROR_CODE).evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok();
}

expected<std::string, IpcChannelError> MessageQueue::receive() const noexcept
{
    /// NOLINTJUSTIFICATION required as raw memory buffer for mq_receive
    /// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    char message[MAX_MESSAGE_SIZE];

    auto mqCall = posixCall(mq_receive)(m_mqDescriptor, &message[0], MAX_MESSAGE_SIZE, nullptr)
                      .failureReturnValue(ERROR_CODE)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok(std::string(&(message[0])));
}

expected<mqd_t, IpcChannelError>
MessageQueue::open(const IpcChannelName_t& name, mq_attr& attributes, const IpcChannelSide channelSide) noexcept
{
    auto sanitizedNameResult = sanitizeIpcChannelName(name);
    if (sanitizedNameResult.has_error())
    {
        return err(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    const auto& sanitizedName = sanitizedNameResult.value();
    {
        IOX_MAYBE_UNUSED std::false_type name; // name shall not be used anymore but only sanitizedName

        int32_t openFlags = O_RDWR;
        if (channelSide == IpcChannelSide::SERVER)
        {
            /// NOLINTJUSTIFICATION used in internal implementation which wraps the posix functionality
            /// NOLINTNEXTLINE(hicpp-signed-bitwise)
            openFlags |= O_CREAT;
        }

        // the mask will be applied to the permissions, therefore we need to set it to 0
        mode_t umaskSaved = umask(0);
        auto mqCall = posixCall(iox_mq_open4)(sanitizedName.c_str(), openFlags, MessageQueue::FILE_MODE, &attributes)
                          .failureReturnValue(MessageQueue::INVALID_DESCRIPTOR)
                          .suppressErrorMessagesForErrnos(ENOENT)
                          .evaluate();

        umask(umaskSaved);

        if (mqCall.has_error())
        {
            return err(MessageQueue::errnoToEnum(sanitizedName, mqCall.error().errnum));
        }

        return ok<mqd_t>(mqCall->value);
    }
}

expected<void, IpcChannelError> MessageQueue::close() noexcept
{
    auto mqCall = posixCall(mq_close)(m_mqDescriptor).failureReturnValue(ERROR_CODE).evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok();
}

expected<void, IpcChannelError> MessageQueue::unlink() noexcept
{
    if (m_channelSide == IpcChannelSide::CLIENT)
    {
        return ok();
    }

    auto mqCall = posixCall(mq_unlink)(m_name.c_str()).failureReturnValue(ERROR_CODE).evaluate();
    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok();
}

expected<std::string, IpcChannelError> MessageQueue::timedReceive(const units::Duration& timeout) const noexcept
{
    timespec timeOut = timeout.timespec(units::TimeSpecReference::Epoch);
    /// NOLINTJUSTIFICATION required as internal buffer for receive
    /// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    char message[MAX_MESSAGE_SIZE];

    auto mqCall = posixCall(mq_timedreceive)(m_mqDescriptor, &message[0], MAX_MESSAGE_SIZE, nullptr, &timeOut)
                      .failureReturnValue(ERROR_CODE)
                      // don't use the suppressErrorMessagesForErrnos method since QNX used EINTR instead of ETIMEDOUT
                      .ignoreErrnos(TIMEOUT_ERRNO)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return ok(std::string(&(message[0])));
}

expected<void, IpcChannelError> MessageQueue::timedSend(const std::string& msg,
                                                        const units::Duration& timeout) const noexcept
{
    const uint64_t messageSize = msg.size() + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        IOX_LOG(ERROR) << "the message '" << msg << "' which should be sent to the message queue '" << m_name
                       << "' is too long";
        return err(IpcChannelError::MESSAGE_TOO_LONG);
    }

    timespec timeOut = timeout.timespec(units::TimeSpecReference::Epoch);

    auto mqCall = posixCall(mq_timedsend)(m_mqDescriptor, msg.c_str(), messageSize, 1U, &timeOut)
                      .failureReturnValue(ERROR_CODE)
                      // don't use the suppressErrorMessagesForErrnos method since QNX used EINTR instead of ETIMEDOUT
                      .ignoreErrnos(TIMEOUT_ERRNO)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return ok();
}

expected<bool, IpcChannelError> MessageQueue::isOutdated() noexcept
{
    return ok(false);
}

IpcChannelError MessageQueue::errnoToEnum(const int32_t errnum) const noexcept
{
    return errnoToEnum(m_name, errnum);
}

IpcChannelError MessageQueue::errnoToEnum(const IpcChannelName_t& name, const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        IOX_LOG(ERROR) << "access denied to message queue '" << name << "'";
        return IpcChannelError::ACCESS_DENIED;
    }
    case EAGAIN:
    {
        IOX_LOG(ERROR) << "the message queue '" << name << "' is full";
        return IpcChannelError::CHANNEL_FULL;
    }
    case ETIMEDOUT:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::TIMEOUT;
    }
    case EEXIST:
    {
        IOX_LOG(ERROR) << "message queue '" << name << "' already exists";
        return IpcChannelError::CHANNEL_ALREADY_EXISTS;
    }
    case EINVAL:
    {
        IOX_LOG(ERROR) << "provided invalid arguments for message queue '" << name << "'";
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::NO_SUCH_CHANNEL;
    }
    case ENAMETOOLONG:
    {
        IOX_LOG(ERROR) << "message queue name '" << name << "' is too long";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    default:
    {
        IOX_LOG(ERROR) << "internal logic error in message queue '" << name << "' occurred";
        return IpcChannelError::INTERNAL_LOGIC_ERROR;
    }
    }
}

expected<IpcChannelName_t, IpcChannelError> MessageQueue::sanitizeIpcChannelName(const IpcChannelName_t& name) noexcept
{
    /// @todo iox-#832 the check for the longest valid queue name is missing
    /// the name for the mqeue is limited by MAX_PATH
    /// The mq_open call is wrapped by posixCall to throw then an ENAMETOOLONG error
    /// See: https://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_open.html
    if (name.empty() || name.size() < SHORTEST_VALID_QUEUE_NAME)
    {
        return err(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    // name is checked for emptiness, so it's ok to get a first member
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (name.c_str()[0] != '/')
    {
        return ok(IpcChannelName_t("/").append(iox::TruncateToCapacity, name));
    }

    return ok(name);
}

} // namespace posix
} // namespace iox
