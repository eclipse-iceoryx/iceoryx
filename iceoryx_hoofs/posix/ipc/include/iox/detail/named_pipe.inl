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

    auto hasData = m_data->sendSemaphore().tryWait().expect("'tryWait' on a semaphore should always be successful");
    if (hasData)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(message));
        m_data->receiveSemaphore().post().expect("'post' on a semaphore should always be successful");
        return ok();
    }
    return err(PosixIpcChannelError::TIMEOUT);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::send(const iox::string<N>& message) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    m_data->sendSemaphore().wait().expect("'wait' on a semaphore should always be successful");
    IOX_DISCARD_RESULT(m_data->messages.push(message));
    m_data->receiveSemaphore().post().expect("'post' on a semaphore should always be successful");

    return ok();
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::timedSend(const iox::string<N>& message,
                                                          const units::Duration& timeout) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto waitState =
        m_data->sendSemaphore().timedWait(timeout).expect("'timedWait' on a semaphore should always be successful");

    if (waitState == SemaphoreWaitState::NO_TIMEOUT)
    {
        IOX_DISCARD_RESULT(m_data->messages.push(message));
        m_data->receiveSemaphore().post().expect("'post' on a semaphore should always be successful");
        return ok();
    }
    return err(PosixIpcChannelError::TIMEOUT);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::receive(iox::string<N>& message) const noexcept
{
    m_data->receiveSemaphore().wait().expect("'wait' on a semaphore should always be successful");
    auto msg = m_data->messages.pop();
    if (msg.has_value())
    {
        m_data->sendSemaphore().post().expect("'post' on a semaphore should always be successful");
        message = *msg;
        return ok();
    }
    return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> NamedPipe::tryReceive(iox::string<N>& message) const noexcept
{
    auto hasData = m_data->receiveSemaphore().tryWait().expect("'tryWait' on a semaphore should always be successful");
    if (hasData)
    {
        auto msg = m_data->messages.pop();
        if (msg.has_value())
        {
            m_data->sendSemaphore().post().expect("'post' on a semaphore should always be successful");
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
    auto waitState =
        m_data->receiveSemaphore().timedWait(timeout).expect("'timedWait' on a semaphore should always be successful");

    if (waitState == SemaphoreWaitState::NO_TIMEOUT)
    {
        auto msg = m_data->messages.pop();
        if (msg.has_value())
        {
            m_data->sendSemaphore().post().expect("'post' on a semaphore should always be successful");
            message = *msg;
            return ok();
        }
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    return err(PosixIpcChannelError::TIMEOUT);
}
} // namespace iox

#endif
