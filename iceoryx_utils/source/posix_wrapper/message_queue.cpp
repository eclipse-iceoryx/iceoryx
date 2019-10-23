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

#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <chrono>
#include <cstdlib>

namespace iox
{
namespace posix
{
MessageQueue::MessageQueue()
{
    this->m_isInitialized = false;
    this->m_errorValue = MessageQueueError::MqNotInitialized;
}

MessageQueue::MessageQueue(const std::string& f_name,
                           const MessageQueueMode f_mode,
                           const MessageQueueOwnership f_ownerShip)
    : m_name{f_name}
    , m_ownerShip(f_ownerShip)
{
    // fields have a different order in QNX,
    // so we need to initialize by name
    m_attributes.mq_flags = (f_mode == MessageQueueMode::NonBlocking) ? O_NONBLOCK : 0;
    m_attributes.mq_maxmsg = MaxMsgNumber;
    m_attributes.mq_msgsize = MaxMsgSize;
    m_attributes.mq_curmsgs = 0;
#ifdef __QNX__
    m_attributes.mq_recvwait = 0;
    m_attributes.mq_sendwait = 0;
#endif
    auto openResult = open(f_name, f_mode, f_ownerShip);

    if (!openResult.has_error())
    {
        this->m_isInitialized = true;
        this->m_errorValue = MessageQueueError::Undefined;
        this->m_mqDescriptor = openResult.get_value();
    }
    else
    {
        this->m_isInitialized = false;
        this->m_errorValue = openResult.get_error();
    }
}

MessageQueue::MessageQueue(MessageQueue&& other)
{
    *this = std::move(other);
}

MessageQueue::~MessageQueue()
{
    if (destroy().has_error())
    {
        std::cerr << "unable to cleanup message queue \"" << m_name << "\" in the destructor" << std::endl;
    }
}

MessageQueue& MessageQueue::operator=(MessageQueue&& other)
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            std::cerr << "unable to cleanup message queue \"" << m_name
                      << "\" during move operation - resource leaks are possible!" << std::endl;
        }

        m_name = std::move(other.m_name);
        m_attributes = std::move(other.m_attributes);
        m_mqDescriptor = std::move(other.m_mqDescriptor);
        m_ownerShip = std::move(other.m_ownerShip);
        other.m_mqDescriptor = InvalidDescriptor;
    }

    return *this;
}

cxx::expected<MessageQueueError> MessageQueue::destroy()
{
    if (m_mqDescriptor != InvalidDescriptor)
    {
        auto closeCall = close();
        if (closeCall.has_error())
        {
            m_mqDescriptor = InvalidDescriptor;
            return closeCall;
        }
        auto unlinkCall = unlink();
        if (unlinkCall.has_error())
        {
            m_mqDescriptor = InvalidDescriptor;
            return unlinkCall;
        }
    }

    m_mqDescriptor = InvalidDescriptor;
    return cxx::success<void>();
}

cxx::expected<MessageQueueError> MessageQueue::send(const std::string& f_msg)
{
    if (f_msg.size() > MaxMsgSize)
    {
        return cxx::error<MessageQueueError>(MessageQueueError::MessageTooLong);
    }

    auto mqCall = cxx::makeSmartC(mq_send,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {ErrorCode},
                                  {},
                                  m_mqDescriptor,
                                  f_msg.c_str(),
                                  f_msg.size() + 1, // +1 for the \0 at the end
                                  1);

    if (mqCall.hasErrors())
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }

    return cxx::success<void>();
}

cxx::expected<std::string, MessageQueueError> MessageQueue::receive()
{
    char message[MaxMsgSize];
    auto mqCall = cxx::makeSmartC(mq_receive,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {static_cast<ssize_t>(ErrorCode)},
                                  {},
                                  m_mqDescriptor,
                                  &(message[0]),
                                  MaxMsgSize,
                                  nullptr);

    if (mqCall.hasErrors())
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }

    return cxx::success<std::string>(std::string(&(message[0])));
}

