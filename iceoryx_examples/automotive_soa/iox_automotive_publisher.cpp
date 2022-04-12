// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-automotive-publisher";

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::popo::Publisher<Topic> publisher({"Service", "Instance", "Event"});

    uint64_t counter = 0;
    while (!iox::posix::hasTerminationRequested())
    {
        ++counter;

        publisher.loan()
            .and_then([&](auto& sample) {
                sample->counter = counter;
                sample.publish();
            })
            .or_else([](auto& error) { std::cerr << "Unable to loan sample, error: " << error << std::endl; });

        std::cout << APP_NAME << " sent: " << counter << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
