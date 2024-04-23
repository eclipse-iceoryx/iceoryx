// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/named_pipe.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/filesystem.hpp"
#include "iox/into.hpp"
#include "iox/logging.hpp"
#include "iox/std_string_support.hpp"

#include <thread>

namespace iox
{
// NOLINTJUSTIFICATION see declaration in header
// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
constexpr const char NamedPipe::NAMED_PIPE_PREFIX[];
constexpr units::Duration NamedPipe::CYCLE_TIME;
constexpr units::Duration NamedPipe::NamedPipeData::WAIT_FOR_INIT_SLEEP_TIME;
constexpr units::Duration NamedPipe::NamedPipeData::WAIT_FOR_INIT_TIMEOUT;

// NOLINTJUSTIFICATION the function size and cognitive complexity are cause by the error handling and the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
expected<NamedPipe, PosixIpcChannelError> NamedPipeBuilder::create() const noexcept
{
    if (m_name.size() + strlen(&NamedPipe::NAMED_PIPE_PREFIX[0]) > NamedPipe::MAX_MESSAGE_SIZE)
    {
        IOX_LOG(ERROR,
                "The named pipe name: '" << m_name << "' is too long. Maxium name length is: "
                                         << NamedPipe::MAX_MESSAGE_SIZE - strlen(&NamedPipe::NAMED_PIPE_PREFIX[0]));
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }

    // leading slash is allowed even though it is not a valid file name
    bool isValidPipeName = isValidFileName(m_name)
                           // name is checked for emptiness, so it's ok to get a first member
                           // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                           || (!m_name.empty() && m_name.c_str()[0] == '/' && isValidFileName(*m_name.substr(1)));
    if (!isValidPipeName)
    {
        IOX_LOG(ERROR, "The named pipe name: '" << m_name << "' is not a valid file path name.");
        return err(PosixIpcChannelError::INVALID_CHANNEL_NAME);
    }

    if (m_maxMsgSize > NamedPipe::MAX_MESSAGE_SIZE)
    {
        IOX_LOG(ERROR,
                "A message size of " << m_maxMsgSize << " exceeds the maximum message size for named pipes of "
                                     << NamedPipe::MAX_MESSAGE_SIZE);
        return err(PosixIpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED);
    }

    if (m_maxMsgNumber > NamedPipe::MAX_NUMBER_OF_MESSAGES)
    {
        IOX_LOG(ERROR,
                "A message amount of " << m_maxMsgNumber
                                       << " exceeds the maximum number of messages for named pipes of "
                                       << NamedPipe::MAX_NUMBER_OF_MESSAGES);
        return err(PosixIpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED);
    }

    auto namedPipeShmName = NamedPipe::mapToSharedMemoryName(NamedPipe::NAMED_PIPE_PREFIX, m_name);
    auto sharedMemoryResult =
        PosixSharedMemoryObjectBuilder()
            .name(namedPipeShmName)
            .memorySizeInBytes(sizeof(NamedPipe::NamedPipeData) + alignof(NamedPipe::NamedPipeData))
            .accessMode(AccessMode::READ_WRITE)
            .openMode((m_channelSide == PosixIpcChannelSide::SERVER) ? OpenMode::OPEN_OR_CREATE
                                                                     : OpenMode::OPEN_EXISTING)
            .permissions(perms::owner_all | perms::group_all)
            .create();

    if (sharedMemoryResult.has_error())
    {
        IOX_LOG(ERROR, "Unable to open shared memory: '" << namedPipeShmName << "' for named pipe '" << m_name << "'");
        return err((m_channelSide == PosixIpcChannelSide::CLIENT) ? PosixIpcChannelError::NO_SUCH_CHANNEL
                                                                  : PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    auto& sharedMemory = sharedMemoryResult.value();

    BumpAllocator allocator(sharedMemory.getBaseAddress(),
                            sharedMemory.get_size().expect("failed to acquire shm size"));

    auto allocationResult = allocator.allocate(sizeof(NamedPipe::NamedPipeData), alignof(NamedPipe::NamedPipeData));
    if (allocationResult.has_error())
    {
        IOX_LOG(ERROR, "Unable to allocate memory for named pipe '" << m_name << "'");
        return err(PosixIpcChannelError::MEMORY_ALLOCATION_FAILED);
    }
    auto* data = static_cast<NamedPipe::NamedPipeData*>(allocationResult.value());

    if (sharedMemory.hasOwnership())
    {
        new (data) NamedPipe::NamedPipeData();
        auto dataInitializeResult = data->initialize(static_cast<uint32_t>(m_maxMsgNumber));
        if (dataInitializeResult.has_error())
        {
            return err(dataInitializeResult.error());
        }
    }
    else
    {
        if (!data->waitForInitialization())
        {
            return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
        }
    }

    return ok(NamedPipe{std::move(sharedMemory), data});
}

NamedPipe::NamedPipe(PosixSharedMemoryObject&& sharedMemory, NamedPipeData* data) noexcept
    : m_sharedMemory(std::move(sharedMemory))
    , m_data(data)
{
}

NamedPipe::NamedPipe(NamedPipe&& rhs) noexcept
    : m_sharedMemory(std::move(rhs.m_sharedMemory))
    , m_data(rhs.m_data)
{
    rhs.m_data = nullptr;
}

NamedPipe& NamedPipe::operator=(NamedPipe&& rhs) noexcept
{
    if (this != &rhs)
    {
        IOX_DISCARD_RESULT(destroy());

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
PosixIpcChannelName_t NamedPipe::mapToSharedMemoryName(const Prefix& p, const PosixIpcChannelName_t& name) noexcept
{
    PosixIpcChannelName_t channelName = p;

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

expected<void, PosixIpcChannelError> NamedPipe::destroy() noexcept
{
    if (m_data != nullptr)
    {
        if (m_sharedMemory.hasOwnership())
        {
            m_data->~NamedPipeData();
        }
        m_data = nullptr;
    }
    return ok();
}

expected<bool, PosixIpcChannelError> NamedPipe::unlinkIfExists(const PosixIpcChannelName_t& name) noexcept
{
    auto result = detail::PosixSharedMemory::unlinkIfExist(mapToSharedMemoryName(NAMED_PIPE_PREFIX, name));
    if (result.has_error())
    {
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    return ok(*result);
}

expected<void, PosixIpcChannelError> NamedPipe::trySend(const std::string& message) const noexcept
{
    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }
    return trySend(into<lossy<Message_t>>(message));
}

expected<void, PosixIpcChannelError> NamedPipe::send(const std::string& message) const noexcept
{
    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }

    return send(into<lossy<Message_t>>(message));
}

expected<void, PosixIpcChannelError> NamedPipe::timedSend(const std::string& message,
                                                          const units::Duration& timeout) const noexcept
{
    if (message.size() > MAX_MESSAGE_SIZE)
    {
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }

    return timedSend(into<lossy<Message_t>>(message), timeout);
}

expected<std::string, PosixIpcChannelError> NamedPipe::receive() const noexcept
{
    Message_t message;
    auto result = receive(message);
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok<std::string>(message.c_str());
}

expected<std::string, PosixIpcChannelError> NamedPipe::tryReceive() const noexcept
{
    Message_t message;
    auto result = tryReceive(message);
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok<std::string>(message.c_str());
}

expected<std::string, PosixIpcChannelError> NamedPipe::timedReceive(const units::Duration& timeout) const noexcept
{
    Message_t message;
    auto result = timedReceive(message, timeout);
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok<std::string>(message.c_str());
}

expected<void, PosixIpcChannelError> NamedPipe::NamedPipeData::initialize(const uint32_t maxMsgNumber) noexcept
{
    auto signalError = [&](const char* name) {
        IOX_LOG(ERROR, "Unable to create '" << name << "' semaphore for named pipe");
    };

    if (UnnamedSemaphoreBuilder()
            .initialValue(maxMsgNumber)
            .isInterProcessCapable(true)
            .create(m_sendSemaphore)
            .has_error())
    {
        signalError("send");
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    if (UnnamedSemaphoreBuilder().initialValue(0U).isInterProcessCapable(true).create(m_receiveSemaphore).has_error())
    {
        signalError("receive");
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    initializationGuard.store(VALID_DATA);

    return ok();
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

} // namespace iox
