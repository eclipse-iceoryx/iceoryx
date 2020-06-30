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
#include "iceoryx_utils/platform/platform_correction.hpp"

namespace iox
{
namespace units
{
struct timespec Duration::timespec(const TimeSpecReference& reference) const
{
    constexpr int64_t NanoSecondsPerSecond{1000000000};
    if (reference == TimeSpecReference::None)
    {
        int64_t timeInNanoSeconds = this->nanoSeconds<int64_t>();
        int64_t seconds = timeInNanoSeconds / NanoSecondsPerSecond;
        return {seconds, timeInNanoSeconds - seconds * NanoSecondsPerSecond};
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
            int64_t timeInNanoSeconds = this->nanoSeconds<int64_t>();
            int64_t remainingNanoSecondsTimeout = timeInNanoSeconds % NanoSecondsPerSecond;
            int64_t sumOfNanoSeconds = remainingNanoSecondsTimeout + referenceTime.tv_nsec;
            int64_t additionalSeconds = sumOfNanoSeconds / NanoSecondsPerSecond;
            int64_t seconds = timeInNanoSeconds / NanoSecondsPerSecond + referenceTime.tv_sec + additionalSeconds;
            int64_t nanoSeconds = sumOfNanoSeconds - additionalSeconds * NanoSecondsPerSecond;

            return {seconds, nanoSeconds};
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
