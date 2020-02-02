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

#define AF_INET 0
#define SOCK_STREAM 1

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
