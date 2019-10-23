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

#include "iceoryx_utils/internal/units/duration.hpp"

namespace iox
{
namespace units
{
struct timespec Duration::timespec(const TimeSpecReference& reference) const
{
    switch (reference)
    {
    case TimeSpecReference::None:
        return {this->seconds<long>(), this->nanoSeconds<long>() - this->seconds<long>() * 1000000000};
    default:
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
            constexpr int64_t NanoSecondsPerSecond{1000000000};
            int64_t remainingNanoSecondsTimeout = this->nanoSeconds<int64_t>() % NanoSecondsPerSecond;
            int64_t sumOfNanoSeconds = remainingNanoSecondsTimeout + referenceTime.tv_nsec;
            int64_t seconds = this->seconds<int64_t>() + referenceTime.tv_sec + sumOfNanoSeconds / NanoSecondsPerSecond;
            int64_t nanoSeconds = sumOfNanoSeconds % NanoSecondsPerSecond;

            return {seconds, nanoSeconds};
        }
    }
    }
}

std::ostream& operator<<(std::ostream& stream, const units::Duration& t)
{
    stream << t.durationInSeconds << "s ";
    return stream;
}

} // namespace units
} // namespace iox
