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

#include "iceoryx_hoofs/platform/socket.hpp"
#include "iceoryx_hoofs/platform/named_pipe.hpp"
#include "iceoryx_hoofs/platform/un.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"

#include <iostream>
#include <map>
#include <memory>

struct Winsock2ApiInitializer
{
    Winsock2ApiInitializer()
    {
        WORD requestedVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        auto result = Win32Call(WSAStartup, requestedVersion, &wsaData).value;
        if (result != 0)
        {
            std::cerr << "unable to initialize winsock2" << std::endl;
            std::terminate();
        }

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
            std::cerr << "required winsock2.dll version is 2.2, found " << HIBYTE(wsaData.wVersion) << "."
                      << LOBYTE(wsaData.wVersion) << std::endl;
            cleanupWinsock();
            std::terminate();
        }
    }

    ~Winsock2ApiInitializer()
    {
        cleanupWinsock();
    }

    void cleanupWinsock()
    {
        Win32Call(WSACleanup);
    }
};

static Winsock2ApiInitializer winsock2ApiInitializer;

struct socketHandle_t
{
    std::unique_ptr<NamedPipeReceiver> receiver;
    std::string senderPipeName;
    uint64_t receiveTimeoutInMs = 0U;
};

static std::map<int, socketHandle_t> unixDomainSockets;
static int i = 0;

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        constexpr uint64_t MAX_NUMBER_OF_MESSAGES = 10U;
        const struct sockaddr_un* addrInfo = reinterpret_cast<const struct sockaddr_un*>(addr);
        iter->second.receiver = std::make_unique<NamedPipeReceiver>(
            addrInfo->sun_path, IOX_SOCKET_MAX_MESSAGE_SIZE, MAX_NUMBER_OF_MESSAGES);
    }
    return 0;
}

int iox_socket(int domain, int type, int protocol)
{
    // only handle unix domain sockets differently
    if (domain == AF_LOCAL && type == SOCK_DGRAM && protocol == 0)
    {
        ++i;
        unixDomainSockets[i];
        return i;
    }
    return 0;
}

int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        if (level == SOL_SOCKET && optname == SO_RCVTIMEO)
        {
            constexpr uint64_t MILLISECONDS_PER_SECOND = 1000U;
            constexpr uint64_t MICROSECONDS_PER_MILLISECOND = 1000U;
            const struct timeval* tv = reinterpret_cast<const struct timeval*>(optval);
            iter->second.receiveTimeoutInMs =
                tv->tv_sec * MILLISECONDS_PER_SECOND + tv->tv_usec / MICROSECONDS_PER_MILLISECOND;
        }
    }
    return 0;
}

ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        std::string message;
        message.resize(len);
        memcpy(message.data(), buf, len);
        NamedPipeSender(iter->second.senderPipeName, 0U).send(message);
        return 0;
    }

    return 0;
}

ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    constexpr uint64_t NULL_TERMINATOR = 1U;
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        auto message = iter->second.receiver->timedReceive(iter->second.receiveTimeoutInMs);
        if (!message)
        {
            errno = EWOULDBLOCK;
            return -1;
        }

        memcpy(buf, message->data(), std::min(len, message->size() + NULL_TERMINATOR));
        return std::min(message->size(), len);
    }

    return 0;
}

int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        const struct sockaddr_un* addrInfo = reinterpret_cast<const struct sockaddr_un*>(addr);
        iter->second.senderPipeName = addrInfo->sun_path;
    }
    return 0;
}

int iox_closesocket(int sockfd)
{
    auto iter = unixDomainSockets.find(sockfd);
    if (iter != unixDomainSockets.end())
    {
        unixDomainSockets.erase(iter);
        return 0;
    }

    return Win32Call(closesocket, static_cast<SOCKET>(sockfd)).value;
}
