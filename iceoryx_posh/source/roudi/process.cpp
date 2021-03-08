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
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/platform/types.hpp"

using namespace iox::units::duration_literals;
namespace iox
{
namespace roudi
{
Process::Process(const ProcessName_t& name,
                 int32_t pid,
                 mepoo::MemoryManager* payloadMemoryManager,
                 bool isMonitored,
                 const uint64_t payloadSegmentId,
                 const uint64_t sessionId) noexcept
    : m_pid(pid)
    , m_ipcChannel(name)
    , m_timestamp(mepoo::BaseClock_t::now())
    , m_payloadMemoryManager(payloadMemoryManager)
    , m_isMonitored(isMonitored)
    , m_payloadSegmentId(payloadSegmentId)
    , m_sessionId(sessionId)
{
}

int32_t Process::getPid() const noexcept
{
    return m_pid;
}

const ProcessName_t Process::getName() const noexcept
{
    return ProcessName_t(cxx::TruncateToCapacity, m_ipcChannel.getInterfaceName());
}

void Process::sendViaIpcChannel(const runtime::IpcMessage& data) noexcept
{
    bool sendSuccess = m_ipcChannel.send(data);
    if (!sendSuccess)
    {
        LogWarn() << "Process cannot send message over communication channel";
        errorHandler(Error::kPOSH__ROUDI_PROCESS_SEND_VIA_IPC_CHANNEL_FAILED, nullptr, ErrorLevel::SEVERE);
    }
}

uint64_t Process::getSessionId() noexcept
{
    return m_sessionId.load(std::memory_order_relaxed);
}

void Process::setTimestamp(const mepoo::TimePointNs_t timestamp) noexcept
{
    m_timestamp = timestamp;
}

mepoo::TimePointNs_t Process::getTimestamp() noexcept
{
    return m_timestamp;
}

mepoo::MemoryManager* Process::getPayloadMemoryManager() const noexcept
{
    return m_payloadMemoryManager;
}

uint64_t Process::getPayloadSegmentId() const noexcept
{
    return m_payloadSegmentId;
}

bool Process::isMonitored() const noexcept
{
    return m_isMonitored;
}

} // namespace roudi
} // namespace iox
