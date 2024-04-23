// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_IPC_UNIX_DOMAIN_SOCKET_HPP
#define IOX_HOOFS_POSIX_IPC_UNIX_DOMAIN_SOCKET_HPP

#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/socket.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/un.hpp"
#include "iox/builder.hpp"
#include "iox/duration.hpp"
#include "iox/filesystem.hpp"
#include "iox/not_null.hpp"
#include "iox/optional.hpp"
#include "iox/posix_ipc_channel.hpp"

namespace iox
{
class UnixDomainSocketBuilder;

/// @brief Wrapper class for unix domain socket
class UnixDomainSocket
{
  public:
    struct NoPathPrefix_t
    {
    };
    static constexpr NoPathPrefix_t NoPathPrefix{};
    static constexpr uint64_t NULL_TERMINATOR_SIZE = 1U;
    static constexpr uint64_t MAX_MESSAGE_SIZE = platform::IOX_UDS_SOCKET_MAX_MESSAGE_SIZE - NULL_TERMINATOR_SIZE;
    static constexpr uint64_t MAX_NUMBER_OF_MESSAGES = 10;
    /// @brief The name length is limited by the size of the sockaddr_un::sun_path buffer and the IOX_SOCKET_PATH_PREFIX
    static constexpr size_t LONGEST_VALID_NAME = sizeof(sockaddr_un::sun_path) - 1;

    using Builder_t = UnixDomainSocketBuilder;

    using UdsName_t = string<LONGEST_VALID_NAME>;
    using Message_t = string<MAX_MESSAGE_SIZE>;

    UnixDomainSocket() noexcept = delete;
    UnixDomainSocket(const UnixDomainSocket& other) = delete;
    UnixDomainSocket(UnixDomainSocket&& other) noexcept;
    UnixDomainSocket& operator=(const UnixDomainSocket& other) = delete;
    UnixDomainSocket& operator=(UnixDomainSocket&& other) noexcept;

    ~UnixDomainSocket() noexcept;

    /// @brief unlink the provided unix domain socket
    /// @param name of the unix domain socket to unlink
    /// @return true if the unix domain socket could be unlinked, false otherwise, PosixIpcChannelError if error occured
    static expected<bool, PosixIpcChannelError> unlinkIfExists(const UdsName_t& name) noexcept;

    /// @brief unlink the provided unix domain socket
    /// @param NoPathPrefix signalling that this method does not add a path prefix
    /// @param name of the unix domain socket to unlink
    /// @return true if the unix domain socket could be unlinked, false otherwise, PosixIpcChannelError if error occured
    static expected<bool, PosixIpcChannelError> unlinkIfExists(const NoPathPrefix_t, const UdsName_t& name) noexcept;

    /// @brief send a message using std::string.
    /// @param msg to send
    /// @return PosixIpcChannelError if error occured
    expected<void, PosixIpcChannelError> send(const std::string& msg) const noexcept;

    /// @brief try to send a message for a given timeout duration using std::string
    /// @param msg to send
    /// @param timout for the send operation
    /// @return PosixIpcChannelError if error occured
    expected<void, PosixIpcChannelError> timedSend(const std::string& msg,
                                                   const units::Duration& timeout) const noexcept;

    /// @brief receive message using std::string.
    /// @return received message. In case of an error, PosixIpcChannelError is returned and msg is empty.
    expected<std::string, PosixIpcChannelError> receive() const noexcept;

    /// @brief try to receive message for a given timeout duration using std::string.
    /// @param timout for the receive operation
    /// @return received message. In case of an error, PosixIpcChannelError is returned and msg is empty.
    expected<std::string, PosixIpcChannelError> timedReceive(const units::Duration& timeout) const noexcept;

    /// @brief send a message using iox::string
    /// @tparam N capacity of the iox::string
    /// @param[in] buf data to send
    /// @return PosixIpcChannelError if error occured
    template <uint64_t N>
    expected<void, PosixIpcChannelError> send(const iox::string<N>& buf) const noexcept;

    /// @brief try to send a message for a given timeout duration using iox::string
    /// @tparam N capacity of the iox::string
    /// @param[in] buf data to send
    /// @param[in] timeout for the send operation
    /// @return PosixIpcChannelError if error occured
    template <uint64_t N>
    expected<void, PosixIpcChannelError> timedSend(const iox::string<N>& buf,
                                                   const units::Duration& timeout) const noexcept;

