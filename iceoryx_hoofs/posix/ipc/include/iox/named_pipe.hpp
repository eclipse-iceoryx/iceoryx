// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_IPC_NAMED_PIPE_HPP
#define IOX_HOOFS_POSIX_IPC_NAMED_PIPE_HPP

#include "iceoryx_platform/semaphore.hpp"
#include "iox/atomic.hpp"
#include "iox/builder.hpp"
#include "iox/detail/mpmc_lockfree_queue.hpp"
#include "iox/duration.hpp"
#include "iox/expected.hpp"
#include "iox/iceoryx_hoofs_deployment.hpp"
#include "iox/optional.hpp"
#include "iox/posix_ipc_channel.hpp"
#include "iox/posix_shared_memory_object.hpp"
#include "iox/string.hpp"
#include "iox/uninitialized_array.hpp"
#include "iox/unnamed_semaphore.hpp"

#include <cstdint>

namespace iox
{
class NamedPipeBuilder;

class NamedPipe
{
  public:
    // no system restrictions at all, except available memory. MAX_MESSAGE_SIZE and MAX_NUMBER_OF_MESSAGES can be
    // increased as long as there is enough memory available
    static constexpr uint64_t MAX_MESSAGE_SIZE = build::IOX_MAX_NAMED_PIPE_MESSAGE_SIZE;
    static constexpr uint32_t MAX_NUMBER_OF_MESSAGES = build::IOX_MAX_NAMED_PIPE_NUMBER_OF_MESSAGES;
    static_assert(MAX_NUMBER_OF_MESSAGES < IOX_SEM_VALUE_MAX,
                  "The maximum number of supported messages must be less than the maximum allowed semaphore value");

    static constexpr uint64_t NULL_TERMINATOR_SIZE = 0U;
    static constexpr units::Duration CYCLE_TIME = units::Duration::fromMilliseconds(10);

    // NOLINTJUSTIFICATION used as safe compile time string literal
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    static constexpr const char NAMED_PIPE_PREFIX[] = "iox_np_";

    using Builder_t = NamedPipeBuilder;

    using Message_t = string<MAX_MESSAGE_SIZE>;
    using MessageQueue_t = concurrent::MpmcLockFreeQueue<Message_t, MAX_NUMBER_OF_MESSAGES>;

    NamedPipe() noexcept = delete;
    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;

    NamedPipe(NamedPipe&& rhs) noexcept;
    NamedPipe& operator=(NamedPipe&& rhs) noexcept;
    ~NamedPipe() noexcept;

    /// @brief removes a named pipe artifact from the system
    /// @return true if the artifact was removed, false when no artifact was found and
    ///         PosixIpcChannelError::INTERNAL_LOGIC_ERROR when shm_unlink failed
    static expected<bool, PosixIpcChannelError> unlinkIfExists(const PosixIpcChannelName_t& name) noexcept;

    /// @brief tries to send a message via the named pipe. if the pipe is full PosixIpcChannelError::TIMEOUT is returned
    /// @return on failure an error which describes the failure
    expected<void, PosixIpcChannelError> trySend(const std::string& message) const noexcept;

    /// @brief sends a message via the named pipe. if the pipe is full this call is blocking until the message could be
    ///        delivered
    /// @param[in] message the message which should be sent, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @return success when message was sent otherwise an error which describes the failure
    expected<void, PosixIpcChannelError> send(const std::string& message) const noexcept;

    /// @brief sends a message via the named pipe.
    /// @param[in] message the message which should be sent, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @param[in] timeout the timeout on how long this method should retry to send the message
    /// @return success when message was sent otherwise an error which describes the failure
    expected<void, PosixIpcChannelError> timedSend(const std::string& message,
                                                   const units::Duration& timeout) const noexcept;

    /// @brief tries to receive a message via the named pipe. if the pipe is empty PosixIpcChannelError::TIMEOUT is
    /// returned
    /// @return on success a string containing the message, otherwise an error which describes the failure
    expected<std::string, PosixIpcChannelError> tryReceive() const noexcept;

    /// @brief receives a message via the named pipe. if the pipe is empty this call is blocking until a message was
    ///        received
    /// @return on success a string containing the message, otherwise an error which describes the failure
    expected<std::string, PosixIpcChannelError> receive() const noexcept;

    /// @brief receives a message via the named pipe.
    /// @param[in] timeout the timeout on how long this method should retry to receive a message
    /// @return on success a string containing the message, otherwise an error which describes the failure
    expected<std::string, PosixIpcChannelError> timedReceive(const units::Duration& timeout) const noexcept;

    /// @brief tries to send a message via the named pipe. if the pipe is full PosixIpcChannelError::TIMEOUT is returned
    /// @tparam N capacity of the iox::string
    /// @param[in] message the message to send
    /// @return on failure an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> trySend(const iox::string<N>& message) const noexcept;

