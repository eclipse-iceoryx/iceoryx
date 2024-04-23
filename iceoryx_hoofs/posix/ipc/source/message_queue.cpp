// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iox/message_queue.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"
#include "iox/std_string_support.hpp"

#include <chrono>
#include <string>

namespace iox
{
expected<MessageQueue, PosixIpcChannelError> MessageQueueBuilder::create() const noexcept
{
    auto sanitzedNameResult = MessageQueue::sanitizeIpcChannelName(m_name);
    if (sanitzedNameResult.has_error())
    {
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }
    auto& sanitizedName = sanitzedNameResult.value();
    [[maybe_unused]] std::false_type m_name; // m_name shall not be used anymore but only sanitizedName

    if (m_maxMsgSize > MessageQueue::MAX_MESSAGE_SIZE)
    {
        return err(PosixIpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED);
    }

    if (m_channelSide == PosixIpcChannelSide::SERVER)
    {
        IOX_POSIX_CALL(mq_unlink)
        (sanitizedName.c_str())
            .failureReturnValue(MessageQueue::ERROR_CODE)
            .ignoreErrnos(ENOENT)
            .evaluate()
            .and_then([&sanitizedName](auto& r) {
                if (r.errnum != ENOENT)
                {
                    IOX_LOG(DEBUG, "MQ still there, doing an unlink of '" << sanitizedName << "'");
                }
            });
    }

    // fields have a different order in QNX, so we need to initialize by name
    mq_attr attributes{};
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

    return ok(MessageQueue{sanitizedName, attributes, mqDescriptor, m_channelSide});
}

MessageQueue::MessageQueue(const PosixIpcChannelName_t& name,
                           const mq_attr attributes,
                           mqd_t mqDescriptor,
                           const PosixIpcChannelSide channelSide) noexcept
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
        IOX_LOG(ERROR, "unable to cleanup message queue '" << m_name << "' in the destructor");
    }
}

MessageQueue& MessageQueue::operator=(MessageQueue&& other) noexcept
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            IOX_LOG(ERROR,
                    "unable to cleanup message queue '" << m_name
                                                        << "' during move operation - resource leaks are possible!");
        }

        m_name = std::move(other.m_name);
        m_attributes = other.m_attributes;
        m_mqDescriptor = other.m_mqDescriptor;
        m_channelSide = other.m_channelSide;
        other.m_mqDescriptor = INVALID_DESCRIPTOR;
    }

    return *this;
}

expected<bool, PosixIpcChannelError> MessageQueue::unlinkIfExists(const PosixIpcChannelName_t& name) noexcept
{
    auto sanitizedIpcChannelName = sanitizeIpcChannelName(name);
    if (sanitizedIpcChannelName.has_error())
    {
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }


    auto mqCall = IOX_POSIX_CALL(mq_unlink)(sanitizedIpcChannelName->c_str())
                      .failureReturnValue(ERROR_CODE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(*sanitizedIpcChannelName, mqCall.error().errnum));
    }
    return ok(mqCall->errnum != ENOENT);
}

expected<void, PosixIpcChannelError> MessageQueue::destroy() noexcept
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

expected<void, PosixIpcChannelError> MessageQueue::send(const std::string& msg) const noexcept
{
    return sendImpl<char, Termination::NULL_TERMINATOR>(msg.c_str(), msg.size());
}

expected<std::string, PosixIpcChannelError> MessageQueue::receive() const noexcept
{
    auto result = expected<uint64_t, PosixIpcChannelError>(in_place, uint64_t(0));
    Message_t msg;
    msg.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        result = this->receiveImpl<char, Termination::NULL_TERMINATOR>(str, info.total_size);
        if (result.has_error())
        {
            return 0;
        }
        return result.value();
    });
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok<std::string>(msg.c_str());
}

