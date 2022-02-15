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

#include "discovery_monitor.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-monitor-discovery";

void printSearchResult(const iox::runtime::ServiceContainer result)
{
    std::cout << "Searchresult " << std::endl;
    for (auto entry : result)
    {
        std::cout << entry.getServiceIDString() << ", " << entry.getInstanceIDString() << ", "
                  << entry.getEventIDString() << std::endl;
    }
}

using namespace discovery;

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    DiscoveryMonitor discovery;

    auto callback = [&](iox::runtime::ServiceDiscovery& discovery) -> void {
        auto result = discovery.findService(
            iox::capro::IdString_t{"Radar"}, iox::capro::IdString_t{"FrontLeft"}, iox::capro::IdString_t{"Counter"});

        if (result.size() > 0)
        {
            std::cout << "Discovery Monitor: service available" << std::endl;
            printSearchResult(result);
        }
        else
        {
            std::cout << "Discovery Monitor: service unavailable" << std::endl;
        }
    };

    // only one callback allowed, hence no handles required
    discovery.registerCallback(callback);

    while (!iox::posix::hasTerminationRequested())
        ;

    discovery.deregisterCallback();

    return (EXIT_SUCCESS);
}
