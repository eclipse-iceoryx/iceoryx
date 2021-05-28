// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/platform/socket.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <chrono>
#include <cstdlib>
#include <string>


namespace iox
{
namespace posix
{
constexpr char UnixDomainSocket::PATH_PREFIX[];

#ifdef _WIN32
NamedPipe::NamedPipe(const UnixDomainSocket::UdsName_t& name,
                     uint64_t maxMessageSize,
                     const uint64_t maxNumberOfMessages) noexcept
{
    const DWORD inputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD outputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD openMode = PIPE_ACCESS_DUPLEX;
    const DWORD pipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    const DWORD noTimeout = 0;
    const LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    m_handle = CreateNamedPipeA(name.c_str(),
                                openMode,
                                pipeMode,
                                PIPE_UNLIMITED_INSTANCES,
                                outputBufferSize,
                                inputBufferSize,
                                noTimeout,
                                noSecurityAttributes);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        __PrintLastErrorToConsole("", "", 0);
        printf("Server CreateNamedPipe failed, GLE=%d.\n", GetLastError());
    }

    ConnectNamedPipe(m_handle, NULL);
}

NamedPipe::NamedPipe(NamedPipe&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipe::~NamedPipe() noexcept
{
    destroy();
}

NamedPipe& NamedPipe::operator=(NamedPipe&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();

        m_handle = rhs.m_handle;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }
    return *this;
}

void NamedPipe::destroy() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(m_handle);
        DisconnectNamedPipe(m_handle);
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

NamedPipe::operator bool() const noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}
#endif

UnixDomainSocket::UnixDomainSocket() noexcept
{
    this->m_isInitialized = false;
    this->m_errorValue = IpcChannelError::NOT_INITIALIZED;
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::isOutdated() noexcept
{
    // This is for being API compatible with the message queue, but has no equivalent for socket.
    // We return false to say that the socket is not outdated. If there is a problem,
    // we rely on the other calls and their error returns

    return cxx::success<bool>(false);
}

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& other) noexcept
{
    *this = std::move(other);
}

bool UnixDomainSocket::isNameValid(const UdsName_t& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    for (uint64_t i = 0; i < name.size(); ++i)
    {
        if (!((65 <= name.c_str()[i] && name.c_str()[i] <= 90) ||  // A-Z
              (97 <= name.c_str()[i] && name.c_str()[i] <= 122) || // a-z
              (48 <= name.c_str()[i] && name.c_str()[i] <= 57) ||  // 0-9
              name.c_str()[i] == 45 ||                             // -
              name.c_str()[i] == 46 ||                             // .
              name.c_str()[i] == 95                                // _
              ))
        {
            return false;
        }
    }

    return true;
}

#if defined(_WIN32)
UnixDomainSocket::UnixDomainSocket(const IpcChannelName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t maxMsgNumber) noexcept
    : UnixDomainSocket(NoPathPrefix, name, channelSide, maxMsgSize, maxMsgNumber)
{
}

UnixDomainSocket::UnixDomainSocket(const NoPathPrefix_t,
                                   const UdsName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t maxMsgNumber) noexcept
    : m_maxMessageSize(maxMsgSize)
    , m_numberOfPipes(maxMsgNumber)
    , m_channelSide(channelSide)
{
    if (!isNameValid(name))
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::INVALID_CHANNEL_NAME;
        return;
    }

    if (maxMsgSize > MAX_MESSAGE_SIZE)
    {
        this->m_isInitialized = false;
        this->m_errorValue = IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED;
        return;
    }

    setPipeName(name);
    switch (channelSide)
    {
    case IpcChannelSide::CLIENT:
    {
        break;
    }
    case IpcChannelSide::SERVER:
    {
        startServerThread();
        break;
    }
    }
    m_isInitialized = true;
}

