// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/named_pipe.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <thread>

namespace iox
{
namespace posix
{
constexpr const char NamedPipe::NAMED_PIPE_PREFIX[];
constexpr units::Duration NamedPipe::CYCLE_TIME;

NamedPipe::NamedPipe() noexcept
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

NamedPipe::NamedPipe(const IpcChannelName_t& name,
                     const IpcChannelSide channelSide,
                     const size_t maxMsgSize,
                     const uint64_t maxMsgNumber) noexcept
{
    // We do not store maxMsgSize or maxMsgNumber, this is just technical debt since every ipc channel
    // requires the same behavior as the message queue. The named pipe would get later two template
    // parameters MAX_MSG_SIZE and MAX_MSG_NUMBER from which the Message_t size and m_messages queue
    // size is obtained. Reducing the max message size / number of messages even further would not gain
    // reduced memory usage or decreased runtime. See issue #832.
    if (name.size() + strlen(NAMED_PIPE_PREFIX) > MAX_MESSAGE_SIZE)
    {
        std::cerr << "The named pipe name: \"" << name
                  << "\" is too long. Maxium name length is: " << MAX_MESSAGE_SIZE - strlen(NAMED_PIPE_PREFIX)
                  << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        return;
    }

    // leading slash is allowed even though it is not a valid file name
    bool isValidPipeName = cxx::isValidFileName(name)
                           || (!name.empty() && name.c_str()[0] == '/' && cxx::isValidFileName(*name.substr(1)));
    if (!isValidPipeName)
    {
        std::cerr << "The named pipe name: \"" << name << "\" is not a valid file path name." << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        return;
    }

    if (maxMsgSize > MAX_MESSAGE_SIZE)
    {
        std::cerr << "A message size of " << maxMsgSize << " exceeds the maximum message size for named pipes of "
                  << MAX_MESSAGE_SIZE << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
        return;
    }

    if (maxMsgNumber > MAX_NUMBER_OF_MESSAGES)
    {
        std::cerr << "A message amount of " << maxMsgNumber
                  << " exceeds the maximum number of messages for named pipes of " << MAX_NUMBER_OF_MESSAGES
                  << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
        return;
    }

    auto sharedMemory = SharedMemoryObject::create(
        convertName(name),
        // add alignment since we require later aligned memory to perform the placement new of
        // m_messages. when we add the alignment it is guaranteed that enough memory should be available.
        sizeof(MessageQueue_t) + alignof(MessageQueue_t),
        AccessMode::READ_WRITE,
        (channelSide == IpcChannelSide::SERVER) ? OwnerShip::MINE : OwnerShip::OPEN_EXISTING_SHM,
        iox::posix::SharedMemoryObject::NO_ADDRESS_HINT);

    if (sharedMemory.has_error())
    {
        std::cerr << "Unable to open shared memory: \"" << convertName(name) << "\" for named pipe \"" << name << "\""
                  << std::endl;
        m_isInitialized = false;
        m_errorValue = (channelSide == IpcChannelSide::CLIENT) ? IpcChannelError::NO_SUCH_CHANNEL
                                                               : IpcChannelError::INTERNAL_LOGIC_ERROR;
        return;
    }

    m_sharedMemory.emplace(std::move(sharedMemory.value()));
    m_messages =
        static_cast<MessageQueue_t*>(m_sharedMemory->allocate(sizeof(MessageQueue_t), alignof(MessageQueue_t)));

    if (channelSide == IpcChannelSide::SERVER)
    {
        new (m_messages) MessageQueue_t();
    }
    m_isInitialized = true;
}

NamedPipe::NamedPipe(NamedPipe&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipe& NamedPipe::operator=(NamedPipe&& rhs) noexcept
{
    if (this != &rhs)
    {
        IOX_DISCARD_RESULT(destroy());
        CreationPattern_t::operator=(std::move(rhs));

        m_sharedMemory = std::move(rhs.m_sharedMemory);
        m_messages = std::move(rhs.m_messages);
        rhs.m_messages = nullptr;
    }

    return *this;
}

NamedPipe::~NamedPipe() noexcept
{
    IOX_DISCARD_RESULT(destroy());
}

IpcChannelName_t NamedPipe::convertName(const IpcChannelName_t& name) noexcept
{
    return IpcChannelName_t(
        cxx::TruncateToCapacity,
        cxx::concatenate(NAMED_PIPE_PREFIX, (name.c_str()[0] == '/') ? *name.substr(1) : name).c_str());
}

cxx::expected<IpcChannelError> NamedPipe::destroy() noexcept
{
    if (m_isInitialized)
    {
        m_isInitialized = false;
        m_errorValue = IpcChannelError::NOT_INITIALIZED;
        m_messages->~LockFreeQueue();
        m_sharedMemory.reset();
        m_messages = nullptr;
    }
    return cxx::success<>();
}

cxx::expected<bool, IpcChannelError> NamedPipe::isOutdated() noexcept
{
    return cxx::success<bool>(false);
}

cxx::expected<bool, IpcChannelError> NamedPipe::unlinkIfExists(const IpcChannelName_t& name) noexcept
{
    constexpr int ERROR_CODE = -1;
    auto unlinkCall =
        posixCall(shm_unlink)(convertName(name).c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate();
    if (!unlinkCall.has_error())
    {
        return cxx::success<bool>(unlinkCall->errnum != ENOENT);
    }
    else
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
}

cxx::expected<IpcChannelError> NamedPipe::send(const std::string& message) const noexcept
{
    return timedSend(message, units::Duration::max());
}

cxx::expected<IpcChannelError> NamedPipe::timedSend(const std::string& message,
                                                    const units::Duration& timeout) const noexcept
{
    if (!m_isInitialized)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    units::Duration remainingTime = timeout;
    do
    {
        if (m_messages->tryPush(Message_t(cxx::TruncateToCapacity, message)))
        {
            return cxx::success<>();
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(CYCLE_TIME.toNanoseconds()));
        remainingTime = remainingTime - CYCLE_TIME;
    } while (remainingTime.toNanoseconds() > 0U);

    return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

cxx::expected<std::string, IpcChannelError> NamedPipe::receive() const noexcept
{
    return timedReceive(units::Duration::max());
}

cxx::expected<std::string, IpcChannelError> NamedPipe::timedReceive(const units::Duration& timeout) const noexcept
{
    if (!m_isInitialized)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    units::Duration remainingTime = timeout;
    do
    {
        auto message = m_messages->pop();
        if (message.has_value())
        {
            return cxx::success<std::string>(message->c_str());
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(CYCLE_TIME.toNanoseconds()));
        remainingTime = remainingTime - CYCLE_TIME;
    } while (remainingTime.toNanoseconds() > 0U);

    return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
}


} // namespace posix
} // namespace iox
