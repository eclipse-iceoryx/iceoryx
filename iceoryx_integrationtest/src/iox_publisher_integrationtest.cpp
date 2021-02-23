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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <iostream>

std::atomic_bool killSwitch{false};

static void sigHandler(int32_t signal [[gnu::unused]])
{
    killSwitch.store(true);
}

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    std::cout << "Application iox_publisher_integrationtest started" << std::endl;

    iox::runtime::PoshRuntime::initRuntime("iox_publisher_integrationtest");

    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
    publisher.offer();

    for (double ct = 0.0; !killSwitch.load(); ++ct)
    {
        // API Usage #3
        //  * Retrieve a sample and provide the logic to immediately populate and publish it via a lambda.
        //
        publisher.loan()
            .and_then([&](auto& sample) {
                auto object = sample.get();
                // Do some stuff leading to eventually generating the data in the samples loaned memory...
                *object = RadarObject(ct, ct, ct);
                // ...then publish the sample
                sample.publish();
                std::cout << "Sent value: " << ct << std::endl;
            })
            .or_else([](iox::popo::AllocationError) { std::cerr << "Error while loaning mempool chunk" << std::endl; });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Exiting application iox_publisher_integrationtest" << std::endl;
    return 0;
}
