// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include <atomic>
#include <chrono>
#include <cstdint>

namespace iox
{
namespace mepoo
{
using SequenceNumberType = std::uint32_t;
using PureLocalTB = std::chrono::steady_clock; // atm sure PureLocalTB
using BaseClock = PureLocalTB;                 // to be able to go to a synced clock
using SamplesCounterType = std::uint8_t;       // AGR counter for send and received samples

// use signed integer for duration;
// there is a bug in gcc 4.8 which leads to a wrong calcutated time
// when sleep_until() is used with a timepoint in the past
using DurationNs = std::chrono::duration<std::int64_t, std::nano>;
using TimePointNs = std::chrono::time_point<BaseClock, DurationNs>;

struct ChunkInfo
{
    bool m_externalSequenceNumber_bl{false};
    SequenceNumberType m_sequenceNumber{0};

    /// @brief size of the user data object
    std::uint32_t m_payloadSize{0};

    /// @brief size of header and used payload (remaining bytes of the memory chunk are not counted)
    std::uint32_t m_usedSizeOfChunk{0};

    TimePointNs m_txTimestamp;
};

} // namespace mepoo
} // namespace iox

