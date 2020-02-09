// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/socket.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <chrono>
#include <cstdlib>

namespace iox
{
namespace posix
{
UnixDomainSocket::UnixDomainSocket()
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

UnixDomainSocket::UnixDomainSocket(const std::string& name,
                                   const IpcChannelMode mode,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t /*maxMsgNumber*/)
    : m_name(name)
    , m_channelSide(channelSide)
{
    if (maxMsgSize > MAX_MESSAGE_SIZE)
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
    }
    else
    {
        memset(&m_sockAddr, 0, sizeof(m_sockAddr));
        m_sockAddr.sun_family = AF_LOCAL;
        strcpy(m_sockAddr.sun_path, name.c_str());

        auto createResult = createSocket(mode);

        if (!createResult.has_error())
        {
            this->m_isInitialized = true;
            this->m_errorValue = IpcChannelError::UNDEFINED;
            this->m_sockfd = createResult.get_value();
        }
        else
        {
            this->m_isInitialized = false;
            this->m_errorValue = createResult.get_error();
        }
    }
}

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& other)
{
    *this = std::move(other);
}

UnixDomainSocket::~UnixDomainSocket()
{
    if (destroy().has_error())
    {
        std::cerr << "unable to cleanup unix domain socket \"" << m_name << "\" in the destructor" << std::endl;
    }
}

