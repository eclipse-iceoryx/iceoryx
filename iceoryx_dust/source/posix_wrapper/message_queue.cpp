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

#include "iceoryx_dust/posix_wrapper/message_queue.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/platform_correction.hpp"

#include <chrono>
#include <cstdlib>
#include <string>


namespace iox
{
namespace posix
{
MessageQueue::MessageQueue() noexcept
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

// NOLINTNEXTLINE(readability-function-size) @todo iox-#832 make a struct out of arguments
MessageQueue::MessageQueue(const IpcChannelName_t& name,
                           const IpcChannelSide channelSide,
                           /// NOLINTJUSTIFICATION @todo iox-#832 should be solved when the arguments are put in a
                           ///                      struct
                           /// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                           const size_t maxMsgSize,
                           const uint64_t maxMsgNumber) noexcept
    : m_channelSide(channelSide)
{
    sanitizeIpcChannelName(name)
        .and_then([this](IpcChannelName_t& name) { this->m_name = std::move(name); })
        .or_else([this](IpcChannelError) {
            this->m_isInitialized = false;
            this->m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        });

    if (maxMsgSize > MAX_MESSAGE_SIZE)
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
    }
    else
    {
        if (channelSide == IpcChannelSide::SERVER)
        {
            posixCall(mq_unlink)(m_name.c_str())
                .failureReturnValue(ERROR_CODE)
                .ignoreErrnos(ENOENT)
                .evaluate()
                .and_then([this](auto& r) {
                    if (r.errnum != ENOENT)
                    {
                        std::cout << "MQ still there, doing an unlink of " << m_name << std::endl;
                    }
                });
        }
        // fields have a different order in QNX,
        // so we need to initialize by name
        m_attributes.mq_flags = 0;
        m_attributes.mq_maxmsg = static_cast<long>(maxMsgNumber);
        m_attributes.mq_msgsize = static_cast<long>(maxMsgSize);
        m_attributes.mq_curmsgs = 0L;
#ifdef __QNX__
        m_attributes.mq_recvwait = 0L;
        m_attributes.mq_sendwait = 0L;
#endif
        auto openResult = open(m_name, channelSide);
        if (!openResult.has_error())
        {
            this->m_isInitialized = true;
            this->m_errorValue = IpcChannelError::UNDEFINED;
            this->m_mqDescriptor = openResult.value();
        }
        else
        {
            this->m_isInitialized = false;
            this->m_errorValue = openResult.get_error();
        }
    }
}

MessageQueue::MessageQueue(MessageQueue&& other) noexcept
{
    *this = std::move(other);
}

MessageQueue::~MessageQueue() noexcept
{
    if (destroy().has_error())
    {
        std::cerr << "unable to cleanup message queue \"" << m_name << "\" in the destructor" << std::endl;
    }
}

MessageQueue& MessageQueue::operator=(MessageQueue&& other) noexcept
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            std::cerr << "unable to cleanup message queue \"" << m_name
                      << "\" during move operation - resource leaks are possible!" << std::endl;
        }
        CreationPattern_t::operator=(std::move(other));

        /// NOLINTJUSTIFICATION iox-#1036 will be fixed with the builder pattern
        /// NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
        m_name = std::move(other.m_name);
        m_attributes = other.m_attributes;
        m_mqDescriptor = other.m_mqDescriptor;
        m_channelSide = other.m_channelSide;
        other.m_mqDescriptor = INVALID_DESCRIPTOR;
    }

    return *this;
}

cxx::expected<bool, IpcChannelError> MessageQueue::unlinkIfExists(const IpcChannelName_t& name) noexcept
{
    auto sanitizedIpcChannelName = sanitizeIpcChannelName(name);
    if (sanitizedIpcChannelName.has_error())
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }


    auto mqCall = posixCall(mq_unlink)(sanitizedIpcChannelName->c_str())
                      .failureReturnValue(ERROR_CODE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(*sanitizedIpcChannelName, mqCall.get_error().errnum);
    }
    return cxx::success<bool>(mqCall->errnum != ENOENT);
}

cxx::expected<IpcChannelError> MessageQueue::destroy() noexcept
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
    m_isInitialized = false;
    return cxx::success<void>();
}

cxx::expected<IpcChannelError> MessageQueue::send(const std::string& msg) const noexcept
{
    const uint64_t messageSize = msg.size() + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    auto mqCall =
        posixCall(mq_send)(m_mqDescriptor, msg.c_str(), messageSize, 1U).failureReturnValue(ERROR_CODE).evaluate();

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    return cxx::success<void>();
}

cxx::expected<std::string, IpcChannelError> MessageQueue::receive() const noexcept
{
    /// NOLINTJUSTIFICATION required as raw memory buffer for mq_receive
    /// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    char message[MAX_MESSAGE_SIZE];

    auto mqCall = posixCall(mq_receive)(m_mqDescriptor, &message[0], MAX_MESSAGE_SIZE, nullptr)
                      .failureReturnValue(ERROR_CODE)
                      .evaluate();

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    return cxx::success<std::string>(std::string(&(message[0])));
}