void UnixDomainSocket::startServerThread() noexcept
{
    if (m_channelSide != IpcChannelSide::SERVER)
    {
        return;
    }

    m_serverThread = std::thread([this] {
        std::vector<NamedPipe> pipes(m_numberOfPipes);

        while (m_keepRunning.load())
        {
            for (uint64_t i = 0; i < m_numberOfPipes; ++i)
            {
                if (!pipes[i])
                {
                    pipes[i] = NamedPipe(m_pipeName, m_maxMessageSize, m_numberOfPipes);
                }

                std::string message;
                message.resize(m_maxMessageSize);
                DWORD bytesRead;
                LPOVERLAPPED noOverlapping = NULL;
                bool fSuccess = ReadFile(pipes[i].m_handle, message.data(), message.size(), &bytesRead, noOverlapping);

                message.resize(bytesRead);
                if (fSuccess)
                {
                    std::lock_guard<std::mutex> lock(m_receivedMessagesMutex);
                    m_receivedMessages.push(Message_t(cxx::TruncateToCapacity, message.c_str()));

                    std::this_thread::sleep_for(std::chrono::milliseconds(m_loopTimeout.toMilliseconds()));
                    pipes[i] = NamedPipe(m_pipeName, m_maxMessageSize, m_numberOfPipes);
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(m_loopTimeout.toMilliseconds()));
        }

        pipes.clear();
    });
}

UnixDomainSocket& UnixDomainSocket::UnixDomainSocket::operator=(UnixDomainSocket&& other) noexcept
{
    if (this != &other)
    {
        CreationPattern_t::operator=(std::move(other));

        destroy();
        m_pipeName = std::move(other.m_pipeName);
        m_channelSide = std::move(other.m_channelSide);
        m_keepRunning.store(other.m_keepRunning.load());
        m_receivedMessages = std::move(other.m_receivedMessages);
        m_maxMessageSize = std::move(other.m_maxMessageSize);

        other.destroy();
        startServerThread();
    }

    return *this;
}

UnixDomainSocket::~UnixDomainSocket() noexcept
{
    destroy();
}

void UnixDomainSocket::setPipeName(const UdsName_t& name) noexcept
{
    m_pipeName = "\\\\.\\pipe\\";
    m_pipeName.append(cxx::TruncateToCapacity, name);
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const UdsName_t& name) noexcept
{
    if (isNameValid(name))
    {
        return cxx::success<bool>(true);
    }
    return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const NoPathPrefix_t,
                                                                      const UdsName_t& name) noexcept
{
    if (isNameValid(name))
    {
        return cxx::success<bool>(true);
    }
    return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
}

cxx::expected<IpcChannelError> UnixDomainSocket::destroy() noexcept
{
    if (m_serverThread.joinable())
    {
        m_keepRunning = false;
        m_serverThread.join();
    }
    return cxx::success<>();
}

cxx::expected<IpcChannelError> UnixDomainSocket::send(const std::string& msg) const noexcept
{
    return timedSend(msg, units::Duration::fromMilliseconds(0));
}

cxx::expected<IpcChannelError> UnixDomainSocket::timedSend(const std::string& msg,
                                                           const units::Duration& timeout) const noexcept
{
    if (IpcChannelSide::SERVER == m_channelSide)
    {
        std::cerr << "sending on server side not supported for unix domain socket \"" << m_pipeName << "\""
                  << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    if (msg.size() > m_maxMessageSize)
    {
        return cxx::error<IpcChannelError>(IpcChannelError::MESSAGE_TOO_LONG);
    }

    // open pipe
    DWORD disableSharing = 0;
    LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    DWORD defaultAttributes = 0;
    HANDLE noTemplateFile = NULL;
    HANDLE namedPipe = CreateFileA(m_pipeName.c_str(),
                                   GENERIC_READ | GENERIC_WRITE,
                                   disableSharing,
                                   noSecurityAttributes,
                                   OPEN_EXISTING,
                                   defaultAttributes,
                                   noTemplateFile);


    if (namedPipe == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            __PrintLastErrorToConsole("", "", 0);
            printf("Could not open pipe for reading. GLE=%d\n", GetLastError());
            return cxx::error<IpcChannelError>(IpcChannelError::I_O_ERROR);
        }

        if (timeout.toMilliseconds() != 0)
        {
            if (!WaitNamedPipeA(m_pipeName.c_str(), timeout.toMilliseconds()))
            {
                return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
            }
        }
    }

    // set to message read mode
    DWORD pipeMode = PIPE_READMODE_MESSAGE;
    bool fSuccess = SetNamedPipeHandleState(namedPipe, &pipeMode, NULL, NULL);
    if (!fSuccess)
    {
        printf("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
        return cxx::error<IpcChannelError>(IpcChannelError::ACCESS_DENIED);
    }

    // send message
    DWORD numberOfSentBytes = 0;
    fSuccess = WriteFile(namedPipe, msg.data(), msg.size(), &numberOfSentBytes, NULL);

    if (!fSuccess)
    {
        printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
        return cxx::error<IpcChannelError>(IpcChannelError::ACCESS_DENIED);
    }

    CloseHandle(namedPipe);
    return cxx::success<>();
}

cxx::expected<std::string, IpcChannelError> UnixDomainSocket::receive() const noexcept
{
    return timedReceive(units::Duration::fromMilliseconds(0));
}

cxx::expected<std::string, IpcChannelError>
UnixDomainSocket::timedReceive(const units::Duration& timeout) const noexcept
{
    if (IpcChannelSide::CLIENT == m_channelSide)
    {
        std::cerr << "receiving on client side not supported for unix domain socket \"" << m_pipeName << "\""
                  << std::endl;
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }

    units::Duration remainingTime = timeout;
    int64_t minimumRetries = 10;
    do
    {
        {
            std::lock_guard<std::mutex> lock(m_receivedMessagesMutex);
            if (!m_receivedMessages.empty())
            {
                std::string msg = m_receivedMessages.front();
                m_receivedMessages.pop();
                return cxx::success<std::string>(msg);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_loopTimeout.toMilliseconds()));
        remainingTime = remainingTime - m_loopTimeout;
        --minimumRetries;
    } while (remainingTime.toMilliseconds() > 0 || minimumRetries > 0);

    return cxx::error<IpcChannelError>(IpcChannelError::TIMEOUT);
}

#else
UnixDomainSocket::UnixDomainSocket(const IpcChannelName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t maxMsgNumber) noexcept
    : UnixDomainSocket(
        NoPathPrefix,
        [&]() -> UdsName_t {
            /// invalid names will be forwarded and handled by the other constructor
            /// separately
            if (!isNameValid(name))
            {
                return name;
            }
            return UdsName_t(PATH_PREFIX).append(iox::cxx::TruncateToCapacity, name);
        }(),
        channelSide,
        maxMsgSize,
        maxMsgNumber)
{
}

UnixDomainSocket::UnixDomainSocket(const NoPathPrefix_t,
                                   const UdsName_t& name,
                                   const IpcChannelSide channelSide,
                                   const size_t maxMsgSize,
                                   const uint64_t maxMsgNumber IOX_MAYBE_UNUSED) noexcept
    : m_name(name)
    , m_channelSide(channelSide)
{
    if (!isNameValid(name))
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

        m_name = std::move(other.m_name);
        m_channelSide = std::move(other.m_channelSide);
        m_sockfd = std::move(other.m_sockfd);
        m_sockAddr = std::move(other.m_sockAddr);
        m_maxMessageSize = std::move(other.m_maxMessageSize);

        other.m_sockfd = INVALID_FD;
    }

    return *this;
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const UdsName_t& name) noexcept
{
    if (!isNameValid(name))
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    if (UdsName_t().capacity() < name.size() + UdsName_t(PATH_PREFIX).size())
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    return unlinkIfExists(NoPathPrefix, UdsName_t(PATH_PREFIX).append(iox::cxx::TruncateToCapacity, name));
}

cxx::expected<bool, IpcChannelError> UnixDomainSocket::unlinkIfExists(const NoPathPrefix_t,
                                                                      const UdsName_t& name) noexcept
{
    if (!isNameValid(name))
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INVALID_CHANNEL_NAME);
    }

    auto unlinkCall = posixCall(unlink)(name.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate();

    if (!unlinkCall.has_error())
    {
        // ENOENT is set if this socket is not known
        return cxx::success<bool>(unlinkCall->errnum != ENOENT);
    }
    else
    {
        return cxx::error<IpcChannelError>(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
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
                unlink(m_sockAddr.sun_path);
            }

            m_sockfd = INVALID_FD;
            m_isInitialized = false;

            return cxx::success<void>();
        }
        else
        {
            return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(closeCall.get_error().errnum));
        }
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

    struct timeval tv = timeout;
