// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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
#include "iox/posh/experimental/node.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-node-publisher";

int main()
{
    iox::log::Logger::init(iox::log::logLevelFromEnvOr(iox::log::LogLevel::INFO));

    double value = 0.0;
    while (!iox::hasTerminationRequested())
    {
        // open a new scope to destroy the node before the sleep
        {
            auto node_result = iox::posh::experimental::NodeBuilder(APP_NAME)
                                   .domain_id_from_env_or_default()
                                   .roudi_registration_timeout(iox::units::Duration::fromSeconds(1))
                                   .create();
            if (node_result.has_error())
            {
                std::cout << "Could not create the node!" << std::endl;
                continue;
            }

            auto node = std::move(node_result.value());
            auto publisher =
                node.publisher({"Radar", "FrontLeft", "Object"}).create<RadarObject>().expect("Getting a publisher");

            publisher->loan()
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
