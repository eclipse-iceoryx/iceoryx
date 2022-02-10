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

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

constexpr char APP_NAME[] = "iox-offer-service";

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // offer services by creating publishers
    iox::popo::Publisher<uint32_t> radarLeft({"Radar", "FrontLeft", "SequenceCounter"});
    iox::popo::Publisher<uint32_t> radarRight({"Radar", "FrontRight", "SequenceCounter"});
    iox::popo::Publisher<uint32_t> lidarLeft({"Lidar", "FrontLeft", "Counter"});

    iox::cxx::vector<iox::popo::Publisher<uint32_t>, 5> cameraPublishers;
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontLeft", "Counter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontLeft", "SequenceCounter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontRight", "Counter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontRight", "SequenceCounter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "BackLeft", "SequenceCounter"});

    bool offer = false;
    while (!iox::posix::hasTerminationRequested())
    {
        if (offer)
        {
            for (auto& publisher : cameraPublishers)
            {
                publisher.offer();
            }
        }
        else
        {
            for (auto& publisher : cameraPublishers)
            {
                publisher.stopOffer();
            }
        }
        offer = !offer;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (EXIT_SUCCESS);
}