cxx::expected<int32_t, MessageQueueError>
MessageQueue::open(const std::string& f_name, const MessageQueueMode f_mode, const MessageQueueOwnership f_ownerShip)
{
    if (f_name.empty() || f_name.at(0) != '/')
    {
        return cxx::error<MessageQueueError>(MessageQueueError::InvalidMessageQueueName);
    }

    int32_t openFlags = O_RDWR;
    openFlags |= (f_mode == MessageQueueMode::NonBlocking) ? O_NONBLOCK : 0;
    if (f_ownerShip == MessageQueueOwnership::CreateNew)
    {
        openFlags |= O_CREAT;
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0);

    auto mqCall = cxx::makeSmartC(mq_open,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {ErrorCode},
                                  {},
                                  f_name.c_str(),
                                  openFlags,
                                  m_filemode,
                                  &m_attributes);

    umask(umaskSaved);

    if (!mqCall.hasErrors())
    {
        return cxx::success<int32_t>(mqCall.getReturnValue());
    }
    else
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }
}

cxx::expected<MessageQueueError> MessageQueue::close()
{
    auto mqCall = cxx::makeSmartC(mq_close, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ErrorCode}, {}, m_mqDescriptor);

    if (mqCall.hasErrors())
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }

    return cxx::success<void>();
}

cxx::expected<MessageQueueError> MessageQueue::unlink()
{
    if (m_ownerShip == MessageQueueOwnership::OpenExisting)
    {
        return cxx::success<void>();
    }
    else
    {
        auto mqCall =
            cxx::makeSmartC(mq_unlink, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ErrorCode}, {}, m_name.c_str());
        if (mqCall.hasErrors())
        {
            return createErrorFromErrnum(mqCall.getErrNum());
        }

        return cxx::success<void>();
    }
}

cxx::expected<std::string, MessageQueueError> MessageQueue::timedReceive(const units::Duration& f_timeout)
{
    timespec timeOut = f_timeout.timespec(units::TimeSpecReference::Epoch);
    char message[MaxMsgSize];

    auto mqCall = cxx::makeSmartC(mq_timedreceive,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {static_cast<ssize_t>(ErrorCode)},
                                  {TIMEOUT_ERRNO},
                                  m_mqDescriptor,
                                  &(message[0]),
                                  MaxMsgSize,
                                  nullptr,
                                  &timeOut);

    if (mqCall.hasErrors())
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }
    else if (mqCall.getErrNum() == TIMEOUT_ERRNO)
    {
        return createErrorFromErrnum(ETIMEDOUT);
    }

    return cxx::success<std::string>(std::string(&(message[0])));
}

cxx::expected<MessageQueueError> MessageQueue::timedSend(const std::string& f_msg, const units::Duration& f_timeout)
{
    if (f_msg.size() > MaxMsgSize)
    {
        std::cerr << "the message \"" << f_msg << "\" which should be sent to the message queue \"" << m_name
                  << "\" is too long" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::MessageTooLong);
    }

    timespec timeOut = f_timeout.timespec(units::TimeSpecReference::Epoch);

    auto mqCall = cxx::makeSmartC(mq_timedsend,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {ErrorCode},
                                  {TIMEOUT_ERRNO},
                                  m_mqDescriptor,
                                  f_msg.c_str(),
                                  f_msg.size(),
                                  1,
                                  &timeOut);

    if (mqCall.hasErrors())
    {
        return createErrorFromErrnum(mqCall.getErrNum());
    }
    else if (mqCall.getErrNum() == TIMEOUT_ERRNO)
    {
        return createErrorFromErrnum(ETIMEDOUT);
    }

    return cxx::success<void>();
}

cxx::error<MessageQueueError> MessageQueue::createErrorFromErrnum(const int errnum)
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "access denied to message queue \"" << m_name << "\"" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::AccessDenied);
    }
    case EAGAIN:
    {
        std::cerr << "the message queue \"" << m_name << "\" is full" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::MessageQueueIsFull);
    }
    case ETIMEDOUT:
    {
        // no error message needed since this is a normal use case
        return cxx::error<MessageQueueError>(MessageQueueError::Timeout);
    }
    case EEXIST:
    {
        std::cerr << "message queue \"" << m_name << "\" already exists" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::MessageQueueAlreadyExists);
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for message queue \"" << m_name << "\"" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::InvalidArguments);
    }
    case ENOENT:
    {
        std::cerr << "message queue \"" << m_name << "\" does not exist" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::NoSuchMessageQueue);
    }
    default:
    {
        std::cerr << "internal logic error in message queue \"" << m_name << "\" occurred" << std::endl;
        return cxx::error<MessageQueueError>(MessageQueueError::InternalLogicError);
    }
    }
}

} // namespace posix
} // namespace iox
