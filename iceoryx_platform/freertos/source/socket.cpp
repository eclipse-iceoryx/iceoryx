// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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
#include "FreeRTOS.h"

int iox_bind(int, const struct sockaddr*, socklen_t)
{
    configASSERT(false);
    return -1; // bind(sockfd, addr, addrlen);
}

int iox_socket(int, int, int)
{
    configASSERT(false);
    return -1; // socket(domain, type, protocol);
}

int iox_setsockopt(int, int, int, const void*, socklen_t)
{
    configASSERT(false);
    return -1; // setsockopt(sockfd, level, optname, optval, optlen);
}

ssize_t iox_sendto(int, const void*, size_t, int, const struct sockaddr*, socklen_t)
{
    configASSERT(false);
    return -1; // sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t iox_recvfrom(int, void*, size_t, int, struct sockaddr*, socklen_t*)
{
    configASSERT(false);
    return -1; // recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

int iox_connect(int, const struct sockaddr*, socklen_t)
{
    configASSERT(false);
    return -1; // connect(sockfd, addr, addrlen);
}

int iox_closesocket(int)
{
    configASSERT(false);
    return -1; // close(sockfd);
}
