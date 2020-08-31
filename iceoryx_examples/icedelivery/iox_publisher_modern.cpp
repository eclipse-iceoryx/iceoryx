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

#include "iceoryx_posh/experimental/popo/typed_publisher.hpp"
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

void getVehiclePosition(Position* allocation)
{
    new (allocation) Position(11.11, 22.22, 33.33);
}

int main(int argc, char *argv[])
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-publisher-modern");

    auto typedPublisher = iox::popo::TypedPublisher<Position>({"Odometry", "Position", "Vehicle"});
    //auto untypedPublisher = iox::popo::Publisher<iox::popo::Untyped>({"Odometry", "Position", "Vehicle"});

    typedPublisher.offer();
    //untypedPublisher.offer();

    float_t ct = 0.0;
    while(!killswitch)
    {
        typedPublisher.loan()
            .and_then([&](iox::popo::Sample<Position>& sample){
                ++ct;
                sample.emplace(ct, ct, ct);
                sample.publish();
            });
            std::this_thread::sleep_for(std::chrono::seconds(1));

//        typedPublisher.publishResultOf(getVehiclePosition);

        //untypedPublisher.loan();
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }

    return 0;
}