    /// @brief receive message using iox::string
    /// @tparam N capacity of the iox::string
    /// @param[in] buf data to receive
    /// @return  PosixIpcChannelError if error occured
    template <uint64_t N>
    expected<void, PosixIpcChannelError> receive(iox::string<N>& buf) const noexcept;

    /// @brief try to receive message for a given timeout duration using iox::string
    /// @tparam N capacity of the iox::string
    /// @param[in] buf data to receive]
    /// @param[in] timeout for the send operation
    /// @return  PosixIpcChannelError if error occured
    template <uint64_t N>
    expected<void, PosixIpcChannelError> timedReceive(iox::string<N>& buf,
                                                      const units::Duration& timeout) const noexcept;

  private:
    friend class UnixDomainSocketBuilderNoPathPrefix;

    UnixDomainSocket(const UdsName_t& name,
                     const PosixIpcChannelSide channelSide,
                     const int32_t sockfd,
                     const sockaddr_un sockAddr,
                     const uint64_t maxMsgSize) noexcept;

    expected<void, PosixIpcChannelError> destroy() noexcept;

    PosixIpcChannelError errnoToEnum(const int32_t errnum) const noexcept;
    static PosixIpcChannelError errnoToEnum(const UdsName_t& name, const int32_t errnum) noexcept;

    expected<void, PosixIpcChannelError> closeFileDescriptor() noexcept;
    static expected<void, PosixIpcChannelError> closeFileDescriptor(const UdsName_t& name,
                                                                    const int sockfd,
                                                                    const sockaddr_un& sockAddr,
                                                                    PosixIpcChannelSide channelSide) noexcept;

    enum class Termination : uint8_t
    {
        NONE,
        NULL_TERMINATOR
    };

    template <typename Type, Termination Terminator>
    expected<void, PosixIpcChannelError>
    timedSendImpl(not_null<const Type*> msg, uint64_t msgSize, const units::Duration& timeout) const noexcept;
    template <typename Type, Termination Terminator>
    expected<uint64_t, PosixIpcChannelError>
    timedReceiveImpl(not_null<Type*> msg, uint64_t maxMsgSize, const units::Duration& timeout) const noexcept;

  private:
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr int32_t INVALID_FD = -1;

    UdsName_t m_name;
    PosixIpcChannelSide m_channelSide = PosixIpcChannelSide::CLIENT;
    int32_t m_sockfd{INVALID_FD};
    sockaddr_un m_sockAddr{};
    uint64_t m_maxMessageSize{MAX_MESSAGE_SIZE};
};

class UnixDomainSocketBuilder
{
    /// @brief Defines the socket name
    IOX_BUILDER_PARAMETER(PosixIpcChannelName_t, name, "")

    /// @brief Defines how the socket is opened, i.e. as client or server
    IOX_BUILDER_PARAMETER(PosixIpcChannelSide, channelSide, PosixIpcChannelSide::CLIENT)

    /// @brief Defines the max message size of the socket
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgSize, UnixDomainSocket::MAX_MESSAGE_SIZE)

    /// @brief Defines the max number of messages for the socket.
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgNumber, UnixDomainSocket::MAX_NUMBER_OF_MESSAGES)

  public:
    /// @brief create a unix domain socket
    /// @return On success a 'UnixDomainSocket' is returned and on failure an 'PosixIpcChannelError'.
    expected<UnixDomainSocket, PosixIpcChannelError> create() const noexcept;
};

class UnixDomainSocketBuilderNoPathPrefix
{
    /// @brief Defines the socket name
    IOX_BUILDER_PARAMETER(UnixDomainSocket::UdsName_t, name, "")

    /// @brief Defines how the socket is opened, i.e. as client or server
    IOX_BUILDER_PARAMETER(PosixIpcChannelSide, channelSide, PosixIpcChannelSide::CLIENT)

    /// @brief Defines the max message size of the socket
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgSize, UnixDomainSocket::MAX_MESSAGE_SIZE)

    /// @brief Defines the max number of messages for the socket.
    IOX_BUILDER_PARAMETER(uint64_t, maxMsgNumber, UnixDomainSocket::MAX_NUMBER_OF_MESSAGES)

  public:
    /// @brief create a unix domain socket
    /// @return On success a 'UnixDomainSocket' is returned and on failure an 'PosixIpcChannelError'.
    expected<UnixDomainSocket, PosixIpcChannelError> create() const noexcept;
};

} // namespace iox

#include "detail/unix_domain_socket.inl"

#endif // IOX_HOOFS_POSIX_IPC_UNIX_DOMAIN_SOCKET_HPP
