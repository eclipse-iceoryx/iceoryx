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

#include "topic_data.hpp"

//! [include subscriber]
#include "iceoryx_posh/popo/subscriber.hpp"
//! [include subscriber]
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-subscriber";

int main()
{
    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create subscriber]
    iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
    //! [create subscriber]

    // run until interrupted by Ctrl-C
    while (!iox::hasTerminationRequested())
    {
        subscriber
            .take()
            //! [sample happy path]
            .and_then([](auto& sample) {
                //! [print sample info]
                std::cout << APP_NAME << " got value: " << sample->x << std::endl;
                //! [print sample info]
            })
            //! [sample happy path]
            .or_else([](auto& result) {
                // only has to be called if the alternative is of interest,
                // i.e. if nothing has to happen when no data is received and
                // a possible error alternative is not checked or_else is not needed
                if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                {
                    std::cout << "Error receiving chunk." << std::endl;
                }
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}
