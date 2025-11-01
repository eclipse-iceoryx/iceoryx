// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
#ifndef IOX_HOOFS_MAC_PLATFORM_SOCKET_HPP
#define IOX_HOOFS_MAC_PLATFORM_SOCKET_HPP

#include <cstdint>
#include <sys/socket.h>

#define SOCK_NONBLOCK 0

int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_socket(int domain, int type, int protocol);
int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
ssize_t
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int iox_closesocket(int sockfd);

#endif // IOX_HOOFS_MAC_PLATFORM_SOCKET_HPP
