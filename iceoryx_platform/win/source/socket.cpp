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
#include "iceoryx_platform/logging.hpp"
#include "iceoryx_platform/types.hpp"
#include <cstdio>

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_bind' is not implemented in windows!");
    return 0;
}

int iox_socket(int domain, int type, int protocol)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_socket' is not implemented in windows!");
    return 0;
}

int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_setsockopt' is not implemented in windows!");
    return 0;
}

iox_ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_sendto' is not implemented in windows!");
    return 0;
}

iox_ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_recvfrom' is not implemented in windows!");
    return 0;
}

int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_connect' is not implemented in windows!");
    return 0;
}

int iox_closesocket(int sockfd)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_closesocket' is not implemented in windows!");
    return 0;
}
