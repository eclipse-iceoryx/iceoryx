// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_utils/platform/platform-correction.hpp"
#include "iceoryx_utils/platform/types.hpp"

#define AF_INET 0
#define SOCK_STREAM 1
#define SOL_SOCKET 2
#define SO_SNDTIMEO 3
#define SO_RCVTIMEO 4
#define AF_LOCAL 5
#define SOCK_DGRAM 6

using in_port_t = int;
using sa_family_t = int;
using socklen_t = int;
using in_addr_t = uint32_t;

struct in_addr
{
    uint32_t s_addr;
};

struct sockaddr_in
{
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};

struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[14];
};

inline in_addr_t inet_addr(const char* cp)
{
    return {0};
}

inline uint16_t htons(uint16_t hostshort)
{
    return 0;
}

inline int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return 0;
}

inline int socket(int domain, int type, int protocol)
{
    return 0;
}

inline int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    return 0;
}

inline ssize_t
sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    return 0;
}

inline ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    return 0;
}

inline int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return 0;
}
