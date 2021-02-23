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

#include <iostream>

std::atomic_bool killSwitch{false};

static void sigHandler(int32_t signal [[gnu::unused]])
{
    killSwitch.store(true);
}

void registerSigHandler()
{
    // register sigHandler for SIGINT, SIGTERM and SIGHUP
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigHandler;
    act.sa_flags = 0;

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGINT, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGINT failed" << std::endl;
    }

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGTERM, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGTERM failed" << std::endl;
    }

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGHUP, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGHUP failed" << std::endl;
    }
}

int main()
{
    registerSigHandler();

    std::cout << "Application iox_publisher_integrationtest started" << std::endl;

    iox::runtime::PoshRuntime::initRuntime("iox_publisher_integrationtest");

    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
    publisher.offer();

    for (double ct = 0.0; !killSwitch.load(); ++ct)
    {
        // API Usage #1
        //  * Retrieve a typed sample from shared memory.
        //  * Sample can be held until ready to publish.
        //  * Data is default constructed during loan
        auto result = publisher.loan();
        if (!result.has_error())
        {
            auto& sample = result.value();
            sample->x = ct;
            sample->y = ct;
            sample->z = ct;
            sample.publish();
            std::cout << "Sent value: " << ct << std::endl;
        }
        else
        {
            auto error = result.get_error();
            std::cerr << "Error while loaning mempool chunk" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Exiting application iox_publisher_integrationtest" << std::endl;
    return 0;
}
