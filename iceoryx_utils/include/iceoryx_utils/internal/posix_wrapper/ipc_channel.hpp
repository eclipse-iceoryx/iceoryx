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
#ifndef IOX_UTILS_POSIX_WRAPPER_IPC_CHANNEL_HPP
#define IOX_UTILS_POSIX_WRAPPER_IPC_CHANNEL_HPP

#include "iceoryx_utils/cxx/string.hpp"


namespace iox
{
#if defined(__APPLE__)
/// @note on macOS the process name length needs to be decreased since the process name is used for the unix domain
/// socket path which has a capacity for only 103 characters. The full path consists of UnixDomainSocket::PATH_PREFIX,
/// which is currently 5 characters and the specified process name
constexpr uint32_t MAX_IPC_CHANNEL_NAME_LENGTH = 98U;
#else
constexpr uint32_t MAX_IPC_CHANNEL_NAME_LENGTH = 100U;
#endif

using IpcChannelName_t = cxx::string<MAX_IPC_CHANNEL_NAME_LENGTH>;
namespace posix
{
enum class IpcChannelError : uint8_t
{
    NOT_INITIALIZED,
    ACCESS_DENIED,
    NO_SUCH_CHANNEL,
    INTERNAL_LOGIC_ERROR,
    CHANNEL_ALREADY_EXISTS,
    INVALID_ARGUMENTS,
    MAX_MESSAGE_SIZE_EXCEEDED,
    MESSAGE_TOO_LONG,
    CHANNEL_FULL,
    INVALID_CHANNEL_NAME,
    TIMEOUT,
    PROCESS_LIMIT,
    SYSTEM_LIMIT,
    OUT_OF_MEMORY,
    INVALID_FILE_DESCRIPTOR,
    I_O_ERROR,
    CONNECTION_RESET_BY_PEER,
    UNDEFINED
};

enum class IpcChannelMode : uint8_t
{
    NON_BLOCKING,
    BLOCKING
};

enum class IpcChannelSide : uint8_t
{
    CLIENT,
    SERVER
};


} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_IPC_CHANNEL_HPP
