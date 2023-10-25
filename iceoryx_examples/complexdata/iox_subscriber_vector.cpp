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

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
#include "iox/string.hpp"
#include "iox/vector.hpp"

constexpr char APP_NAME[] = "iox-cpp-subscriber-vector";

int main()
{
    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // initialize subscriber
    iox::popo::Subscriber<iox::vector<double, 5>> subscriber({"Radar", "FrontRight", "VectorData"});

    // run until interrupted by Ctrl-C
    while (!iox::hasTerminationRequested())
    {
        subscriber.take()
            .and_then([](auto& sample) {
                std::stringstream s;
                s << APP_NAME << " got values:";

                const char* separator = " ";
                //! [vector output]
                for (const auto& entry : *sample)
                {
                    s << separator << entry;
                    separator = ", ";
                }
                //! [vector output]

                std::cout << s.str() << std::endl;
            })
            .or_else([](auto& result) {
                // only has to be called if the alternative is of interest,
                // i.e. if nothing has to happen when no data is received and
                // a possible error alternative is not checked or_else is not needed
                if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                {
                    std::cout << "Error receiving chunk." << std::endl;
                }
            });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (EXIT_SUCCESS);
}
