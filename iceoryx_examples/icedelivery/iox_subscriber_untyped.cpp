// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

//! [includes]
#include "topic_data.hpp"

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [includes]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-subscriber-untyped";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create untyped subscriber]
    iox::popo::UntypedSubscriber subscriber({"Radar", "FrontLeft", "Object"});
    //! [create untyped subscriber]

    // run until interrupted by Ctrl-C
    //! [loop]
    while (!iox::hasTerminationRequested())
    {
        subscriber
            .take()
            //! [chunk happy path]
            .and_then([&](const void* userPayload) {
                //! [chunk received]
                auto object = static_cast<const RadarObject*>(userPayload);
                std::cout << APP_NAME << " got value: " << object->x << std::endl;
                //! [chunk received]

                //! [release]
                // note that we explicitly have to release the sample
                // and afterwards the pointer access is undefined behavior
                subscriber.release(userPayload);
                //! [release]
            })
            //! [chunk happy path]
            .or_else([](auto& result) {
                if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                {
                    std::cout << "Error receiving chunk." << std::endl;
                }
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    //! [loop]

    return (EXIT_SUCCESS);
}
