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

//! [include custom discovery]
#include "discovery_monitor.hpp"
//! [include custom discovery]

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

using namespace discovery;

constexpr char APP_NAME[] = "iox-monitor-discovery";

iox::capro::IdString_t service{"Camera"};
iox::capro::IdString_t instance{"FrontLeft"};
iox::capro::IdString_t event{"Image"};

void printSearchResult(const discovery::ServiceContainer& result)
{
    std::cout << APP_NAME << " search result:" << (result.empty() ? "empty\n" : "") << std::endl;

    for (const auto& entry : result)
    {
        std::cout << APP_NAME << " " << entry.getServiceIDString() << ", " << entry.getInstanceIDString() << ", "
                  << entry.getEventIDString() << "\n"
                  << std::endl;
    }
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create custom discovery]
    // requires the runtime to be created first
    Discovery discovery;
    //! [create custom discovery]

    //! [create monitoring callback]
    auto callback = [&](Discovery& discovery) -> void {
        auto result = discovery.findService(service, instance, event);

        if (!result.empty())
        {
            std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> available" << std::endl;
        }
        else
        {
            std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> unavailable"
                      << std::endl;
        }
        printSearchResult(result);
    };
    //! [create monitoring callback]

    // only one callback allowed, hence we require no handles to deregister
    // the callback later

    //! [register callback]
    discovery.registerCallback(callback);
    //! [register callback]

    while (!iox::hasTerminationRequested())
    {
        // here the app would run its functional code while the
        // service availability is monitored in the background

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    //! [deregister callback]
    discovery.deregisterCallback();
    //! [deregister callback]

    return (EXIT_SUCCESS);
}
