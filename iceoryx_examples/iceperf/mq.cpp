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

#include "mq.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <chrono>

MQ::MQ(const std::string& publisherName, const std::string& subscriberName) noexcept
{
}

void MQ::initLeader() noexcept
{

}

void MQ::initFollower() noexcept
{

}

void MQ::shutdown() noexcept
{
}


void MQ::prePingPongLeader(uint32_t payloadSizeInBytes) noexcept
{

}

void MQ::postPingPongLeader() noexcept
{
}

void MQ::triggerEnd() noexcept
{
}

double MQ::pingPongLeader(int64_t numRoundTrips) noexcept
{
    return 0.0;
}

void MQ::pingPongFollower() noexcept
{

}