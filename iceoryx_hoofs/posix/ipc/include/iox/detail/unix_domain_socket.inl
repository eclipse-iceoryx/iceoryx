// Copyright 2023, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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
#ifndef IOX_HOOFS_POSIX_IPC_UNIX_DOMAIN_SOCKET_INL
#define IOX_HOOFS_POSIX_IPC_UNIX_DOMAIN_SOCKET_INL

#include "iox/not_null.hpp"
#include "iox/posix_call.hpp"
#include "iox/unix_domain_socket.hpp"

namespace iox
{
template <typename Type, UnixDomainSocket::Termination Terminator>
expected<void, PosixIpcChannelError> UnixDomainSocket::timedSendImpl(not_null<const Type*> msg,
                                                                     uint64_t msgSize,
                                                                     const units::Duration& timeout) const noexcept
{
    if (msgSize > m_maxMessageSize)
    {
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }

    if (PosixIpcChannelSide::SERVER == m_channelSide)
    {
        IOX_LOG(ERROR, "sending on server side not supported for unix domain socket \"" << m_name << "\"");
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = IOX_POSIX_CALL(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return err(errnoToEnum(setsockoptCall.error().errnum));
    }
    uint64_t msgSizeToSend = msgSize;
    if constexpr (Terminator == Termination::NULL_TERMINATOR)
    {
        msgSizeToSend += NULL_TERMINATOR_SIZE;
    }
    auto sendCall = IOX_POSIX_CALL(iox_sendto)(m_sockfd, msg, static_cast<size_t>(msgSizeToSend), 0, nullptr, 0)
                        .failureReturnValue(ERROR_CODE)
                        .evaluate();

    if (sendCall.has_error())
    {
        return err(errnoToEnum(sendCall.error().errnum));
    }
    return ok();
}

template <typename Type, UnixDomainSocket::Termination Terminator>
expected<uint64_t, PosixIpcChannelError> UnixDomainSocket::timedReceiveImpl(
    not_null<Type*> msg, uint64_t maxMsgSize, const units::Duration& timeout) const noexcept
{
    if (PosixIpcChannelSide::CLIENT == m_channelSide)
    {
        IOX_LOG(ERROR, "receiving on client side not supported for unix domain socket \"" << m_name << "\"");
        return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = IOX_POSIX_CALL(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return err(errnoToEnum(setsockoptCall.error().errnum));
    }

    auto recvCall = IOX_POSIX_CALL(iox_recvfrom)(m_sockfd, msg, static_cast<size_t>(maxMsgSize), 0, nullptr, nullptr)
                        .failureReturnValue(ERROR_CODE)
                        .suppressErrorMessagesForErrnos(EAGAIN, EWOULDBLOCK)
                        .evaluate();
    if (recvCall.has_error())
    {
        if constexpr (Terminator == Termination::NULL_TERMINATOR)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            msg[0] = 0;
        }
        return err(errnoToEnum(recvCall.error().errnum));
    }
    const auto receivedMsgLength = static_cast<uint64_t>(recvCall->value);
    if constexpr (Terminator == Termination::NULL_TERMINATOR)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (msg[receivedMsgLength - NULL_TERMINATOR_SIZE] != 0)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            msg[0] = 0;
            return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
        }
        return ok<uint64_t>(receivedMsgLength - NULL_TERMINATOR_SIZE);
    }
    return ok<uint64_t>(receivedMsgLength);
}
template <uint64_t N>
expected<void, PosixIpcChannelError> UnixDomainSocket::send(const iox::string<N>& buf) const noexcept
{
    // we also support timedSend. The setsockopt call sets the timeout for all further sendto calls, so we must set
    // it to 0 to turn the timeout off
    return timedSend(buf, units::Duration::fromSeconds(0ULL));
}

template <uint64_t N>
expected<void, PosixIpcChannelError> UnixDomainSocket::timedSend(const iox::string<N>& buf,
                                                                 const units::Duration& timeout) const noexcept
{
    return timedSendImpl<char, Termination::NULL_TERMINATOR>(buf.c_str(), buf.size(), timeout);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> UnixDomainSocket::receive(iox::string<N>& buf) const noexcept
{
    // we also support timedReceive. The setsockopt call sets the timeout for all further recvfrom calls, so we must set
    // it to 0 to turn the timeout off
    return timedReceive(buf, units::Duration::fromSeconds(0ULL));
}

template <uint64_t N>
expected<void, PosixIpcChannelError> UnixDomainSocket::timedReceive(iox::string<N>& buf,
                                                                    const units::Duration& timeout) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto result = expected<uint64_t, PosixIpcChannelError>(in_place, uint64_t(0));
    buf.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        result = timedReceiveImpl<char, Termination::NULL_TERMINATOR>(str, info.total_size, timeout);
        if (result.has_error())
        {
            return 0;
        }
        return result.value();
    });
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok();
}
} // namespace iox

#endif
