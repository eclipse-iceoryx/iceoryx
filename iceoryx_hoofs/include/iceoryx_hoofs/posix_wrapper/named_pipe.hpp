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
    static constexpr uint64_t MAX_MESSAGE_SIZE = IOX_UDS_SOCKET_MAX_MESSAGE_SIZE;
    static constexpr uint64_t MAX_NUMBER_OF_MESSAGES = 10U;
    static constexpr units::Duration CYCLE_TIME = units::Duration::fromMilliseconds(10);
    static constexpr const char NAMED_PIPE_PREFIX[] = "/";

    using Message_t = cxx::string<MAX_MESSAGE_SIZE>;
    using MessageQueue_t = concurrent::LockFreeQueue<Message_t, MAX_NUMBER_OF_MESSAGES>;

    NamedPipe() noexcept;
    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;

    NamedPipe(NamedPipe&& rhs) noexcept;
    NamedPipe& operator=(NamedPipe&& rhs) noexcept;
    ~NamedPipe() noexcept;

    cxx::expected<IpcChannelError> destroy() noexcept;
    static cxx::expected<bool, IpcChannelError> unlinkIfExists(const IpcChannelName_t& name) noexcept;
    cxx::expected<bool, IpcChannelError> isOutdated() noexcept;

    cxx::expected<IpcChannelError> send(const std::string& message) const noexcept;
    cxx::expected<IpcChannelError> timedSend(const std::string& message, const units::Duration& timeout) const noexcept;
    cxx::expected<std::string, IpcChannelError> receive() const noexcept;
    cxx::expected<std::string, IpcChannelError> timedReceive(const units::Duration& timeout) const noexcept;

  private:
    friend class DesignPattern::Creation<NamedPipe, IpcChannelError>;

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
