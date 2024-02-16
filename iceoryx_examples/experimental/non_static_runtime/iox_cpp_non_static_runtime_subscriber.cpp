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

constexpr char APP_NAME[] = "iox-cpp-non-static-runtime-subscriber";

int main()
{
    iox::log::Logger::init(iox::log::logLevelFromEnvOr(iox::log::LogLevel::INFO));

    iox::optional<iox::posh::experimental::Runtime> runtime;

    while (!iox::hasTerminationRequested()
           && iox::posh::experimental::RuntimeBuilder(APP_NAME).create(runtime).has_error())
    {
        std::cout << "Could not create the runtime!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (iox::hasTerminationRequested())
    {
        return EXIT_SUCCESS;
    }


    auto subscriber =
        runtime->subscriber({"Radar", "FrontLeft", "Object"}).create<RadarObject>().expect("Getting a subscriber");

    while (!iox::hasTerminationRequested())
    {
        subscriber.take()
            .and_then([](const auto& sample) { std::cout << "Receive value: " << sample->x << std::endl; })
            .or_else([](auto& result) {
                if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                {
                    std::cout << "Error receiving chunk." << std::endl;
                }
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}
