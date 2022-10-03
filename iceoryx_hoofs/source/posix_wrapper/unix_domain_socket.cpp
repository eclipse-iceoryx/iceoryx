// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/scope_guard.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/socket.hpp"
#include "iceoryx_platform/unistd.hpp"

#include <string>


namespace iox
{
namespace posix
{
constexpr uint64_t UnixDomainSocket::MAX_MESSAGE_SIZE;
constexpr uint64_t UnixDomainSocket::NULL_TERMINATOR_SIZE;

UnixDomainSocket::UnixDomainSocket() noexcept
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

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
            if (!cxx::isValidPathToFile(name))
            {
                return name;
            }
            return UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).append(iox::cxx::TruncateToCapacity, name);
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
    if (!cxx::isValidPathToFile(name))
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
        std::cerr << "unable to cleanup unix domain socket \"" << m_name << "\" in the destructor" << std::endl;
    }
}

UnixDomainSocket& UnixDomainSocket::operator=(UnixDomainSocket&& other) noexcept
{
    if (this != &other)
    {
        if (destroy().has_error())
        {
            std::cerr << "Unable to cleanup unix domain socket \"" << m_name
                      << "\" in the move constructor/move assingment operator" << std::endl;
        }

        CreationPattern_t::operator=(std::move(other));

        // @todo iox-#1036
        // NOLINTJUSTIFICATION will be fixed with refactoring in #1036
        // NOLINTBEGIN(bugprone-use-after-move, hicpp-invalid-access-moved)
        m_name = std::move(other.m_name);
        m_channelSide = other.m_channelSide;
        m_sockfd = other.m_sockfd;
        m_sockAddr = other.m_sockAddr;
        m_maxMessageSize = other.m_maxMessageSize;
        // NOLINTEND(bugprone-use-after-move, hicpp-invalid-access-moved)

        other.m_sockfd = INVALID_FD;
    }

    return *this;
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const UdsName_t& name) noexcept
{
    if (!cxx::isValidPathToFile(name))
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    if (UdsName_t::capacity() < name.size() + UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).size())
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    return unlinkIfExists(NoPathPrefix,
                          UdsName_t(platform::IOX_UDS_SOCKET_PATH_PREFIX).append(iox::cxx::TruncateToCapacity, name));
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const NoPathPrefix_t,
                                                                      const UdsName_t& name) noexcept
{
    if (!cxx::isValidPathToFile(name))
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    auto unlinkCall = posixCall(unlink)(name.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate();

    if (!unlinkCall.has_error())
    {
        // ENOENT is set if this socket is not known
        return cxx::success<bool>(unlinkCall->errnum != ENOENT);
    }
    return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
}

cxx::expected<IpcChannelError> UnixDomainSocket::closeFileDescriptor() noexcept
{
    if (m_sockfd != INVALID_FD)
    {
        auto closeCall = posixCall(iox_closesocket)(m_sockfd).failureReturnValue(ERROR_CODE).evaluate();

        if (!closeCall.has_error())
        {
            if (IpcChannelSide::SERVER == m_channelSide)
            {
                unlink(&(m_sockAddr.sun_path[0]));
            }

            m_sockfd = INVALID_FD;
            m_isInitialized = false;

            return cxx::success<void>();
        }
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(closeCall.get_error().errnum));
    }
    return cxx::success<>();
}

cxx::expected<IpcChannelError> UnixDomainSocket::destroy() noexcept
{
    if (m_isInitialized)
    {
        return closeFileDescriptor();
    }

    return cxx::success<void>();
}

cxx::expected<IpcChannelError> UnixDomainSocket::send(const std::string& msg) const noexcept
{
    // we also support timedSend. The setsockopt call sets the timeout for all further sendto calls, so we must set
    // it to 0 to turn the timeout off
    return timedSend(msg, units::Duration::fromSeconds(0ULL));
}

cxx::expected<IpcChannelError> UnixDomainSocket::timedSend(const std::string& msg,
                                                           const units::Duration& timeout) const noexcept
{
    if (msg.size() > m_maxMessageSize)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    if (IpcChannelSide::SERVER == m_channelSide)
    {
        std::cerr << "sending on server side not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
    }
    auto sendCall = posixCall(iox_sendto)(m_sockfd, msg.c_str(), msg.size() + NULL_TERMINATOR_SIZE, 0, nullptr, 0)
                        .failureReturnValue(ERROR_CODE)
                        .evaluate();

    if (sendCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(sendCall.get_error().errnum));
    }
    return cxx::success<void>();
}

