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

//! [include]
#include "topic_data.hpp"

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [include]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-subscriber-helloworld";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [initialize subscriber]
    iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
    //! [initialize subscriber]

    // run until interrupted by Ctrl-C
    while (!iox::hasTerminationRequested())
    {
        //! [receive]
        auto takeResult = subscriber.take();
        if (takeResult.has_value())
        {
            std::cout << APP_NAME << " got value: " << takeResult.value()->x << std::endl;
        }
        //! [receive]
        else
        {
            //! [error]
            if (takeResult.error() == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
            {
                std::cout << "No chunk available." << std::endl;
            }
            else
            {
                std::cout << "Error receiving chunk." << std::endl;
            }
            //! [error]
        }

        //! [wait]
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //! [wait]
    }

    return (EXIT_SUCCESS);
}