#if defined(__APPLE__)
    if (tv.tv_sec != 0 || tv.tv_usec != 0)
    {
        std::cerr
            << "socket: \"" << m_name
            << "\", timedSend with a timeout != 0 is not supported on MacOS. timedSend will behave like send instead."
            << std::endl;
    }
#endif

    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
    }
    else
    {
        auto sendCall = posixCall(iox_sendto)(m_sockfd, msg.c_str(), msg.size() + NULL_TERMINATOR_SIZE, 0, nullptr, 0)
                            .failureReturnValue(ERROR_CODE)
                            .evaluate();

        if (sendCall.has_error())
        {
            return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(sendCall.get_error().errnum));
        }
        else
        {
            return cxx::success<void>();
        }
    }
}

cxx::expected<std::string, IpcChannelError> UnixDomainSocket::receive() const noexcept
{
    // we also support timedReceive. The setsockopt call sets the timeout for all further recvfrom calls, so we must set
    // it to 0 to turn the timeout off
    struct timeval tv;
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

    struct timeval tv = timeout;
#if defined(__APPLE__)
    if (tv.tv_sec != 0 || tv.tv_usec != 0)
    {
        std::cerr
            << "socket: \"" << m_name
            << "\", timedReceive with a timeout != 0 is not supported on MacOS. timedReceive will behave like receive "
               "instead."
            << std::endl;
    }
#endif

    auto setsockoptCall = posixCall(iox_setsockopt)(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
                              .failureReturnValue(ERROR_CODE)
                              .ignoreErrnos(EWOULDBLOCK)
                              .evaluate();

    if (setsockoptCall.has_error())
    {
        return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(setsockoptCall.get_error().errnum));
    }
    else
    {
        char message[MAX_MESSAGE_SIZE + 1];

        auto recvCall = posixCall(iox_recvfrom)(m_sockfd, message, MAX_MESSAGE_SIZE, 0, nullptr, nullptr)
                            .failureReturnValue(ERROR_CODE)
                            .suppressErrorMessagesForErrnos(EAGAIN)
                            .evaluate();
        message[MAX_MESSAGE_SIZE] = 0;

        if (recvCall.has_error())
        {
            return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(recvCall.get_error().errnum));
        }
        return cxx::success<std::string>(std::string(message));
    }
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
    strncpy(m_sockAddr.sun_path, m_name.c_str(), m_name.size());

    // the mask will be applied to the permissions, we only allow users and group members to have read and write access
    // the system call always succeeds, no need to check for errors
    mode_t umaskSaved = umask(S_IXUSR | S_IXGRP | S_IRWXO);
    // Reset to old umask when going out of scope
    cxx::GenericRAII umaskGuard([&] { umask(umaskSaved); });

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
        unlink(m_sockAddr.sun_path);

        auto bindCall =
            posixCall(iox_bind)(m_sockfd, reinterpret_cast<struct sockaddr*>(&m_sockAddr), sizeof(m_sockAddr))
                .failureReturnValue(ERROR_CODE)
                .evaluate();

        if (!bindCall.has_error())
        {
            return cxx::success<>();
        }
        else
        {
            closeFileDescriptor().or_else([](auto) {
                std::cerr << "Unable to close socket file descriptor in error related cleanup during initialization."
                          << std::endl;
            });
            // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
            return cxx::error<IpcChannelError>(convertErrnoToIpcChannelError(bindCall.get_error().errnum));
        }
    }
    else
    {
        // we use a connected socket, this leads to a behavior closer to the message queue (e.g. error if client
        // is created and server not present)
        auto connectCall =
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
        else
        {
            return cxx::success<>();
        }
    }
}

