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

#ifndef IOX_POSH_RUNTIME_HEARTBEAT_HPP
#define IOX_POSH_RUNTIME_HEARTBEAT_HPP

#include "iox/atomic.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
/// @brief A small class to handle the heartbeat
class Heartbeat
{
  public:
    Heartbeat() noexcept;

    Heartbeat(const Heartbeat&) = delete;
    Heartbeat(Heartbeat&&) = delete;

    Heartbeat& operator=(const Heartbeat&) = delete;
    Heartbeat& operator=(Heartbeat&&) = delete;

    /// @brief Get the elapsed milliseconds since the last heartbeat
    uint64_t elapsed_milliseconds_since_last_beat() const noexcept;

    /// @brief Update the heartbeat timestamp
    void beat() noexcept;

  private:
    static uint64_t milliseconds_since_epoch() noexcept;

  private:
    concurrent::Atomic<uint64_t> m_timestamp_last_beat{0};
};
} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_HEARTBEAT_HPP
