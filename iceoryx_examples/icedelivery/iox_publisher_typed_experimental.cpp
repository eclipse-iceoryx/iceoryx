// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>
#include <chrono>
#include <thread>

struct Position {
    Position(double_t x, double_t y, double_t z) : x(x), y(y), z(z)
    {};
    double_t x = 0.0;
    double_t y = 0.0;
    double_t z = 0.0;
};

bool killswitch = false;

static void sigHandler(int f_sig[[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

void getVehiclePosition(Position* allocation, uint64_t multiplier)
{
    new (allocation) Position(1111.1111 * multiplier, 1111.1111 * multiplier, 1111.1111* multiplier);
}

int main(int argc, char *argv[])
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-publisher-modern");

    auto typedPublisher = iox::popo::TypedPublisher<Position>({"Odometry", "Position", "Vehicle"});
    typedPublisher.offer();

    float_t ct = 0.0;
    while(!killswitch)
    {
        ++ct;

        // Retrieve a typed sample from shared memory.
        // Sample can be held until ready to publish.
        auto result = typedPublisher.loan();
        if(!result.has_error())
        {
            auto& sample = result.get_value();
            auto val = sample.get();
            val->x = ct * 1.1;
            val->y = ct * 1.1;
            val->z = ct * 1.1;
            typedPublisher.publish(sample);
        }

        // Retrieve a sample and provide logic to immediately populate and publish via a lambda.
        typedPublisher.loan()
            .and_then([&](iox::popo::Sample<Position>& sample){
                auto allocation = sample.get();
                // Do some stuff leading to eventually generating the data in the provided sample's shared memory...
                new (allocation) Position(ct * 11.11, ct * 11.11, ct * 11.11);
                // ...then publish the bytes
                sample.publish();
            });

        // Simple copy-and-publish. Useful for smaller data types.
        auto position = Position(ct * 111.111, ct * 111.111, ct * 111.111);
        typedPublisher.publishCopyOf(position);

        // Samples can be generated within any callable and written directly to the loaned memory
        // allocation. The first argument of the callable must be T*, this will point to the memory
        // allocation.
        typedPublisher.publishResultOf(getVehiclePosition, ct);
        typedPublisher.publishResultOf([](Position* allocation){
            new (allocation) Position(0, 0, 0);
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
