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
#include <cstdio>

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

int iox_socket(int domain, int type, int protocol)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}

int iox_closesocket(int sockfd)
{
    fprintf(stderr, "%s is not implemented in windows!\n", __PRETTY_FUNCTION__);
    return 0;
}
