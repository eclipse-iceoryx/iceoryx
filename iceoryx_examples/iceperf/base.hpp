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
#ifndef IOX_EXAMPLES_ICEPERF_BASE_HPP
#define IOX_EXAMPLES_ICEPERF_BASE_HPP

#include "topic_data.hpp"
#include <chrono>
#include <iostream>

class IcePerfBase
{
  public:
    static constexpr uint32_t ONE_KILOBYTE = 1024U;
    virtual void initLeader() noexcept = 0;
    virtual void initFollower() noexcept = 0;
    virtual void shutdown() noexcept = 0;

    void prePingPongLeader(uint32_t payloadSizeInBytes) noexcept;
    void postPingPongLeader() noexcept;
    void releaseFollower() noexcept;
    double pingPongLeader(int64_t numRoundTrips) noexcept;
    void pingPongFollower() noexcept;

  private:
    virtual void sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept = 0;
    virtual PerfTopic receivePerfTopic() noexcept = 0;
};

#endif // IOX_EXAMPLES_ICEPERF_BASE_HPP
