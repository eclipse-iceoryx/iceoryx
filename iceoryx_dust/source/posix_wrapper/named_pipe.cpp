// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_dust/posix_wrapper/named_pipe.hpp"
#include "iceoryx_dust/cxx/std_string_support.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/into.hpp"

#include <thread>

namespace iox
{
namespace posix
{
/// NOLINTJUSTIFICATION see declaration in header
/// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
constexpr const char NamedPipe::NAMED_PIPE_PREFIX[];
constexpr units::Duration NamedPipe::CYCLE_TIME;
constexpr units::Duration NamedPipe::NamedPipeData::WAIT_FOR_INIT_SLEEP_TIME;
constexpr units::Duration NamedPipe::NamedPipeData::WAIT_FOR_INIT_TIMEOUT;

NamedPipe::NamedPipe() noexcept
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

// NOLINTNEXTLINE(readability-function-size) @todo iox-#832 make a struct out of arguments
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
    if (name.size() + strlen(&NAMED_PIPE_PREFIX[0]) > MAX_MESSAGE_SIZE)
    {
        std::cerr << "The named pipe name: \"" << name
                  << "\" is too long. Maxium name length is: " << MAX_MESSAGE_SIZE - strlen(&NAMED_PIPE_PREFIX[0])
                  << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        return;
    }

    // leading slash is allowed even though it is not a valid file name
    bool isValidPipeName = cxx::isValidFileName(name)
                           // name is checked for emptiness, so it's ok to get a first member
                           // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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

    if (!SharedMemoryObjectBuilder()
             .name(convertName(NAMED_PIPE_PREFIX, name))
             .memorySizeInBytes(sizeof(NamedPipeData) + alignof(NamedPipeData))
             .accessMode(AccessMode::READ_WRITE)
             .openMode((channelSide == IpcChannelSide::SERVER) ? OpenMode::OPEN_OR_CREATE : OpenMode::OPEN_EXISTING)
             .permissions(cxx::perms::owner_all | cxx::perms::group_all)
             .create()
             .and_then([this](auto& value) { m_sharedMemory.emplace(std::move(value)); }))
    {
        std::cerr << "Unable to open shared memory: \"" << convertName(NAMED_PIPE_PREFIX, name)
                  << "\" for named pipe \"" << name << "\"" << std::endl;
        m_isInitialized = false;
        m_errorValue = (channelSide == IpcChannelSide::CLIENT) ? IpcChannelError::NO_SUCH_CHANNEL
                                                               : IpcChannelError::INTERNAL_LOGIC_ERROR;
        return;
    }

    auto allocationResult = m_sharedMemory->allocate(sizeof(NamedPipeData), alignof(NamedPipeData));
    if (allocationResult.has_error())
    {
        std::cerr << "Unable to allocate memory for named pipe \"" << name << "\"" << std::endl;
        m_isInitialized = false;
        m_errorValue = IpcChannelError::MEMORY_ALLOCATION_FAILED;
        return;
    }
    m_data = static_cast<NamedPipeData*>(allocationResult.value());

    m_isInitialized = true;
    if (m_sharedMemory->hasOwnership())
    {
        new (m_data) NamedPipeData(m_isInitialized, m_errorValue, static_cast<uint32_t>(maxMsgNumber));
    }
    else
    {
        m_isInitialized = m_data->waitForInitialization();
        if (!m_isInitialized)
        {
            m_errorValue = IpcChannelError::INTERNAL_LOGIC_ERROR;
        }
    }
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

        /// NOLINTJUSTIFICATION iox-#1036 will be fixed with the builder pattern
        /// NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
        m_sharedMemory = std::move(rhs.m_sharedMemory);
        m_data = rhs.m_data;
        rhs.m_data = nullptr;
    }

    return *this;
}

NamedPipe::~NamedPipe() noexcept
{
    IOX_DISCARD_RESULT(destroy());
}

template <typename Prefix>
IpcChannelName_t NamedPipe::convertName(const Prefix& p, const IpcChannelName_t& name) noexcept
{
    IpcChannelName_t channelName = p;

    if (name[0] == '/')
    {
        name.substr(1).and_then([&](auto& s) { channelName.append(TruncateToCapacity, s); });
    }
    else
    {
        channelName.append(TruncateToCapacity, name);
    }

    return channelName;
}

expected<IpcChannelError> NamedPipe::destroy() noexcept
{
    if (m_isInitialized)
    {
        m_isInitialized = false;
        m_errorValue = IpcChannelError::NOT_INITIALIZED;
        if (m_sharedMemory->hasOwnership())
        {
            m_data->~NamedPipeData();
        }
        m_sharedMemory.reset();
        m_data = nullptr;
    }
    return success<>();
}

