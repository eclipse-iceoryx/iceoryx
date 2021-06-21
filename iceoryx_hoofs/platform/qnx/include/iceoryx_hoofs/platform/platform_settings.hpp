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
#ifndef IOX_HOOFS_QNX_PLATFORM_PLATFORM_SETTINGS_HPP
#define IOX_HOOFS_QNX_PLATFORM_PLATFORM_SETTINGS_HPP

#include <cstdint>

namespace iox
{
namespace posix
{
class UnixDomainSocket;
}

namespace platform
{
constexpr uint64_t IOX_MAX_FILENAME_LENGTH = 255U;
constexpr uint64_t IOX_MAX_PATH_LENGTH = 1023U;
constexpr bool IOX_SHM_WRITE_ZEROS_ON_CREATION = true;
constexpr uint64_t IOX_MAX_SHM_NAME_LENGTH = 1024U;
constexpr const char IOX_PATH_SEPARATORS[] = "/";
constexpr uint64_t IOX_UDS_SOCKET_MAX_MESSAGE_SIZE = 4096;
constexpr const char IOX_UDS_SOCKET_PATH_PREFIX[] = "/tmp/";
constexpr const char IOX_LOCK_FILE_PATH_PREFIX[] = "/var/lock/";
using IoxIpcChannelType = iox::posix::UnixDomainSocket;
} // namespace platform
} // namespace iox

#endif // IOX_HOOFS_QNX_PLATFORM_PLATFORM_SETTINGS_HPP
