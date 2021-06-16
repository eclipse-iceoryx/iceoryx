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
#ifndef IOX_HOOFS_QNX_PLATFORM_SOCKET_HPP
#define IOX_HOOFS_QNX_PLATFORM_SOCKET_HPP

#include <cstdint>
#include <sys/socket.h>

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_socket(int domain, int type, int protocol);
int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_closesocket(int sockfd);

#endif // IOX_HOOFS_QNX_PLATFORM_SOCKET_HPP
