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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);
    // we register a sighandler for SIGTERM to check if Roudi exits 
    // the process correctly for the test
    signal(SIGTERM, sigHandler);

    std::cout << "Application iox_publisher_systemtest started" << std::endl;

    iox::runtime::PoshRuntime::initRuntime("iox-ex-publisher-untyped");

    iox::popo::UntypedPublisher publisher({"Radar", "FrontLeft", "Object"});
    publisher.offer();

    double ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // API Usage #1
        //  * Loaned chunk can be held until ready to publish
        auto result = publisher.loan_1_0(sizeof(RadarObject));
        if (!result.has_error())
        {
            // In the untyped API we get a void pointer to the payload, 
            // therefore the data must be constructed
            // in place
            void* chunk = result.value();
            auto data = new (chunk) RadarObject(ct, ct, ct);

            // data and chunk should be equal. otherwise we have a misalignment
            // (note that we only requested as many bytes as needed for the object to be send)
            assert(chunk == data);
            publisher.publish(chunk);
        } else
        {
            auto error = result.get_error();
            // Do something with the error
        }


        // API Usage #2
        // * Loan chunk and provide logic to populate it via a lambda
        publisher.loan_1_0(sizeof(RadarObject))
            .and_then([&](auto& chunk) {
                auto data = new (chunk) RadarObject(ct, ct, ct);
                assert(chunk == data);
                publisher.publish(chunk);
            })
            .or_else([&](iox::popo::AllocationError error) {
                // Do something with the error
            });

        std::cout << "Sent two times value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Exiting application iox-ex-publisher-untyped" << std::endl;
    return 0;
}
