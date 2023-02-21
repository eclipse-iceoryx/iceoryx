// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/socket.hpp"
#include "iceoryx_platform/unistd.hpp"
#include "iox/logging.hpp"
#include "iox/scope_guard.hpp"

#include <chrono>
#include <string>


namespace iox
{
namespace posix
{
constexpr uint64_t UnixDomainSocket::MAX_MESSAGE_SIZE;
constexpr uint64_t UnixDomainSocket::NULL_TERMINATOR_SIZE;

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& other) noexcept
{
    *this = std::move(other);
}

// @todo iox-#832
// NOLINTJUSTIFICATION make a struct out of arguments in #832
// NOLINTBEGIN(readability-function-size, bugprone-easily-swappable-parameters)
UnixDomainSocket::UnixDomainSocket(const IpcChannelName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t maxMsgNumber) noexcept
    // NOLINTEND(readability-function-size, bugprone-easily-swappable-parameters)
    : UnixDomainSocket(
        NoPathPrefix,
        [&]() -> UdsName_t {
            /// invalid names will be forwarded and handled by the other constructor
            /// separately
            if (!isValidPathToFile(name))
            {
                return name;
            }
            return UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).append(iox::TruncateToCapacity, name);
        }(),
        channelSide,
        maxMsgSize,
        maxMsgNumber)
{
}

// @todo iox-#832
// NOLINTJUSTIFICATION make a struct out of arguments in #832
// NOLINTBEGIN(readability-function-size, bugprone-easily-swappable-parameters)
UnixDomainSocket::UnixDomainSocket(const NoPathPrefix_t,
                                   const UdsName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t) noexcept
    // NOLINTEND(readability-function-size, bugprone-easily-swappable-parameters)
    : m_name(name)
    , m_channelSide(channelSide)
{
    if (!isValidPathToFile(name))
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        return;
    }

    if (maxMsgSize > MAX_MESSAGE_SIZE)
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
    }
    else
    {
        m_maxMessageSize = maxMsgSize;
        initalizeSocket().and_then([this]() { this->m_isInitialized = true; }).or_else([this](IpcChannelError& error) {
            this->m_isInitialized = false;
            this->m_errorValue = error;
        });
    }
}

UnixDomainSocket::~UnixDomainSocket() noexcept
{
    if (destroy().has_error())
    {
        IOX_LOG(ERROR) << "unable to cleanup unix domain socket \"" << m_name << "\" in the destructor";
    }
}

UnixDomainSocket& UnixDomainSocket::operator=(UnixDomainSocket&& other) noexcept
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            IOX_LOG(ERROR) << "Unable to cleanup unix domain socket \"" << m_name
                           << "\" in the move constructor/move assignment operator";
        }

        m_isInitialized = other.m_isInitialized;
        m_errorValue = other.m_errorValue;
        m_name = std::move(other.m_name);
        m_channelSide = other.m_channelSide;
        m_sockfd = other.m_sockfd;
        m_sockAddr = other.m_sockAddr;
        m_maxMessageSize = other.m_maxMessageSize;

        other.m_isInitialized = false;
        other.m_sockfd = INVALID_FD;
    }

    return *this;
}

bool UnixDomainSocket::isInitialized() const noexcept
{
    return m_isInitialized;
}

expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const UdsName_t& name) noexcept
{
    if (!isValidPathToFile(name))
    {
        return error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    if (UdsName_t::capacity() < name.size() + UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).size())
    {
        return error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    return unlinkIfExists(NoPathPrefix,
                          UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).append(iox::TruncateToCapacity, name));
}

expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const NoPathPrefix_t, const UdsName_t& name) noexcept
{
    if (!isValidPathToFile(name))
    {
        return error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    auto unlinkCall = posixCall(unlink)(name.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate();

    if (!unlinkCall.has_error())
    {
        // ENOENT is set if this socket is not known
        return success<bool>(unlinkCall->errnum != ENOENT);
    }
    return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
}

expected<IpcChannelError> UnixDomainSocket::closeFileDescriptor() noexcept
{
    if (m_sockfd != INVALID_FD)
    {
        auto closeCall = posixCall(iox_closesocket)(m_sockfd).failureReturnValue(ERROR_CODE).evaluate();

        if (!closeCall.has_error())
        {
            if (IpcChannelSide::SERVER == m_channelSide)
            {
                auto unlinkCall = posixCall(unlink)(&(m_sockAddr.sun_path[0]))
                                      .failureReturnValue(ERROR_CODE)
                                      .ignoreErrnos(ENOENT)
                                      .evaluate();
                if (unlinkCall.has_error())
                {
                    return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
                }
            }

            m_sockfd = INVALID_FD;
            m_isInitialized = false;

            return success<void>();
        }
        return error<IpcChannelError>(convertErrnoToIpcChannelError(closeCall.get_error().errnum));
    }
    return success<>();
}

expected<IpcChannelError> UnixDomainSocket::destroy() noexcept
{
    if (m_isInitialized)
    {
        return closeFileDescriptor();
    }

    return success<void>();
}

expected<IpcChannelError> UnixDomainSocket::send(const std::string& msg) const noexcept
{
    // we also support timedSend. The setsockopt call sets the timeout for all further sendto calls, so we must set
    // it to 0 to turn the timeout off
    return timedSend(msg, units::Duration::fromSeconds(0ULL));
}

expected<IpcChannelError> UnixDomainSocket::timedSend(const std::string& msg,
                                                      const units::Duration& timeout) const noexcept
{
    if (msg.size() > m_maxMessageSize)
    {
        return error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    if (IpcChannelSide::SERVER == m_channelSide)
    {
        IOX_LOG(ERROR) << "sending on server side not supported for unix domain socket \"" << m_name << "\"";
        return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
    }
    auto sendCall = posixCall(iox_sendto)(m_sockfd, msg.c_str(), msg.size() + NULL_TERMINATOR_SIZE, 0, nullptr, 0)
                        .failureReturnValue(ERROR_CODE)
                        .evaluate();

    if (sendCall.has_error())
    {
        return error<IpcChannelError>(convertErrnoToIpcChannelError(sendCall.get_error().errnum));
    }
    return success<void>();
}

expected<std::string, IpcChannelError> UnixDomainSocket::receive() const noexcept
{
    // we also support timedReceive. The setsockopt call sets the timeout for all further recvfrom calls, so we must set
    // it to 0 to turn the timeout off
    struct timeval tv = {};
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return timedReceive(units::Duration(tv));
}

expected<std::string, IpcChannelError> UnixDomainSocket::timedReceive(const units::Duration& timeout) const noexcept
{
    if (IpcChannelSide::CLIENT == m_channelSide)
    {
        IOX_LOG(ERROR) << "receiving on client side not supported for unix domain socket \"" << m_name << "\"";
        return error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
    }
    // NOLINTJUSTIFICATION needed for recvfrom
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char message[MAX_MESSAGE_SIZE + 1];

    auto recvCall = posixCall(iox_recvfrom)(m_sockfd, &message[0], MAX_MESSAGE_SIZE, 0, nullptr, nullptr)
                        .failureReturnValue(ERROR_CODE)
                        .suppressErrorMessagesForErrnos(EAGAIN, EWOULDBLOCK)
                        .evaluate();
    message[MAX_MESSAGE_SIZE] = 0;

    if (recvCall.has_error())
    {
        return error<IpcChannelError>(convertErrnoToIpcChannelError(recvCall.get_error().errnum));
    }
    return success<std::string>(&message[0]);
}

expected<IpcChannelError> UnixDomainSocket::initalizeSocket() noexcept
{
    // initialize the sockAddr data structure with the provided name
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));
    m_sockAddr.sun_family = AF_LOCAL;
    if (m_name.size() > LONGEST_VALID_NAME)
    {
        return error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    strncpy(&(m_sockAddr.sun_path[0]), m_name.c_str(), m_name.size());

    // the mask will be applied to the permissions, we only allow users and group members to have read and write access
    // the system call always succeeds, no need to check for errors
    // NOLINTJUSTIFICATION type is defined by POSIX, no logical fault
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    mode_t umaskSaved = umask(S_IXUSR | S_IXGRP | S_IRWXO);
    // Reset to old umask when going out of scope
    ScopeGuard umaskGuard([&] { umask(umaskSaved); });

    auto socketCall = posixCall(iox_socket)(AF_LOCAL, SOCK_DGRAM, 0)
                          .failureReturnValue(ERROR_CODE)
                          .evaluate()
                          .and_then([this](auto& r) { m_sockfd = r.value; });

    if (socketCall.has_error())
    {
        return error<IpcChannelError>(convertErrnoToIpcChannelError(socketCall.get_error().errnum));
    }

    if (IpcChannelSide::SERVER == m_channelSide)
    {
        unlink(&(m_sockAddr.sun_path[0]));

        auto bindCall =
            // NOLINTJUSTIFICATION enforced by POSIX API
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            posixCall(iox_bind)(m_sockfd, reinterpret_cast<struct sockaddr*>(&m_sockAddr), sizeof(m_sockAddr))
                .failureReturnValue(ERROR_CODE)
                .evaluate();

        if (!bindCall.has_error())
        {
            return success<>();
        }
        closeFileDescriptor().or_else([](auto) {
            IOX_LOG(ERROR) << "Unable to close socket file descriptor in error related cleanup during initialization.";
        });
        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return error<IpcChannelError>(convertErrnoToIpcChannelError(bindCall.get_error().errnum));
    }
    // we use a connected socket, this leads to a behavior closer to the message queue (e.g. error if client
    // is created and server not present)
    auto connectCall =
        // NOLINTJUSTIFICATION enforced by POSIX API
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        posixCall(iox_connect)(m_sockfd, reinterpret_cast<struct sockaddr*>(&m_sockAddr), sizeof(m_sockAddr))
            .failureReturnValue(ERROR_CODE)
            .suppressErrorMessagesForErrnos(ENOENT, ECONNREFUSED)
            .evaluate();

    if (connectCall.has_error())
    {
        closeFileDescriptor().or_else([](auto) {
            IOX_LOG(ERROR) << "Unable to close socket file descriptor in error related cleanup during initialization.";
        });
        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return error<IpcChannelError>(convertErrnoToIpcChannelError(connectCall.get_error().errnum));
    }
    return success<>();
}

IpcChannelError UnixDomainSocket::convertErrnoToIpcChannelError(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        IOX_LOG(ERROR) << "permission to create unix domain socket denied \"" << m_name << "\"";
        return IpcChannelError::ACCESS_DENIED;
    }
    case EAFNOSUPPORT:
    {
        IOX_LOG(ERROR) << "address family not supported for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EINVAL:
    {
        IOX_LOG(ERROR) << "provided invalid arguments for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EMFILE:
    {
        IOX_LOG(ERROR) << "process limit reached for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::PROCESS_LIMIT;
    }
    case ENFILE:
    {
        IOX_LOG(ERROR) << "system limit reached for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::SYSTEM_LIMIT;
    }
    case ENOBUFS:
    {
        IOX_LOG(ERROR) << "queue is full for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::OUT_OF_MEMORY;
    }
    case ENOMEM:
    {
        IOX_LOG(ERROR) << "out of memory for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::OUT_OF_MEMORY;
    }
    case EPROTONOSUPPORT:
    {
        IOX_LOG(ERROR) << "protocol type not supported for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EADDRINUSE:
    {
        IOX_LOG(ERROR) << "unix domain socket already in use \"" << m_name << "\"";
        return IpcChannelError::CHANNEL_ALREADY_EXISTS;
    }
    case EBADF:
    {
        IOX_LOG(ERROR) << "invalid file descriptor for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_FILE_DESCRIPTOR;
    }
    case ENOTSOCK:
    {
        IOX_LOG(ERROR) << "invalid unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_FILE_DESCRIPTOR;
    }
    case EADDRNOTAVAIL:
    {
        IOX_LOG(ERROR) << "interface or address error for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case EFAULT:
    {
        IOX_LOG(ERROR) << "outside address space error for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ELOOP:
    {
        IOX_LOG(ERROR) << "too many symbolic links for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENAMETOOLONG:
    {
        IOX_LOG(ERROR) << "name too long for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENOTDIR:
    {
        IOX_LOG(ERROR) << "not a directory error for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::NO_SUCH_CHANNEL;
    }
    case EROFS:
    {
        IOX_LOG(ERROR) << "read only error for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case EIO:
    {
        IOX_LOG(ERROR) << "I/O for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::I_O_ERROR;
    }
    case ENOPROTOOPT:
    {
        IOX_LOG(ERROR) << "invalid option for unix domain socket \"" << m_name << "\"";
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case ECONNREFUSED:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::NO_SUCH_CHANNEL;
    }
    case ECONNRESET:
    {
        IOX_LOG(ERROR) << "connection was reset by peer for \"" << m_name << "\"";
        return IpcChannelError::CONNECTION_RESET_BY_PEER;
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::TIMEOUT;
    }
    default:
    {
        IOX_LOG(ERROR) << "internal logic error in unix domain socket \"" << m_name << "\" occurred";
        return IpcChannelError::INTERNAL_LOGIC_ERROR;
    }
    }
}
} // namespace posix
} // namespace iox
