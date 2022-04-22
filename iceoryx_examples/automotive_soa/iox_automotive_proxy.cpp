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

#include "owl/runtime.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "minimal_proxy.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-automotive-proxy";

int main()
{
    owl::Runtime::GetInstance(APP_NAME);

    MinimalProxy proxy;

    // auto serviceContainer = proxy.FindService();
    // for(auto& service : serviceContainer)
    // {
    //  service.subscribe();
    // }

    uint64_t addend1{0};
    uint64_t addend2{0};

    while (!iox::posix::hasTerminationRequested())
    {
        // Event
        proxy.m_event.GetNewSamples(
            [](const auto& topic) { std::cout << "Receiving event: " << topic->counter << std::endl; });

        // Field
        proxy.m_field.GetNewSamples(
            [](const auto& field) { std::cout << "Receiving field: " << field->counter << std::endl; });

        // // Method
        // auto future = proxy.computeSum(addend1, addend2);
        // try
        // {
        //     auto result = future.get();
        //     std::cout << "Result of " << std::to_string(addend1) << " + " << std::to_string(addend2) << " is "
        //               << result.sum << std::endl;
        // }
        // catch (const std::future_error& e)
        // {
        //     std::cout << "Empty future received, please start the 'iox-cpp-automotive-skeleton' before '" << APP_NAME << "'"
        //               << std::endl;
        // }

        addend1 += addend2 + addend2;
        addend2++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return (EXIT_SUCCESS);
}
