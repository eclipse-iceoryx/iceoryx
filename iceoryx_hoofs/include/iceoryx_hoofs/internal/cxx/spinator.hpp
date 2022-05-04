// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_SPINATOR_HPP
#define IOX_HOOFS_CXX_SPINATOR_HPP

#include "iceoryx_hoofs/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace cxx
{
namespace internal
{
struct spinator_properties
{
    units::Duration maxWaitingTime = units::Duration::fromMilliseconds(10U);
    units::Duration initialWaitingTime = units::Duration::fromMilliseconds(0U);
    uint64_t stepCount = 10U;
    uint64_t repetitionsPerStep = 1000U;
};

class spinator
{
  public:
    explicit spinator(const spinator_properties& properties = spinator_properties()) noexcept;
    void yield() noexcept;

  private:
    spinator_properties m_properties;

    uint64_t m_yieldCount = 0U;
    units::Duration m_currentWaitingTime;
    units::Duration m_increasePerStep;
};
} // namespace internal
} // namespace cxx
} // namespace iox

#endif
