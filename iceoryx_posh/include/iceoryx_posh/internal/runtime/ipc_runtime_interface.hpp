// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_RUNTIME_IPC_RUNTIME_INTERFACE_HPP
#define IOX_POSH_RUNTIME_IPC_RUNTIME_INTERFACE_HPP

#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_user.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace runtime
{
enum class IpcRuntimeInterfaceError
{
    CANNOT_CREATE_APPLICATION_CHANNEL,
    TIMEOUT_WAITING_FOR_ROUDI,
    SENDING_REQUEST_TO_ROUDI_FAILED,
    NO_RESPONSE_FROM_ROUDI,
};

class IpcRuntimeInterface
{
  public:
    /// @brief Creates an 'IpcRuntimeInterface' which tries to register at RouDi and delegates any error up in the stack
    /// @param[in] runtimeName is the name of the runtime to register at RouDi
    /// @param[in] domainId to tie the interface to
    /// @param[in] roudiWaitingTimeout is the time to wait for RouDi to start if it is nor running
    /// @return an IPC interface to communicate with RouDi or a IpcRuntimeInterfaceError
    static expected<IpcRuntimeInterface, IpcRuntimeInterfaceError> create(
        const RuntimeName_t& runtimeName, const DomainId domainId, const units::Duration roudiWaitingTimeout) noexcept;

    ~IpcRuntimeInterface() noexcept = default;

    IpcRuntimeInterface(IpcRuntimeInterface&&) noexcept = default;
    IpcRuntimeInterface& operator=(IpcRuntimeInterface&&) noexcept = default;

    /// @brief Not needed therefore deleted
    IpcRuntimeInterface(const IpcRuntimeInterface&) = delete;
    IpcRuntimeInterface& operator=(const IpcRuntimeInterface&) = delete;

    /// @brief send a request to the RouDi daemon
    /// @param[in] msg request to RouDi
    /// @param[out] answer response from RouDi
    /// @return true if communication was successful, false if not
    bool sendRequestToRouDi(const IpcMessage& msg, IpcMessage& answer) noexcept;

    /// @brief get the adress offset of the segment manager
    /// @return address offset as iox::RelativePointer::offset_t
    UntypedRelativePointer::offset_t getSegmentManagerAddressOffset() const noexcept;

    /// @brief get the size of the management shared memory object
    /// @return size in bytes
    uint64_t getShmTopicSize() noexcept;

    /// @brief get the segment id of the shared memory object
    /// @return segment id
    uint64_t getSegmentId() const noexcept;

    /// @brief Access the relative pointer offset for the heartbeat
    /// @return relative pointer offset for the heartbeat or 'nullopt' if monitoring is disabled
    optional<UntypedRelativePointer::offset_t> getHeartbeatAddressOffset() const noexcept;

  private:
    struct MgmtShmCharacteristics
    {
        uint64_t shmTopicSize{0U};
        uint64_t segmentId{0U};
        UntypedRelativePointer::offset_t segmentManagerAddressOffset{UntypedRelativePointer::NULL_POINTER_OFFSET};
        optional<UntypedRelativePointer::offset_t> heartbeatAddressOffset;
    };

    enum class RegAckResult
    {
        SUCCESS,
        TIMEOUT,
        MALFORMED_RESPONSE
    };

    IpcRuntimeInterface(IpcInterfaceCreator&& appIpcInterface,
                        IpcInterfaceUser&& roudiIpcInterface,
                        MgmtShmCharacteristics&& mgmtShmCharacteristics) noexcept;

    static void waitForRoudi(IpcInterfaceUser& roudiIpcInterface, deadline_timer& timer) noexcept;

    static RegAckResult waitForRegAck(const int64_t transmissionTimestamp,
                                      IpcInterfaceCreator& appIpcInterface,
                                      MgmtShmCharacteristics& mgmtShmCharacteristics) noexcept;

  private:
    IpcInterfaceCreator m_AppIpcInterface;
    IpcInterfaceUser m_RoudiIpcInterface;
    MgmtShmCharacteristics m_mgmtShmCharacteristics;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_RUNTIME_INTERFACE_HPP
