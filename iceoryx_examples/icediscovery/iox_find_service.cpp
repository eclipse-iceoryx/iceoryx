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

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [include ServiceDiscovery]
#include "iceoryx_posh/runtime/service_discovery.hpp"
//! [include ServiceDiscovery]

#include <iostream>

constexpr char APP_NAME[] = "iox-find-service";

//! [print function to be applied to search results]
void printSearchResult(const iox::capro::ServiceDescription& service)
{
    std::cout << "- " << service << std::endl;
}
//! [print function to be applied to search results]

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create ServiceDiscovery object]
    iox::runtime::ServiceDiscovery serviceDiscovery;
    //! [create ServiceDiscovery object]

    while (!iox::hasTerminationRequested())
    {
        std::cout << "\n=========================================" << std::endl;

        std::cout << "\nSearched for {'Radar', 'FrontLeft', 'Objects'}. Found the following services:" << std::endl;
        //! [search for unique service]
        serviceDiscovery.findService(iox::capro::IdString_t{"Radar"},
                                     iox::capro::IdString_t{"FrontLeft"},
                                     iox::capro::IdString_t{"Objects"},
                                     printSearchResult,
                                     iox::popo::MessagingPattern::PUB_SUB);
        //! [search for unique service]

        std::cout << "\nSearched for {'Radar', *, *}. Found the following services:" << std::endl;
        serviceDiscovery.findService(iox::capro::IdString_t{"Radar"},
                                     iox::capro::Wildcard,
                                     iox::capro::Wildcard,
                                     printSearchResult,
                                     iox::popo::MessagingPattern::PUB_SUB);

        std::cout << "\nSearched for {*, 'FrontLeft', *}. Found the following services:" << std::endl;
        serviceDiscovery.findService(iox::capro::Wildcard,
                                     iox::capro::IdString_t{"FrontLeft"},
                                     iox::capro::Wildcard,
                                     printSearchResult,
                                     iox::popo::MessagingPattern::PUB_SUB);

        std::cout << "\nSearched for {*, 'FrontRight', 'Image'}. Found the following services:" << std::endl;
        serviceDiscovery.findService(iox::capro::Wildcard,
                                     iox::capro::IdString_t{"FrontRight"},
                                     iox::capro::IdString_t{"Image"},
                                     printSearchResult,
                                     iox::popo::MessagingPattern::PUB_SUB);

        std::cout << "\nSearched for {'Camera', *, *}. Found the following services:" << std::endl;
        //! [search for all Camera services]
        serviceDiscovery.findService(iox::capro::IdString_t{"Camera"},
                                     iox::capro::Wildcard,
                                     iox::capro::Wildcard,
                                     printSearchResult,
                                     iox::popo::MessagingPattern::PUB_SUB);
        //! [search for all Camera services]

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (EXIT_SUCCESS);
}