    /// @brief sends a message via the named pipe. if the pipe is full this call is blocking until the message could be
    ///        delivered
    /// @tparam N capacity of the iox::string, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @param[in] message the message which should be sent
    /// @return success when message was sent otherwise an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> send(const iox::string<N>& message) const noexcept;

    /// @brief sends a message via the named pipe.
    /// @tparam N capacity of the iox::string, is not allowed to be longer then MAX_MESSAGE_SIZE
    /// @param[in] message the message which should be sent
    /// @param[in] timeout the timeout on how long this method should retry to send the message
    /// @return success when message was sent otherwise an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> timedSend(const iox::string<N>& message,
                                                   const units::Duration& timeout) const noexcept;

    /// @brief tries to receive a message via the named pipe. if the pipe is empty PosixIpcChannelError::TIMEOUT is
    /// returned
    /// @tparam N capacity of the iox::string
    /// @param[in] message the message to receive
    /// @return on success a string containing the message, otherwise an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> tryReceive(iox::string<N>& message) const noexcept;

    /// @brief receives a message via the named pipe. if the pipe is empty this call is blocking until a message was
    ///        received
    /// @tparam N capacity of the iox::string
    /// @param[in] message the message to receive
    /// @return on success a string containing the message, otherwise an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> receive(iox::string<N>& message) const noexcept;

    /// @brief receives a message via the named pipe.
    /// @tparam N capacity of the iox::string
    /// @param[in] message the message to receive
    /// @param[in] timeout the timeout on how long this method should retry to receive a message
    /// @return on success a string containing the message, otherwise an error which describes the failure
    template <uint64_t N>
    expected<void, PosixIpcChannelError> timedReceive(iox::string<N>& message,
                                                      const units::Duration& timeout) const noexcept;

  private:
    friend class NamedPipeBuilder;

    class NamedPipeData;
    NamedPipe(PosixSharedMemoryObject&& sharedMemory, NamedPipeData* data) noexcept;

    template <typename Prefix>
    static PosixIpcChannelName_t mapToSharedMemoryName(const Prefix& p, const PosixIpcChannelName_t& name) noexcept;

    /// @brief destroys an initialized named pipe.
    /// @return is always successful
    expected<void, PosixIpcChannelError> destroy() noexcept;

    class NamedPipeData
    {
      public:
        NamedPipeData() noexcept = default;
        NamedPipeData(const NamedPipeData&) = delete;
        NamedPipeData(NamedPipeData&& rhs) = delete;

        NamedPipeData& operator=(const NamedPipeData&) = delete;
        NamedPipeData& operator=(NamedPipeData&& rhs) = delete;
        ~NamedPipeData() = default;

        UnnamedSemaphore& sendSemaphore() noexcept;
        UnnamedSemaphore& receiveSemaphore() noexcept;

        expected<void, PosixIpcChannelError> initialize(const uint32_t maxMsgNumber) noexcept;

        bool waitForInitialization() const noexcept;
        bool hasValidState() const noexcept;

        MessageQueue_t messages;

      private:
        static constexpr uint64_t INVALID_DATA = 0xBAADF00DAFFEDEAD;
        static constexpr uint64_t VALID_DATA = 0xBAD0FF1CEBEEFBEE;
        static constexpr units::Duration WAIT_FOR_INIT_TIMEOUT = units::Duration::fromSeconds(1);
        static constexpr units::Duration WAIT_FOR_INIT_SLEEP_TIME = units::Duration::fromMilliseconds(1);

        concurrent::Atomic<uint64_t> initializationGuard{INVALID_DATA};
        optional<UnnamedSemaphore> m_sendSemaphore;
        optional<UnnamedSemaphore> m_receiveSemaphore;
    };

  private:
    PosixSharedMemoryObject m_sharedMemory;
    NamedPipeData* m_data = nullptr;
};

class NamedPipeBuilder
{
    /// @brief Defines the named pipe name
    IOX_BUILDER_PARAMETER(PosixIpcChannelName_t, name, "")

    /// @brief Defines how the named pipe is opened, i.e. as client or server
    IOX_BUILDER_PARAMETER(PosixIpcChannelSide, channelSide, PosixIpcChannelSide::CLIENT)

    /// @brief Defines the max message size of the named pipe
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgSize, NamedPipe::MAX_MESSAGE_SIZE)

    /// @brief Defines the max number of messages for the named pipe.
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgNumber, NamedPipe::MAX_NUMBER_OF_MESSAGES)

  public:
    /// @brief create a named pipe
    /// @return On success a 'NamedPipe' is returned and on failure an 'PosixIpcChannelError'.
    expected<NamedPipe, PosixIpcChannelError> create() const noexcept;
};

} // namespace iox

#include "detail/named_pipe.inl"


#endif // IOX_HOOFS_POSIX_IPC_NAMED_PIPE_HPP
