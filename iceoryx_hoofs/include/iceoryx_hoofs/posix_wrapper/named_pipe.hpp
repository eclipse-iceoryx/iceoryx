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
#ifndef IOX_HOOFS_POSIX_WRAPPER_NAMED_PIPE_HPP
#define IOX_HOOFS_POSIX_WRAPPER_NAMED_PIPE_HPP

#include "iceoryx_hoofs/concurrent/lockfree_queue.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/design_pattern/creation.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class NamedPipe : public DesignPattern::Creation<NamedPipe, IpcChannelError>
{
  public:
    // no system restrictions at all, except available memory. MAX_MESSAGE_SIZE and MAX_NUMBER_OF_MESSAGES can be
    // increased as long as there is enough memory available
    static constexpr uint64_t MAX_MESSAGE_SIZE = 4U * 1024U;
    static constexpr uint64_t MAX_NUMBER_OF_MESSAGES = 10U;

    static constexpr uint64_t NULL_TERMINATOR_SIZE = 0U;
    static constexpr units::Duration CYCLE_TIME = units::Duration::fromMilliseconds(10);
    static constexpr const char NAMED_PIPE_PREFIX[] = "/iox_np_";

    using Message_t = cxx::string<MAX_MESSAGE_SIZE>;
    using MessageQueue_t = concurrent::LockFreeQueue<Message_t, MAX_NUMBER_OF_MESSAGES>;

    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;

    /// @brief For compatibility with IpcChannel alias, default ctor which creates
    ///        an uninitialized NamedPipe.
    NamedPipe() noexcept;

    NamedPipe(NamedPipe&& rhs) noexcept;
    NamedPipe& operator=(NamedPipe&& rhs) noexcept;
    ~NamedPipe() noexcept;

    /// @brief destroys an initialized named pipe.
    /// @return is always successful
    cxx::expected<IpcChannelError> destroy() noexcept;

    /// @brief removes a named pipe artifact from the system
    /// @return true if the artifact was removed, false when no artifact was found and
    ///         IpcChannelError::INTERNAL_LOGIC_ERROR when shm_unlink failed
    static cxx::expected<bool, IpcChannelError> unlinkIfExists(const IpcChannelName_t& name) noexcept;

    /// @brief for compatibility with IpcChannelError
    /// @return always false
    cxx::expected<bool, IpcChannelError> isOutdated() noexcept;

    /// @brief sends a message via the named pipe. if the pipe is full this call is blocking until the message could be
    ///        delivered
    /// @param[in] message the message which should be sent, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @return success when message was sent otherwise an error which describes the failure
    cxx::expected<IpcChannelError> send(const std::string& message) const noexcept;

    /// @brief sends a message via the named pipe.
    /// @param[in] message the message which should be sent, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @param[in] timeout the timeout on how long this method should retry to send the message
    /// @return success when message was sent otherwise an error which describes the failure
    cxx::expected<IpcChannelError> timedSend(const std::string& message, const units::Duration& timeout) const noexcept;

    /// @brief receives a message via the named pipe. if the pipe is empty this call is blocking until a message was
    ///        received
    /// @return on success a string containing the message, otherwise an error which describes the failure
    cxx::expected<std::string, IpcChannelError> receive() const noexcept;

    /// @brief receives a message via the named pipe.
    /// @param[in] timeout the timeout on how long this method should retry to receive a message
    /// @return on success a string containing the message, otherwise an error which describes the failure
    cxx::expected<std::string, IpcChannelError> timedReceive(const units::Duration& timeout) const noexcept;

  private:
    friend class DesignPattern::Creation<NamedPipe, IpcChannelError>;

    /// @brief constructor which creates a named pipe. This creates a shared memory file with the
    ///        prefix NAMED_PIPE_PREFIX concatenated with name.
    /// @param[in] name the name of the named pipe
    /// @param[in] channelSide defines the channel side (server creates the shared memory, clients opens it)
    /// @param[in] maxMsgSize maximum message size, must be less or equal than MAX_MESSAGE_SIZE
    /// @param[in] maxMsgNumber the maximum number of messages, must be less or equal than MAX_NUMBER_OF_MESSAGES
    NamedPipe(const IpcChannelName_t& name,
              const IpcChannelSide channelSide,
              const size_t maxMsgSize = MAX_MESSAGE_SIZE,
              const uint64_t maxMsgNumber = MAX_NUMBER_OF_MESSAGES) noexcept;

    static IpcChannelName_t convertName(const IpcChannelName_t& name) noexcept;

  private:
    cxx::optional<SharedMemoryObject> m_sharedMemory;
    MessageQueue_t* m_messages = nullptr;
};
} // namespace posix
} // namespace iox

#endif
