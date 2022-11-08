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

#include "iceoryx_platform/socket.hpp"
#include <unistd.h>

#include <thread>

static struct timeval getTimeoutOfSocket(int sockfd, int option_name)
{
    struct timeval tv;
    socklen_t optionLength = sizeof(struct timeval);
    if (getsockopt(sockfd, SOL_SOCKET, option_name, &tv, &optionLength) == -1)
    {
        return {0, 0};
    }
    return tv;
}

static void sleepFor(struct timeval& tv)
{
    std::this_thread::sleep_for(std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec));
}

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

int iox_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    return setsockopt(sockfd, level, optname, optval, optlen);
}

ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    auto timeout = getTimeoutOfSocket(sockfd, SO_SNDTIMEO);
    ssize_t sentBytes = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (sentBytes <= 0 && timeout.tv_sec != 0 && timeout.tv_usec != 0)
    {
        sleepFor(timeout);
        return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    return sentBytes;
}

ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    auto timeout = getTimeoutOfSocket(sockfd, SO_RCVTIMEO);
    ssize_t receivedBytes = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    if (receivedBytes <= 0 && timeout.tv_sec != 0 && timeout.tv_usec != 0)
    {
        sleepFor(timeout);
        return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }
    return receivedBytes;
}

int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect(sockfd, addr, addrlen);
}

int iox_closesocket(int sockfd)
{
    return close(sockfd);
}
