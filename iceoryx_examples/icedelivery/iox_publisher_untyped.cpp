// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/untyped_publisher.hpp"
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

    iox::runtime::PoshRuntime::initRuntime("iox-ex-publisher-untyped");

    iox::popo::UntypedPublisher untypedPublisher({"Radar", "FrontLeft", "Object"});
    untypedPublisher.offer();

    double ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // API Usage #1
        //  * Loaned sample can be held until ready to publish
        auto result = untypedPublisher.loan(sizeof(RadarObject));
        if (!result.has_error())
        {
            auto& sample = result.value();
            // In the untyped API, the returned sample is a void pointer, therefore the data must be constructed
            // in place
            auto object = static_cast<RadarObject*>(sample.get());
            *object = RadarObject(ct, ct, ct);
            sample.publish();
        }
        else
        {
            auto error = result.get_error();
            // Do something with error
        }


        // API Usage #2
        // * Loan sample and provide logic to use it immediately via a lambda
        untypedPublisher.loan(sizeof(RadarObject))
            .and_then([&](auto& sample) {
                auto object = static_cast<RadarObject*>(sample.get());
                *object = RadarObject(ct, ct, ct);
                sample.publish();
            })
            .or_else([&](iox::popo::AllocationError error) {
                // Do something with error
            });

        std::cout << "Sent two times value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
