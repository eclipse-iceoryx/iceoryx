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

#include "iox/vector.hpp"

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

constexpr char APP_NAME[] = "iox-cpp-publisher-vector";

int main()
{
    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // initialize publisher
    //! [create publisher]
    iox::popo::Publisher<iox::vector<double, 5>> publisher({"Radar", "FrontRight", "VectorData"});
    //! [create publisher]

    uint64_t ct = 0;
    // run until interrupted by Ctrl-C
    while (!iox::hasTerminationRequested())
    {
        publisher.loan()
            .and_then([&](auto& sample) {
                //! [vector emplace_back]
                for (uint64_t i = 0U; i < sample->capacity(); ++i)
                {
                    // we can omit the check of the return value since the loop doesn't exceed the capacity of the
                    // vector
                    sample->emplace_back(static_cast<double>(ct + i));
                }
                //! [vector emplace_back]

                sample.publish();
            })
            .or_else([](auto& error) {
                // do something with error
                std::cerr << "Unable to loan sample, error code: " << error << std::endl;
            });
        ++ct;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (EXIT_SUCCESS);
}