cxx::expected<mqd_t, IpcChannelError> MessageQueue::open(const IpcChannelName_t& name,
                                                         const IpcChannelSide channelSide) noexcept
{
    auto sanitizedIpcChannelName = sanitizeIpcChannelName(name);
    if (sanitizedIpcChannelName.has_error())
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    int32_t openFlags = O_RDWR;
    if (channelSide == IpcChannelSide::SERVER)
    {
        /// NOLINTJUSTIFICATION used in internal implementation which wraps the posix functionality
        /// NOLINTNEXTLINE(hicpp-signed-bitwise)
        openFlags |= O_CREAT;
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0);
    auto mqCall = posixCall(iox_mq_open4)(sanitizedIpcChannelName->c_str(), openFlags, m_filemode, &m_attributes)
                      .failureReturnValue(INVALID_DESCRIPTOR)
                      .suppressErrorMessagesForErrnos(ENOENT)
                      .evaluate();

    umask(umaskSaved);

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    return cxx::success<mqd_t>(mqCall->value);
}

cxx::expected<IpcChannelError> MessageQueue::close() noexcept
{
    auto mqCall = posixCall(mq_close)(m_mqDescriptor).failureReturnValue(ERROR_CODE).evaluate();

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    return cxx::success<void>();
}

cxx::expected<IpcChannelError> MessageQueue::unlink() noexcept
{
    if (m_channelSide == IpcChannelSide::CLIENT)
    {
        return cxx::success<void>();
    }

    auto mqCall = posixCall(mq_unlink)(m_name.c_str()).failureReturnValue(ERROR_CODE).evaluate();
    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    return cxx::success<void>();
}

cxx::expected<std::string, IpcChannelError> MessageQueue::timedReceive(const units::Duration& timeout) const noexcept
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
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return createErrorFromErrnum(ETIMEDOUT);
    }

    return cxx::success<std::string>(std::string(&(message[0])));
}

cxx::expected<IpcChannelError> MessageQueue::timedSend(const std::string& msg,
                                                       const units::Duration& timeout) const noexcept
{
    const uint64_t messageSize = msg.size() + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        std::cerr << "the message \"" << msg << "\" which should be sent to the message queue \"" << m_name
                  << "\" is too long" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    timespec timeOut = timeout.timespec(units::TimeSpecReference::Epoch);

    auto mqCall = posixCall(mq_timedsend)(m_mqDescriptor, msg.c_str(), messageSize, 1U, &timeOut)
                      .failureReturnValue(ERROR_CODE)
                      // don't use the suppressErrorMessagesForErrnos method since QNX used EINTR instead of ETIMEDOUT
                      .ignoreErrnos(TIMEOUT_ERRNO)
                      .evaluate();

    if (mqCall.has_error())
    {
        return createErrorFromErrnum(mqCall.get_error().errnum);
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return createErrorFromErrnum(ETIMEDOUT);
    }

    return cxx::success<void>();
}

cxx::expected<bool, IpcChannelError> MessageQueue::isOutdated() noexcept
{
    return cxx::success<bool>(false);
}

cxx::error<IpcChannelError> MessageQueue::createErrorFromErrnum(const int32_t errnum) const noexcept
{
    return createErrorFromErrnum(m_name, errnum);
}

cxx::error<IpcChannelError> MessageQueue::createErrorFromErrnum(const IpcChannelName_t& name,
                                                                const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "access denied to message queue \"" << name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::ACCESS_DENIED);
    }
    case EAGAIN:
    {
        std::cerr << "the message queue \"" << name << "\" is full" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::CHANNEL_FULL);
    }
    case ETIMEDOUT:
    {
        // no error message needed since this is a normal use case
        return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
    }
    case EEXIST:
    {
        std::cerr << "message queue \"" << name << "\" already exists" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::CHANNEL_ALREADY_EXISTS);
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for message queue \"" << name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_ARGUMENTS);
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return cxx::error<IpcChannelError>(IpcChannelError::NO_SUCH_CHANNEL);
    }
    case ENAMETOOLONG:
    {
        std::cerr << "message queue name \"" << name << "\" is too long" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    default:
    {
        std::cerr << "internal logic error in message queue \"" << name << "\" occurred" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    }
}

cxx::expected<IpcChannelName_t, IpcChannelError>
MessageQueue::sanitizeIpcChannelName(const IpcChannelName_t& name) noexcept
{
    /// @todo iox-#832 the check for the longest valid queue name is missing
    /// the name for the mqeue is limited by MAX_PATH
    /// The mq_open call is wrapped by posixCall to throw then an ENAMETOOLONG error
    /// See: https://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_open.html
    if (name.empty() || name.size() < SHORTEST_VALID_QUEUE_NAME)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    // name is checked for emptiness, so it's ok to get a first member
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (name.c_str()[0] != '/')
    {
        return cxx::success<IpcChannelName_t>(IpcChannelName_t("/").append(iox::cxx::TruncateToCapacity, name));
    }

    return cxx::success<IpcChannelName_t>(name);
}

} // namespace posix
} // namespace iox
