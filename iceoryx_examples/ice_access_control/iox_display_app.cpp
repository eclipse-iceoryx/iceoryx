// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "topic_data.hpp"

#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-display";

int main()
{
    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // initialized subscriber
    iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
    iox::popo::Publisher<RadarObject> publisher({"Radar", "HMI-Display", "Object"});

    // run until interrupted by Ctrl-C
    while (!iox::posix::hasTerminationRequested())
    {
        auto takeResult = subscriber.take();

        if (!takeResult.has_error())
        {
            publisher.loan().and_then([&](auto& sample) {
                sample->x = 2 * takeResult.value()->x;
                sample->y = 2 * takeResult.value()->y;
                sample->z = 2 * takeResult.value()->z;
                std::cout << APP_NAME << " sending value: " << takeResult.value()->x << std::endl;
                publish(std::move(sample));
            });
        }
        else
        {
            if (takeResult.get_error() == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
            {
                std::cout << "No chunk available." << std::endl;
            }
            else
            {
                std::cout << "Error receiving chunk." << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}
