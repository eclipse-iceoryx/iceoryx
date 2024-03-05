// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/process.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/logging.hpp"

using namespace iox::units::duration_literals;
namespace iox
{
namespace roudi
{
Process::Process(const RuntimeName_t& name,
                 const DomainId domainId,
                 const uint32_t pid,
                 const PosixUser& user,
                 const HeartbeatPoolIndexType heartbeatPoolIndex,
                 const uint64_t sessionId) noexcept
    : m_pid(pid)
    , m_ipcChannel(name, domainId, ResourceType::USER_DEFINED)
    , m_heartbeatPoolIndex(heartbeatPoolIndex)
    , m_user(user)
    , m_sessionId(sessionId)
{
}

uint32_t Process::getPid() const noexcept
{
    return m_pid;
}

const RuntimeName_t Process::getName() const noexcept
{
    return m_ipcChannel.getRuntimeName();
}

void Process::sendViaIpcChannel(const runtime::IpcMessage& data) noexcept
{
    bool sendSuccess = m_ipcChannel.send(data);
    if (!sendSuccess)
    {
        IOX_LOG(WARN, "Process cannot send message over communication channel");
        IOX_REPORT(PoshError::POSH__ROUDI_PROCESS_SEND_VIA_IPC_CHANNEL_FAILED, iox::er::RUNTIME_ERROR);
    }
}

uint64_t Process::getSessionId() noexcept
{
    return m_sessionId.load(std::memory_order_relaxed);
}

PosixUser Process::getUser() const noexcept
{
    return m_user;
}

HeartbeatPoolIndexType Process::getHeartbeatPoolIndex() const noexcept
{
    return m_heartbeatPoolIndex;
}

bool Process::isMonitored() const noexcept
{
    return m_heartbeatPoolIndex != HeartbeatPool::Index::INVALID;
}

} // namespace roudi
} // namespace iox
