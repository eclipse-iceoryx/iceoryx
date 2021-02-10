// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include <iostream>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

int main()
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("iox-ex-publisher-with-history");

    // create publisher options to set a historyCapacity of 10U
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.historyCapacity = 10U;

    iox::popo::TypedPublisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"}, publisherOptions);
    publisher.offer();

    double ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // Retrieve a sample, construct it with the given arguments and publish it via a lambda.
        publisher.loan(ct, ct, ct).and_then([](auto& sample) { sample.publish(); });

        std::cout << "Sent value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    return 0;
}
