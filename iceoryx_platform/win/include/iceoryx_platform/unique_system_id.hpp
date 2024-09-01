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
#ifndef IOX_HOOFS_WIN_PLATFORM_UNIQUE_SYSTEM_ID_HPP
#define IOX_HOOFS_WIN_PLATFORM_UNIQUE_SYSTEM_ID_HPP

#include "iceoryx_platform/atomic.hpp"

#include <cstdint>
#include <string>

/// @brief IPC constructs in windows like mutex, semaphore etc are hard to handle
///        and it is even harder when this should be done in a platform independent
///        manner.
///        An easier approach is to create named mutex and semaphores and open them
///        in every process which requires access. This requires that the mutex and
///        semaphores are created with a system wide unique name. This class generates
///        a system wide unique id which looks like the following:
///        ProcessId_Timestamp_ProcessUniqueCounter.
class UniqueSystemId
{
  public:
    UniqueSystemId() noexcept;

    operator std::string() const noexcept;

    bool operator==(const UniqueSystemId& rhs) const noexcept;
    bool operator!=(const UniqueSystemId& rhs) const noexcept;
    bool operator<(const UniqueSystemId& rhs) const noexcept;

  private:
    uint64_t m_processId = 0U;
    uint64_t m_timestamp = 0U;
    uint64_t m_sequenceNumber = 0U;

    static iox::concurrent::Atomic<uint64_t> sequenceCounter;
};

#endif
