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
#ifndef IOX_HOOFS_WIN_PLATFORM_PLATFORM_SETTINGS_HPP
#define IOX_HOOFS_WIN_PLATFORM_PLATFORM_SETTINGS_HPP

#include <cstdint>

namespace iox
{
namespace posix
{
class NamedPipe;
}

namespace platform
{
constexpr bool IOX_SHM_WRITE_ZEROS_ON_CREATION = false;
constexpr uint64_t IOX_MAX_SHM_NAME_LENGTH = 255U;
// yes, windows has two possible path separators!
constexpr const char IOX_PATH_SEPARATORS[] = "\\/";
// unix domain sockets are not supported in windows but the variables have to be defined
// so that the code with stub implementation at least compiles
constexpr uint64_t IOX_UDS_SOCKET_MAX_MESSAGE_SIZE = 1024U;
constexpr char IOX_UDS_SOCKET_PATH_PREFIX[] = "";
constexpr const char IOX_LOCK_FILE_PATH_PREFIX[] = "C:\\Windows\\Temp\\";
using IoxIpcChannelType = iox::posix::NamedPipe;
constexpr uint64_t IOX_MAX_FILENAME_LENGTH = 255U;
constexpr uint64_t IOX_MAX_PATH_LENGTH = 255U;

namespace win32
{
// just increase this number to increase the maximum shared memory size supported
// by windows
constexpr uint64_t IOX_MAXIMUM_SUPPORTED_SHM_SIZE = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
} // namespace win32
} // namespace platform
} // namespace iox
#endif // IOX_HOOFS_WIN_PLATFORM_PLATFORM_SETTINGS_HPP
