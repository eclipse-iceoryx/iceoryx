// Copyright 2024, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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
#ifndef IOX_HOOFS_POSIX_IPC_NAMED_PIPE_INL
#define IOX_HOOFS_POSIX_IPC_NAMED_PIPE_INL

#include "iox/duration.hpp"
#include "iox/into.hpp"
#include "iox/named_pipe.hpp"

namespace iox
{
template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::trySend(const iox::string<N>& message) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto result = m_data->sendSemaphore().tryWait();
    IOX_EXPECTS(!result.has_error());

    if (*result)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(message));
        IOX_EXPECTS(!m_data->receiveSemaphore().post().has_error());
        return ok();
    }
    return err(PosixIpcChannelError::TIMEOUT);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::send(const iox::string<N>& message) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    IOX_EXPECTS(!m_data->sendSemaphore().wait().has_error());
    IOX_DISCARD_RESULT(m_data->messages.push(message));
    IOX_EXPECTS(!m_data->receiveSemaphore().post().has_error());

    return ok();
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::timedSend(const iox::string<N>& message,
                                                          const units::Duration& timeout) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto result = m_data->sendSemaphore().timedWait(timeout);
    IOX_EXPECTS(!result.has_error());

    if (*result == SemaphoreWaitState::NO_TIMEOUT)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(message));
        IOX_EXPECTS(!m_data->receiveSemaphore().post().has_error());
        return ok();
    }
    return err(PosixIpcChannelError::TIMEOUT);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::receive(iox::string<N>& message) const noexcept
{
    IOX_EXPECTS(!m_data->receiveSemaphore().wait().has_error());
    auto msg = m_data->messages.pop();
    if (msg.has_value())
    {
        IOX_EXPECTS(!m_data->sendSemaphore().post().has_error());
        message = *msg;
        return ok();
    }
    return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::tryReceive(iox::string<N>& message) const noexcept
{
    auto result = m_data->receiveSemaphore().tryWait();
    IOX_EXPECTS(!result.has_error());

    if (*result)
    {
        auto msg = m_data->messages.pop();
        if (msg.has_value())
        {
            IOX_EXPECTS(!m_data->sendSemaphore().post().has_error());
            message = *msg;
        }
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    return err(PosixIpcChannelError::TIMEOUT);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::timedReceive(iox::string<N>& message,
                                                             const units::Duration& timeout) const noexcept
{
    auto result = m_data->receiveSemaphore().timedWait(timeout);
    IOX_EXPECTS(!result.has_error());

    if (*result == SemaphoreWaitState::NO_TIMEOUT)
    {
        auto msg = m_data->messages.pop();
        if (msg.has_value())
        {
            IOX_EXPECTS(!m_data->sendSemaphore().post().has_error());
            message = *msg;
            return ok();
        }
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    return err(PosixIpcChannelError::TIMEOUT);
}
} // namespace iox

#endif