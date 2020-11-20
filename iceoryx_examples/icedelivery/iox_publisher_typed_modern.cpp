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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/modern_api/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <chrono>
#include <iostream>
#include <thread>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

void getVehiclePosition(Position* const allocation, const uint64_t& val) noexcept
{
    new (allocation) Position(val, val, val);
}

int main()
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::getInstance("/iox-ex-publisher-typed-modern");

    auto typedPublisher = iox::popo::TypedPublisher<Position>({"Odometry", "Position", "Vehicle"});
    typedPublisher.offer();

    float_t ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // API Usage #1
        //  * Retrieve a typed sample from shared memory.
        //  * Sample can be held until ready to publish.
        //
        auto result = typedPublisher.loan();
        if (!result.has_error())
        {
            auto& sample = result.get_value();
            sample->x = ct;
            sample->y = ct;
            sample->z = ct;
            sample.publish();
        }

        // API Usage #2
        //  * Retrieve a sample and provide the logic to immediately populate and publish it via a lambda.
        //
        typedPublisher.loan().and_then([&](iox::popo::Sample<Position>& sample) {
            auto allocation = sample.get();
            // Do some stuff leading to eventually generating the data in the samples loaned memory...
            new (allocation) Position(ct, ct, ct);
            // ...then publish the sample
            sample.publish();
        }).or_else([](iox::popo::AllocationError){
            // Do something with error.
        });


        // API Usage #3
        //  * Basic copy-and-publish. Useful for smaller data types.
        //
        auto position = Position(ct, ct, ct);
        typedPublisher.publishCopyOf(position);

        // API Usage #4
        //  * Provide a callable that will be used to populate the loaned sample.
        //  * The first argument of the callable must be T* and is the location that the callable should
        //      write its result to.
        //
        typedPublisher.publishResultOf(getVehiclePosition, ct);
        typedPublisher.publishResultOf([&ct](Position* allocation) { new (allocation) Position(ct, ct, ct); });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
