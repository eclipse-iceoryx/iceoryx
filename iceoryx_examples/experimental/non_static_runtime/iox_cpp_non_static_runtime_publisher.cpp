// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iox/optional.hpp"
#include "iox/posh/experimental/runtime.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-non-static-runtime-publisher";

int main()
{
    iox::log::Logger::init(iox::log::logLevelFromEnvOr(iox::log::LogLevel::INFO));

    double value = 0.0;
    while (!iox::hasTerminationRequested())
    {
        // open a new scope to destroy the runtime before the sleep
        // the 'reset' method of the optional cannot be used since there would still be the publisher who needs to
        // access the shared memory on destruction -> get rid of the optional and make the runtime movable
        {
            iox::optional<iox::posh::experimental::Runtime> runtime;
            if (iox::posh::experimental::RuntimeBuilder(APP_NAME).create(runtime).has_error())
            {
                std::cout << "Could not create the runtime!" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            auto publisher = runtime->publisher({"Radar", "FrontLeft", "Object"})
                                 .create<RadarObject>()
                                 .expect("Getting a publisher");

            publisher.loan()
                .and_then([&](auto& sample) {
                    sample->x = value;
                    sample->y = value;
                    sample->z = value;
                    sample.publish();
                })
                .expect("Getting a sample");

            std::cout << "Sent value: " << value << std::endl;
        }

        value += 1.0;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