cxx::expected<std::string, IpcChannelError> UnixDomainSocket::receive() const noexcept
{
    // we also support timedReceive. The setsockopt call sets the timeout for all further recvfrom calls, so we must set
    // it to 0 to turn the timeout off
    struct timeval tv = {};
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return timedReceive(units::Duration(tv));
}

cxx::expected<std::string, IpcChannelError>
UnixDomainSocket::timedReceive(const units::Duration& timeout) const noexcept
{
    if (IpcChannelSide::CLIENT == m_channelSide)
    {
        std::cerr << "receiving on client side not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    auto tv = timeout.timeval();
    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
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
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(recvCall.get_error().errnum));
    }
    return cxx::success<std::string>(message);
}

cxx::expected<IpcChannelError> UnixDomainSocket::initalizeSocket() noexcept
{
    // initialize the sockAddr data structure with the provided name
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));
    m_sockAddr.sun_family = AF_LOCAL;
    if (m_name.size() > LONGEST_VALID_NAME)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    strncpy(&(m_sockAddr.sun_path[0]), m_name.c_str(), m_name.size());

    // the mask will be applied to the permissions, we only allow users and group members to have read and write access
    // the system call always succeeds, no need to check for errors
    // NOLINTJUSTIFICATION type is defined by POSIX, no logical fault
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    mode_t umaskSaved = umask(S_IXUSR | S_IXGRP | S_IRWXO);
    // Reset to old umask when going out of scope
    cxx::ScopeGuard umaskGuard([&] { umask(umaskSaved); });

    auto socketCall = posixCall(iox_socket)(AF_LOCAL, SOCK_DGRAM, 0)
                          .failureReturnValue(ERROR_CODE)
                          .evaluate()
                          .and_then([this](auto& r) { m_sockfd = r.value; });

    if (socketCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(socketCall.get_error().errnum));
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
            return cxx::success<>();
        }
        closeFileDescriptor().or_else([](auto) {
            std::cerr << "Unable to close socket file descriptor in error related cleanup during initialization."
                      << std::endl;
        });
        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(bindCall.get_error().errnum));
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
            std::cerr << "Unable to close socket file descriptor in error related cleanup during initialization."
                      << std::endl;
        });
        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(connectCall.get_error().errnum));
    }
    return cxx::success<>();
}

IpcChannelError UnixDomainSocket::convertErrnoToIpcChannelError(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to create unix domain socket denied \"" << m_name << "\"" << std::endl;
        return IpcChannelError::ACCESS_DENIED;
    }
    case EAFNOSUPPORT:
    {
        std::cerr << "address family not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::PROCESS_LIMIT;
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::SYSTEM_LIMIT;
    }
    case ENOBUFS:
    {
        std::cerr << "queue is full for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::OUT_OF_MEMORY;
    }
    case ENOMEM:
    {
        std::cerr << "out of memory for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::OUT_OF_MEMORY;
    }
    case EPROTONOSUPPORT:
    {
        std::cerr << "protocol type not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case EADDRINUSE:
    {
        std::cerr << "unix domain socket already in use \"" << m_name << "\"" << std::endl;
        return IpcChannelError::CHANNEL_ALREADY_EXISTS;
    }
    case EBADF:
    {
        std::cerr << "invalid file descriptor for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_FILE_DESCRIPTOR;
    }
    case ENOTSOCK:
    {
        std::cerr << "invalid unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_FILE_DESCRIPTOR;
    }
    case EADDRNOTAVAIL:
    {
        std::cerr << "interface or address error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENAMETOOLONG:
    {
        std::cerr << "name too long for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENOTDIR:
    {
        std::cerr << "not a directory error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::NO_SUCH_CHANNEL;
    }
    case EROFS:
    {
        std::cerr << "read only error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_CHANNEL_NAME;
    }
    case EIO:
    {
        std::cerr << "I/O for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::I_O_ERROR;
    }
    case ENOPROTOOPT:
    {
        std::cerr << "invalid option for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError::INVALID_ARGUMENTS;
    }
    case ECONNREFUSED:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::NO_SUCH_CHANNEL;
    }
    case ECONNRESET:
    {
        std::cerr << "connection was reset by peer for \"" << m_name << "\"" << std::endl;
        return IpcChannelError::CONNECTION_RESET_BY_PEER;
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError::TIMEOUT;
    }
    default:
    {
        std::cerr << "internal logic error in unix domain socket \"" << m_name << "\" occurred" << std::endl;
        return IpcChannelError::INTERNAL_LOGIC_ERROR;
    }
    }
}
} // namespace posix
} // namespace iox