IpcChannelError UnixDomainSocket::convertErrnoToIpcChannelError(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to create unix domain socket denied \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::ACCESS_DENIED);
    }
    case EAFNOSUPPORT:
    {
        std::cerr << "address family not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::PROCESS_LIMIT);
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::SYSTEM_LIMIT);
    }
    case ENOBUFS:
    {
        std::cerr << "out of memory for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::OUT_OF_MEMORY);
    }
    case ENOMEM:
    {
        std::cerr << "out of memory for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::OUT_OF_MEMORY);
    }
    case EPROTONOSUPPORT:
    {
        std::cerr << "protocol type not supported for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_ARGUMENTS);
    }
    case EADDRINUSE:
    {
        std::cerr << "unix domain socket already in use \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::CHANNEL_ALREADY_EXISTS);
    }
    case EBADF:
    {
        std::cerr << "invalid file descriptor for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_FILE_DESCRIPTOR);
    }
    case ENOTSOCK:
    {
        std::cerr << "invalid file descriptor for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_FILE_DESCRIPTOR);
    }
    case EADDRNOTAVAIL:
    {
        std::cerr << "interface or address error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENAMETOOLONG:
    {
        std::cerr << "name too long for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENOTDIR:
    {
        std::cerr << "not a directory error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case ENOENT:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError(IpcChannelError::NO_SUCH_CHANNEL);
    }
    case EROFS:
    {
        std::cerr << "read only error for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_CHANNEL_NAME);
    }
    case EIO:
    {
        std::cerr << "I/O for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::I_O_ERROR);
    }
    case ENOPROTOOPT:
    {
        std::cerr << "invalid option for unix domain socket \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::INVALID_ARGUMENTS);
    }
    case ECONNREFUSED:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError(IpcChannelError::NO_SUCH_CHANNEL);
    }
    case ECONNRESET:
    {
        std::cerr << "connection was reset by peer for \"" << m_name << "\"" << std::endl;
        return IpcChannelError(IpcChannelError::CONNECTION_RESET_BY_PEER);
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return IpcChannelError(IpcChannelError::TIMEOUT);
    }
    default:
    {
        std::cerr << "internal logic error in unix domain socket \"" << m_name << "\" occurred" << std::endl;
        return IpcChannelError(IpcChannelError::INTERNAL_LOGIC_ERROR);
    }
    }
}

#endif

} // namespace posix
} // namespace iox
