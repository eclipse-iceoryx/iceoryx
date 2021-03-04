// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

#include <utility>

namespace iox
{
namespace units
{
struct timespec Duration::timespec(const TimeSpecReference& reference) const noexcept
{
    using SEC_TYPE = decltype(std::declval<struct timespec>().tv_sec);
    using NSEC_TYPE = decltype(std::declval<struct timespec>().tv_nsec);

    if (reference == TimeSpecReference::None)
    {
        static_assert(sizeof(uint64_t) >= sizeof(SEC_TYPE), "casting might alter result");
        if (this->m_seconds > static_cast<uint64_t>(std::numeric_limits<SEC_TYPE>::max()))
        {
            std::clog << __PRETTY_FUNCTION__ << ": Result of conversion would overflow, clamping to max value!"
                      << std::endl;
            return {std::numeric_limits<SEC_TYPE>::max(), NANOSECS_PER_SEC - 1U};
        }

        auto tv_sec = static_cast<SEC_TYPE>(this->m_seconds);
        auto tv_nsec = static_cast<NSEC_TYPE>(this->m_nanoseconds);
        return {tv_sec, tv_nsec};
    }
    else
    {
        struct timespec referenceTime;
        auto result = cxx::makeSmartC(clock_gettime,
                                      cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                      {-1},
                                      {},
                                      (reference == TimeSpecReference::Epoch) ? CLOCK_REALTIME : CLOCK_MONOTONIC,
                                      &referenceTime);
        if (result.hasErrors())
        {
            return {0, 0};
        }
        else
        {
            auto targetTime = Duration(referenceTime) + *this;

            static_assert(sizeof(uint64_t) >= sizeof(SEC_TYPE), "casting might alter result");
            if (targetTime.m_seconds > static_cast<uint64_t>(std::numeric_limits<SEC_TYPE>::max()))
            {
                std::clog << __PRETTY_FUNCTION__ << ": Result of conversion would overflow, clamping to max value!"
                          << std::endl;
                return {std::numeric_limits<SEC_TYPE>::max(), NANOSECS_PER_SEC - 1U};
            }

            auto tv_sec = static_cast<SEC_TYPE>(targetTime.m_seconds);
            auto tv_nsec = static_cast<NSEC_TYPE>(targetTime.m_nanoseconds);
            return {tv_sec, tv_nsec};
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const units::Duration& t) noexcept
{
    stream << t.m_seconds << "s " << t.m_nanoseconds << "ns";
    return stream;
}

} // namespace units
} // namespace iox
