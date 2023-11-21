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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

constexpr char APP_NAME[] = "iox-offer-service";

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // offer services by creating publishers
    iox::popo::Publisher<uint32_t> radarLeft({"Radar", "FrontLeft", "Objects"});
    iox::popo::Publisher<uint32_t> radarRight({"Radar", "FrontRight", "Objects"});
    iox::popo::Publisher<uint32_t> lidarLeft({"Lidar", "FrontLeft", "Counter"});

    iox::vector<iox::popo::Publisher<uint32_t>, 5> cameraPublishers;
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontLeft", "Counter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontLeft", "Image"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontRight", "Counter"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "FrontRight", "Image"});
    cameraPublishers.emplace_back(iox::capro::ServiceDescription{"Camera", "BackLeft", "Image"});

    bool offer = false;
    while (!iox::hasTerminationRequested())
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
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return (EXIT_SUCCESS);
}
