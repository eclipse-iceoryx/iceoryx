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

class IcePerfBase
{

  public:
    static constexpr uint32_t ONE_KILOBYTE = 1024u;
    virtual void initLeader() noexcept = 0;
    virtual void initFollower() noexcept = 0;
    virtual void shutdown() noexcept = 0;
    virtual void prePingPongLeader(uint32_t payloadSizeInBytes) noexcept = 0;
    virtual void postPingPongLeader() noexcept = 0;
    virtual void triggerEnd() noexcept = 0;
    virtual double pingPongLeader(int64_t numRoundTrips) noexcept = 0;
    virtual void pingPongFollower() noexcept = 0;
};

#endif // IOX_EXAMPLES_ICEPERF_BASE_HPP