expected<mqd_t, PosixIpcChannelError> MessageQueue::open(const PosixIpcChannelName_t& name,
                                                         mq_attr& attributes,
                                                         const PosixIpcChannelSide channelSide) noexcept
{
    auto sanitizedNameResult = sanitizeIpcChannelName(name);
    if (sanitizedNameResult.has_error())
    {
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }
    const auto& sanitizedName = sanitizedNameResult.value();
    {
        [[maybe_unused]] std::false_type name; // name shall not be used anymore but only sanitizedName

        int32_t openFlags = O_RDWR;
        if (channelSide == PosixIpcChannelSide::SERVER)
        {
            // NOLINTJUSTIFICATION used in internal implementation which wraps the posix functionality
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            openFlags |= O_CREAT;
        }

        // the mask will be applied to the permissions, therefore we need to set it to 0
        mode_t umaskSaved = umask(0);
        auto mqCall =
            IOX_POSIX_CALL(iox_mq_open4)(sanitizedName.c_str(), openFlags, MessageQueue::FILE_MODE, &attributes)
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

expected<void, PosixIpcChannelError> MessageQueue::close() noexcept
{
    auto mqCall = IOX_POSIX_CALL(mq_close)(m_mqDescriptor).failureReturnValue(ERROR_CODE).evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok();
}

expected<void, PosixIpcChannelError> MessageQueue::unlink() noexcept
{
    if (m_channelSide == PosixIpcChannelSide::CLIENT)
    {
        return ok();
    }

    auto mqCall = IOX_POSIX_CALL(mq_unlink)(m_name.c_str()).failureReturnValue(ERROR_CODE).evaluate();
    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    return ok();
}

expected<std::string, PosixIpcChannelError> MessageQueue::timedReceive(const units::Duration& timeout) const noexcept
{
    auto result = expected<uint64_t, PosixIpcChannelError>(in_place, uint64_t(0));
    Message_t msg;
    msg.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        result = this->timedReceiveImpl<char, Termination::NULL_TERMINATOR>(str, info.total_size, timeout);
        if (result.has_error())
        {
            return 0;
        }
        return result.value();
    });
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok<std::string>(msg.c_str());
}

expected<void, PosixIpcChannelError> MessageQueue::timedSend(const std::string& msg,
                                                             const units::Duration& timeout) const noexcept
{
    return timedSendImpl<char, Termination::NULL_TERMINATOR>(msg.c_str(), msg.size(), timeout);
}

expected<bool, PosixIpcChannelError> MessageQueue::isOutdated() noexcept
{
    return ok(false);
}

PosixIpcChannelError MessageQueue::errnoToEnum(const int32_t errnum) const noexcept
{
    return errnoToEnum(m_name, errnum);
}

PosixIpcChannelError MessageQueue::errnoToEnum(const PosixIpcChannelName_t& name, const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        IOX_LOG(ERROR, "access denied to message queue '" << name << "'");
        return PosixIpcChannelError::ACCESS_DENIED;
    }
    case EAGAIN:
    {
        IOX_LOG(ERROR, "the message queue '" << name << "' is full");
        return PosixIpcChannelError::CHANNEL_FULL;
    }
    case ETIMEDOUT:
    {
        // no error message needed since this is a normal use case
        return PosixIpcChannelError::TIMEOUT;
    }
    case EEXIST:
    {
        IOX_LOG(ERROR, "message queue '" << name << "' already exists");
        return PosixIpcChannelError::CHANNEL_ALREADY_EXISTS;
    }
    case EINVAL:
    {
        IOX_LOG(ERROR, "provided invalid arguments for message queue '" << name << "'");
        return PosixIpcChannelError::INVALID_ARGUMENTS;
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return PosixIpcChannelError::NO_SUCH_CHANNEL;
    }
    case ENAMETOOLONG:
    {
        IOX_LOG(ERROR, "message queue name '" << name << "' is too long");
        return PosixIpcChannelError::INVALID_CHANNEL_NAME;
    }
    default:
    {
        IOX_LOG(ERROR, "internal logic error in message queue '" << name << "' occurred");
        return PosixIpcChannelError::INTERNAL_LOGIC_ERROR;
    }
    }
}

expected<PosixIpcChannelName_t, PosixIpcChannelError>
MessageQueue::sanitizeIpcChannelName(const PosixIpcChannelName_t& name) noexcept
{
    /// @todo iox-#832 the check for the longest valid queue name is missing
    /// the name for the mqeue is limited by MAX_PATH
    /// The mq_open call is wrapped by IOX_POSIX_CALL to throw then an ENAMETOOLONG error
    /// See: https://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_open.html
    if (name.empty() || name.size() < SHORTEST_VALID_QUEUE_NAME)
    {
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }
    // name is checked for emptiness, so it's ok to get a first member
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (name.c_str()[0] != '/')
    {
        return ok(PosixIpcChannelName_t("/").append(iox::TruncateToCapacity, name));
    }

    return ok(name);
}

} // namespace iox
