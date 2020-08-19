// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "base.hpp"


void IcePerfBase::prePingPongLeader(uint32_t payloadSizeInBytes) noexcept
{
    sendPerfTopic(payloadSizeInBytes, true);
}

void IcePerfBase::postPingPongLeader() noexcept
{
    // Wait for the last response
    receivePerfTopic();

    std::cout << "done" << std::endl;
}

void IcePerfBase::releaseFollower() noexcept
{
    sendPerfTopic(sizeof(PerfTopic), false);
}

double IcePerfBase::pingPongLeader(int64_t numRoundTrips) noexcept
{
    auto start = std::chrono::high_resolution_clock::now();

    // run the performance test
    for (auto i = 0U; i < numRoundTrips; ++i)
    {
        auto perfTopic = receivePerfTopic();
        sendPerfTopic(perfTopic.payloadSize, true);
    }

    auto finish = std::chrono::high_resolution_clock::now();

    constexpr int64_t TRANSMISSIONS_PER_ROUNDTRIP{2};
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    auto latencyInNanoSeconds = (duration.count() / (numRoundTrips * TRANSMISSIONS_PER_ROUNDTRIP));
    auto latencyInMicroSeconds = static_cast<double>(latencyInNanoSeconds) / 1000;
    return latencyInMicroSeconds;
}

void IcePerfBase::pingPongFollower() noexcept
{
    while (true)
    {
        auto perfTopic = receivePerfTopic();

        // stop replying when no more run
        if (!perfTopic.run)
        {
            break;
        }

        sendPerfTopic(perfTopic.payloadSize, true);
    }
}
