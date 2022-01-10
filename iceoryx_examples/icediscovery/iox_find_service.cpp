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

#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
//! [include servicediscovery]
#include "iceoryx_posh/runtime/service_discovery.hpp"
//! [include servicediscovery]
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

bool killswitch = false;
constexpr char APP_NAME[] = "iox-cpp-find-service";

static void sigHandler(int f_sig IOX_MAYBE_UNUSED)
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::runtime::ServiceDiscovery serviceDiscovery;

    // run until interrupted by Ctrl-C
    while (!killswitch)
    {
        /// @todo #415 Instead of polling should we showcase the getServiceRegistryChangeCounter() or will that be
        /// removed?

        /// @todo #415 Why is the implicit conversion to cxx::variant not working?
        serviceDiscovery.findService(iox::capro::IdString_t("Radar"), iox::capro::IdString_t("FrontLeft"))
            .and_then([](auto& serviceContainter) {
                /// @todo #415 Maybe use some colors to beautify the output?
                std::cout << "Searched for {'Radar', 'FrontLeft', '*'} and found the following events: " << std::endl;
                for (auto& service : serviceContainter)
                {
                    std::cout << "  - " << service << std::endl;
                }
                std::cout << std::endl;
            })
            .or_else([](auto& error) {
                std::cerr << "findService() call failed with: " << static_cast<uint64_t>(error) << std::endl;
            });


        serviceDiscovery.findService(iox::capro::IdString_t("Radar"), iox::runtime::Wildcard_t())
            .and_then([](auto& serviceContainter) {
                std::cout << "Searched for {'Radar', '*', '*'} and found the following events: " << std::endl;
                for (auto& service : serviceContainter)
                {
                    std::cout << "  - " << service << std::endl;
                }
                std::cout << std::endl;
            })
            .or_else([](auto& error) {
                std::cerr << "findService() call failed with: " << static_cast<uint64_t>(error) << std::endl;
            });
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }


    return (EXIT_SUCCESS);
}
