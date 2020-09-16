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

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/experimental/popo/subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include <thread>
#include <chrono>
#include <csignal>
#include <iostream>

struct Position {
    Position(double_t x, double_t y, double_t z) : x(x), y(y), z(z)
    {};
    double_t x = 0.0;
    double_t y = 0.0;
    double_t z = 0.0;
};

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

void receiving()
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-modern");
    iox::popo::TypedSubscriber<Position> mySubscriber({"Odometry", "Position", "Vehicle"});
    mySubscriber.subscribe(10);

    while (!killswitch)
    {

        iox::popo::ConditionVariableData m_condVarData;
        iox::popo::WaitSet waitSet{&m_condVarData};

    }

    mySubscriber.unsubscribe();
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
