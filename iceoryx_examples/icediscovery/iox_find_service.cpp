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

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-find-service";

void printSearchResult(const iox::cxx::expected<iox::runtime::ServiceContainer, iox::runtime::FindServiceError>& res)
{
    res.and_then([](auto& serviceContainer) {
           for (auto& service : serviceContainer)
           {
               std::cout << "- " << service << std::endl;
           }
           std::cout << std::endl;
       })
        .or_else([](auto& error) {
            std::cerr << "findService() call failed with: " << static_cast<uint64_t>(error) << std::endl;
        });
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::runtime::ServiceDiscovery serviceDiscovery;

    while (!iox::posix::hasTerminationRequested())
    {
        std::cout << "=========================================" << std::endl;

        std::cout << "Searched for {'Radar', 'FrontLeft', 'SequenceCounter'} and found the following events: "
                  << std::endl;
        auto searchResult = serviceDiscovery.findService(iox::capro::IdString_t{"Radar"},
                                                         iox::capro::IdString_t{"FrontLeft"},
                                                         iox::capro::IdString_t{"SequenceCounter"});
        printSearchResult(searchResult);

        std::cout << "Searched for {'Radar', *, *} and found the following events: " << std::endl;
        searchResult =
            serviceDiscovery.findService(iox::capro::IdString_t{"Radar"}, iox::capro::Wildcard, iox::capro::Wildcard);
        printSearchResult(searchResult);

        std::cout << "Searched for {*, 'FrontLeft', *} and found the following events: " << std::endl;
        searchResult = serviceDiscovery.findService(
            iox::capro::Wildcard, iox::capro::IdString_t{"FrontLeft"}, iox::capro::Wildcard);
        printSearchResult(searchResult);

        std::cout << "Searched for {*, FrontRight, SequenceCounter} and found the following events: " << std::endl;
        searchResult = serviceDiscovery.findService(
            iox::capro::Wildcard, iox::capro::IdString_t{"FrontRight"}, iox::capro::IdString_t{"SequenceCounter"});
        printSearchResult(searchResult);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (EXIT_SUCCESS);
}