expected<bool, IpcChannelError> NamedPipe::unlinkIfExists(const IpcChannelName_t& name) noexcept
{
    auto result = SharedMemory::unlinkIfExist(convertName(NAMED_PIPE_PREFIX, name));
    if (result.has_error())
    {
        return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    return success<bool>(*result);
}

expected<IpcChannelError> NamedPipe::trySend(const std::string& message) const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    auto result = m_data->sendSemaphore().tryWait();
    cxx::Expects(!result.has_error());

    if (*result)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(into<lossy<Message_t>>(message)));
        cxx::Expects(!m_data->receiveSemaphore().post().has_error());
        return success<>();
    }
    return error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

expected<IpcChannelError> NamedPipe::send(const std::string& message) const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    cxx::Expects(!m_data->sendSemaphore().wait().has_error());
    IOX_DISCARD_RESULT(m_data->messages.push(into<lossy<Message_t>>(message)));
    cxx::Expects(!m_data->receiveSemaphore().post().has_error());

    return success<>();
}

expected<IpcChannelError> NamedPipe::timedSend(const std::string& message,
                                               const units::Duration& timeout) const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    auto result = m_data->sendSemaphore().timedWait(timeout);
    cxx::Expects(!result.has_error());

    if (*result == SemaphoreWaitState::NO_TIMEOUT)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(into<lossy<Message_t>>(message)));
        cxx::Expects(!m_data->receiveSemaphore().post().has_error());
        return success<>();
    }
    return error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

expected<std::string, IpcChannelError> NamedPipe::receive() const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    cxx::Expects(!m_data->receiveSemaphore().wait().has_error());
    auto message = m_data->messages.pop();
    if (message.has_value())
    {
        cxx::Expects(!m_data->sendSemaphore().post().has_error());
        return success<std::string>(message->c_str());
    }
    return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
}

expected<std::string, IpcChannelError> NamedPipe::tryReceive() const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    auto result = m_data->receiveSemaphore().tryWait();
    cxx::Expects(!result.has_error());

    if (*result)
    {
        auto message = m_data->messages.pop();
        if (message.has_value())
        {
            cxx::Expects(!m_data->sendSemaphore().post().has_error());
            return success<std::string>(message->c_str());
        }
        return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    return error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

expected<std::string, IpcChannelError> NamedPipe::timedReceive(const units::Duration& timeout) const noexcept
{
    if (!m_isInitialized)
    {
        return error<IpcChannelError>(IpcChannelError::NOT_INITIALIZED);
    }

    auto result = m_data->receiveSemaphore().timedWait(timeout);
    cxx::Expects(!result.has_error());

    if (*result == SemaphoreWaitState::NO_TIMEOUT)
    {
        auto message = m_data->messages.pop();
        if (message.has_value())
        {
            cxx::Expects(!m_data->sendSemaphore().post().has_error());
            return success<std::string>(message->c_str());
        }
        return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    return error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init) semaphores are initalized via placementCreate call
NamedPipe::NamedPipeData::NamedPipeData(bool& isInitialized,
                                        IpcChannelError& error,
                                        const uint32_t maxMsgNumber) noexcept
{
    auto signalError = [&](const char* name) {
        std::cerr << "Unable to create " << name << " semaphore for named pipe \"" << 'x' << "\"";
        isInitialized = false;
        error = IpcChannelError::INTERNAL_LOGIC_ERROR;
    };

    UnnamedSemaphoreBuilder()
        .initialValue(maxMsgNumber)
        .isInterProcessCapable(true)
        .create(m_sendSemaphore)
        .or_else([&](auto) { signalError("send"); });

    if (!isInitialized)
    {
        return;
    }

    UnnamedSemaphoreBuilder()
        .initialValue(0U)
        .isInterProcessCapable(true)
        .create(m_receiveSemaphore)
        .or_else([&](auto) { signalError("receive"); });

    if (!isInitialized)
    {
        return;
    }

    initializationGuard.store(VALID_DATA);
}

UnnamedSemaphore& NamedPipe::NamedPipeData::sendSemaphore() noexcept
{
    return *m_sendSemaphore;
}

UnnamedSemaphore& NamedPipe::NamedPipeData::receiveSemaphore() noexcept
{
    return *m_receiveSemaphore;
}

bool NamedPipe::NamedPipeData::waitForInitialization() const noexcept
{
    if (hasValidState())
    {
        return true;
    }

    deadline_timer deadlineTimer(WAIT_FOR_INIT_TIMEOUT);

    while (!deadlineTimer.hasExpired())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(WAIT_FOR_INIT_SLEEP_TIME.toNanoseconds()));
        if (hasValidState())
        {
            return true;
        }
    }

    return false;
}

bool NamedPipe::NamedPipeData::hasValidState() const noexcept
{
    return initializationGuard.load() == VALID_DATA;
}

} // namespace posix
} // namespace iox
