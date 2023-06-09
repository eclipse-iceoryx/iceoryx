// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_SOCKET_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_SOCKET_HPP

#include <cstdint>

#include <FreeRTOS_POSIX.h>

/* For setsockopt(2) */
#define SOL_SOCKET 1

#define SO_DEBUG 1
#define SO_REUSEADDR 2
#define SO_TYPE 3
#define SO_ERROR 4
#define SO_DONTROUTE 5
#define SO_BROADCAST 6
#define SO_SNDBUF 7
#define SO_RCVBUF 8
#define SO_SNDBUFFORCE 32
#define SO_RCVBUFFORCE 33
#define SO_KEEPALIVE 9
#define SO_OOBINLINE 10
#define SO_NO_CHECK 11
#define SO_PRIORITY 12
#define SO_LINGER 13
#define SO_BSDCOMPAT 14
#define SO_REUSEPORT 15
#define SO_PASSCRED 16
#define SO_PEERCRED 17
#define SO_RCVLOWAT 18
#define SO_SNDLOWAT 19
#define SO_RCVTIMEO 20
#define SO_SNDTIMEO 21

#define AF_LOCAL 1

/* Types of sockets.  */
enum __socket_type
{
    SOCK_STREAM = 1, /* Sequenced, reliable, connection-based
                        byte streams.  */
#define SOCK_STREAM SOCK_STREAM
    SOCK_DGRAM = 2, /* Connectionless, unreliable datagrams
                       of fixed maximum length.  */
#define SOCK_DGRAM SOCK_DGRAM
    SOCK_RAW = 3, /* Raw protocol interface.  */
#define SOCK_RAW SOCK_RAW
    SOCK_RDM = 4, /* Reliably-delivered messages.  */
#define SOCK_RDM SOCK_RDM
    SOCK_SEQPACKET = 5, /* Sequenced, reliable, connection-based,
                           datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
    SOCK_DCCP = 6, /* Datagram Congestion Control Protocol.  */
#define SOCK_DCCP SOCK_DCCP
    SOCK_PACKET = 10, /* Linux specific way of getting packets
                         at the dev level.  For writing rarp and
                         other similar things on the user level. */
#define SOCK_PACKET SOCK_PACKET

    /* Flags to be ORed into the type parameter of socket and socketpair and
       used for the flags parameter of paccept.  */

    SOCK_CLOEXEC = 02000000, /* Atomically set close-on-exec flag for the
                                new descriptor(s).  */
#define SOCK_CLOEXEC SOCK_CLOEXEC
    SOCK_NONBLOCK = 00004000 /* Atomically mark descriptor(s) as
                                non-blocking.  */
#define SOCK_NONBLOCK SOCK_NONBLOCK
};

using sa_family_t = int;
typedef uint32_t __socklen_t;
typedef __socklen_t socklen_t;
struct sockaddr_un
{
    sa_family_t sun_family; /* AF_UNIX */
    char sun_path[108];     /* Pathname */
};
int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_socket(int domain, int type, int protocol);
int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_closesocket(int sockfd);

#endif // IOX_HOOFS_FREERTOS_PLATFORM_SOCKET_HPP
