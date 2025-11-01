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

#include "iceoryx_platform/socket.hpp"
#include <unistd.h>

// NOLINTNEXTLINE(readability-identifier-naming)
int iox_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

// NOLINTNEXTLINE(readability-identifier-naming)
int iox_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-size)
int iox_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    return setsockopt(sockfd, level, optname, optval, optlen);
}

ssize_t
// NOLINTNEXTLINE(readability-identifier-naming,readability-function-size)
iox_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
    return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-size)
ssize_t iox_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

// NOLINTNEXTLINE(readability-identifier-naming)
int iox_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect(sockfd, addr, addrlen);
}

// NOLINTNEXTLINE(readability-identifier-naming)
int iox_closesocket(int sockfd)
{
    return close(sockfd);
}
