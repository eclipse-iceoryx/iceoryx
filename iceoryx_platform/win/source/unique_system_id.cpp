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

#include "iceoryx_platform/unique_system_id.hpp"
#include "iceoryx_platform/atomic.hpp"
#include "iceoryx_platform/windows.hpp"

#include <chrono>
#include <processthreadsapi.h>


iox::concurrent::Atomic<uint64_t> UniqueSystemId::sequenceCounter{0U};

UniqueSystemId::UniqueSystemId() noexcept
    : m_processId{GetCurrentProcessId()}
    , m_timestamp{static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count())}
    , m_sequenceNumber{sequenceCounter.fetch_add(1U, std::memory_order_relaxed)}
{
}

bool UniqueSystemId::operator==(const UniqueSystemId& rhs) const noexcept
{
    return m_processId == rhs.m_processId && m_timestamp == rhs.m_timestamp && m_sequenceNumber == rhs.m_sequenceNumber;
}

bool UniqueSystemId::operator!=(const UniqueSystemId& rhs) const noexcept
{
    return !(*this == rhs);
}

bool UniqueSystemId::operator<(const UniqueSystemId& rhs) const noexcept
{
    return (m_processId < rhs.m_processId) || (m_processId == rhs.m_processId && m_timestamp < rhs.m_timestamp)
           || (m_processId == rhs.m_processId && m_timestamp == rhs.m_timestamp
               && m_sequenceNumber < rhs.m_sequenceNumber);
}

UniqueSystemId::operator std::string() const noexcept
{
    return std::to_string(m_processId) + "_" + std::to_string(m_timestamp) + "_" + std::to_string(m_sequenceNumber);
}
