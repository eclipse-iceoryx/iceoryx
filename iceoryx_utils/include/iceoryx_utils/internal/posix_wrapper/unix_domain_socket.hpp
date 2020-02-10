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

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/mqueue.hpp"
#include "iceoryx_utils/platform/stat.hpp"
#include "iceoryx_utils/platform/un.hpp"

namespace iox
{
namespace posix
{
/// @brief Wrapper class for unix domain socket
class UnixDomainSocket : public DesignPattern::Creation<UnixDomainSocket, IpcChannelError>
{
  public:
    /// @ todo in ipc_channel.hpp? The same for all channels?
    static constexpr size_t MAX_MESSAGE_SIZE = 4096;
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr int32_t INVALID_FD = -1;

    /// for calling private constructor in create method
    friend class DesignPattern::Creation<UnixDomainSocket, IpcChannelError>;

    /// default constructor. The result is an invalid UnixDomainSocket object which can be reassigned later by using the
    /// move constructor.
    UnixDomainSocket() noexcept;

    UnixDomainSocket(const UnixDomainSocket& other) = delete;
    UnixDomainSocket(UnixDomainSocket&& other) noexcept;
    UnixDomainSocket& operator=(const UnixDomainSocket& other) = delete;
    UnixDomainSocket& operator=(UnixDomainSocket&& other) noexcept;

    ~UnixDomainSocket() noexcept;

    static cxx::expected<bool, IpcChannelError> unlinkIfExists(const std::string& name) noexcept;

    /// close the unix domain socket.
    cxx::expected<IpcChannelError> destroy() noexcept;

    /// @brief send a message using std::string.
    /// @return true if sent without errors, false otherwise
    cxx::expected<IpcChannelError> send(const std::string& msg) noexcept;

    /// @brief try to send a message for a given timeout duration using std::string
    cxx::expected<IpcChannelError> timedSend(const std::string& msg, const units::Duration& timeout) noexcept;

    /// @brief receive message using std::string.
    /// @return number of characters received. In case of an error, returns -1 and msg is empty.
    cxx::expected<std::string, IpcChannelError> receive() noexcept;

    /// @brief try to receive message for a given timeout duration using std::string. Only defined
    /// for NON_BLOCKING == false.
    /// @return optional containing the received string. In case of an error, nullopt type is returned.
    cxx::expected<std::string, IpcChannelError> timedReceive(const units::Duration& timeout) noexcept;


    cxx::expected<bool, IpcChannelError> isOutdated() noexcept;

  private:
    UnixDomainSocket(const std::string& name,
                     const IpcChannelMode mode,
                     const IpcChannelSide channelSide,
                     const size_t maxMsgSize = 512u,
                     const uint64_t maxMsgNumber = 10u) noexcept;

    cxx::expected<int, IpcChannelError> createSocket(const IpcChannelMode mode) noexcept;

    cxx::error<IpcChannelError> createErrorFromErrnum(const int errnum) noexcept;

  private:
    std::string m_name;
    IpcChannelSide m_channelSide;
    int m_sockfd{INVALID_FD};
    struct sockaddr_un m_sockAddr;
};
} // namespace posix
} // namespace iox