UnixDomainSocket& UnixDomainSocket::operator=(UnixDomainSocket&& other)
{
    if (this != &other)
    {
        m_name = std::move(other.m_name);
        m_channelSide = std::move(other.m_channelSide);
        m_sockfd = std::move(other.m_sockfd);
        m_sockAddr = std::move(other.m_sockAddr);
        other.m_sockfd = INVALID_FD;
    }

    return *this;
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::exists(const std::string& name)
{
    return cxx::success<bool>(true);
}

cxx::expected<IpcChannelError> UnixDomainSocket::destroy()
{
    if (m_sockfd != INVALID_FD)
    {
        auto closeCall = cxx::makeSmartC(
            closePlatformFileHandle, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_sockfd);

        if (!closeCall.hasErrors())
        {
            if (IpcChannelSide::SERVER == m_channelSide)
            {
                unlink(m_name.c_str());
            }

            m_sockfd = INVALID_FD;

            return cxx::success<void>();
        }
        else
        {
            return createErrorFromErrnum(closeCall.getErrNum());
        }
    }

    return cxx::success<void>();
}

cxx::expected<IpcChannelError> UnixDomainSocket::send(const std::string& msg)
{
    // we also support timedSend. The setsockopt call sets the timeout for all further sendto calls, so we must set
    // it to 0 to turn the timeout off
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return timedSend(msg, units::Duration(tv));
}

cxx::expected<IpcChannelError> UnixDomainSocket::timedSend(const std::string& msg, const units::Duration& timeout)
{
    if (msg.size() > MAX_MESSAGE_SIZE)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    if (IpcChannelSide::SERVER == m_channelSide)
    {
        std::cerr << "sending on server side not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    struct timeval tv = timeout;

    auto setsockoptCall = cxx::makeSmartC(setsockopt,
                                          cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                          {static_cast<ssize_t>(ERROR_CODE)},
                                          {EWOULDBLOCK},
                                          m_sockfd,
                                          SOL_SOCKET,
                                          SO_SNDTIMEO,
                                          (const char*)&tv,
                                          sizeof(tv));

    if (setsockoptCall.hasErrors())
    {
        return createErrorFromErrnum(setsockoptCall.getErrNum());
    }
    else
    {
        /// @todo extend createErrorFromErrnum with possible sendto errors
        auto sendCall = cxx::makeSmartC(sendto,
                                        cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                        {ERROR_CODE},
                                        {},
                                        m_sockfd,
                                        msg.c_str(),
                                        msg.size() + 1, // +1 for the \0 at the end
                                        0,
                                        (struct sockaddr*)&m_sockAddr,
                                        sizeof(m_sockAddr));

        if (sendCall.hasErrors())
        {
            return createErrorFromErrnum(sendCall.getErrNum());
        }
        else
        {
            return cxx::success<void>();
        }
    }
}

cxx::expected<std::string, IpcChannelError> UnixDomainSocket::receive()
{
    // we also support timedReceive. The setsockopt call sets the timeout for all further recvfrom calls, so we must set
    // it to 0 to turn the timeout off
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return timedReceive(units::Duration(tv));
}


cxx::expected<std::string, IpcChannelError> UnixDomainSocket::timedReceive(const units::Duration& timeout)
{
    if (IpcChannelSide::CLIENT == m_channelSide)
    {
        std::cerr << "receiving on client side not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    struct timeval tv = timeout;

    auto setsockoptCall = cxx::makeSmartC(setsockopt,
                                          cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                          {static_cast<ssize_t>(ERROR_CODE)},
                                          {EWOULDBLOCK},
                                          m_sockfd,
                                          SOL_SOCKET,
                                          SO_RCVTIMEO,
                                          (const char*)&tv,
                                          sizeof(tv));

    if (setsockoptCall.hasErrors())
    {
        return createErrorFromErrnum(setsockoptCall.getErrNum());
    }
    else
    {
        char message[MAX_MESSAGE_SIZE + 1]; // +1 for the \0 at the end

        /// @todo extend createErrorFromErrnum with possible recvfrom errors
        auto recvCall = cxx::makeSmartC(recvfrom,
                                        cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                        {static_cast<ssize_t>(ERROR_CODE)},
                                        {},
                                        m_sockfd,
                                        &(message[0]),
                                        MAX_MESSAGE_SIZE,
                                        0,
                                        nullptr,
                                        nullptr);

        if (recvCall.hasErrors())
        {
            // timeout is also an error and would return here
            return createErrorFromErrnum(recvCall.getErrNum());
        }
        else
        {
            return cxx::success<std::string>(std::string(&(message[0])));
        }
    }
}


cxx::expected<int, IpcChannelError> UnixDomainSocket::createSocket(const IpcChannelMode /*mode*/)
{
    /// @todo mode handling
    if (m_name.empty())
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    auto socketCall =
        cxx::makeSmartC(socket, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, AF_LOCAL, SOCK_DGRAM, 0);

    if (!socketCall.hasErrors())
    {
        int sockfd = socketCall.getReturnValue();

        if (IpcChannelSide::SERVER == m_channelSide)
        {
            unlink(m_name.c_str());

            auto bindCall = cxx::makeSmartC(bind,
                                            cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                            {ERROR_CODE},
                                            {},
                                            sockfd,
                                            (struct sockaddr*)&m_sockAddr,
                                            sizeof(m_sockAddr));

            if (!bindCall.hasErrors())
            {
                return cxx::success<int>(sockfd);
            }
            else
            {
                return createErrorFromErrnum(bindCall.getErrNum());
            }
        }
        else
        {
            return cxx::success<int>(sockfd);
        }
    }
    else
    {
        return createErrorFromErrnum(socketCall.getErrNum());
    }
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::isOutdated()
{
    /// @todo check if this can be used for unix domain sockets too

    struct stat sb;
    /// @todo extend createErrorFromErrnum with possible fstat errors
    auto fstatCall = cxx::makeSmartC(fstat, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_sockfd, &sb);
    if (fstatCall.hasErrors())
    {
        return createErrorFromErrnum(fstatCall.getErrNum());
    }
    return cxx::success<bool>(sb.st_nlink == 0);
}


cxx::error<IpcChannelError> UnixDomainSocket::createErrorFromErrnum(const int errnum)
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to create unix domain socket denied \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::ACCESS_DENIED);
    }
    case EAFNOSUPPORT:
    {
        std::cerr << "address family not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::PROCESS_LIMIT);
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::SYSTEM_LIMIT);
    }
    case ENOBUFS:
    {
        std::cerr << "out of memory for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::OUT_OF_MEMORY);
    }
    case ENOMEM:
    {
        std::cerr << "out of memory for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::OUT_OF_MEMORY);
    }
    case EPROTONOSUPPORT:
    {
        std::cerr << "protocol type not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EADDRINUSE:
    {
        std::cerr << "unix domain socket already in use \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::CHANNEL_ALREADY_EXISTS);
    }
    case EBADF:
    {
        std::cerr << "invalid file descriptor for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_FILE_DESCRIPTOR);
    }
    case ENOTSOCK:
    {
        std::cerr << "invalid file descriptor for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_FILE_DESCRIPTOR);
    }
    case EADDRNOTAVAIL:
    {
        std::cerr << "interface or address error for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENAMETOOLONG:
    {
        std::cerr << "name too long for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENOTDIR:
    {
        std::cerr << "not a directory error for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENOENT:
    {
        std::cerr << "directory prefix error for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case EROFS:
    {
        std::cerr << "read only error for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case EIO:
    {
        std::cerr << "I/O for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::I_O_ERROR);
    }
    case ENOPROTOOPT:
    {
        std::cerr << "invalid option for unix domain socket \"" << m_name << "\"" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
    }
    default:
    {
        std::cerr << "internal logic error in message queue \"" << m_name << "\" occurred" << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    }
}

} // namespace posix
} // namespace iox
