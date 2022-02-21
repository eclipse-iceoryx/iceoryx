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

#include "discovery_blocking.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

using namespace discovery;

constexpr char APP_NAME[] = "iox-wait-for-service";

iox::capro::IdString_t service{"Camera"};
iox::capro::IdString_t instance{"FrontLeft"};
iox::capro::IdString_t event{"Image"};

void printSearchResult(const iox::runtime::ServiceContainer& result)
{
    std::cout << "Search result: " << (result.empty() ? "empty" : "") << std::endl;

    for (auto entry : result)
    {
        std::cout << "<" << entry.getServiceIDString() << ", " << entry.getInstanceIDString() << ", "
                  << entry.getEventIDString() << ">" << std::endl;
    }
}

Discovery* discoveryPtr{nullptr};

void sigHandler(int)
{
    std::cout << "sig handler" << std::endl;
    if (discoveryPtr)
    {
        discoveryPtr->unblockWait();
    }
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // requires the runtime
    Discovery discovery;
    discoveryPtr = &discovery;

    auto sigTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);
    auto sigIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);

    auto query = [&]() {
        auto result = discovery.findService(service, instance, event);
        return result.size() > 0;
    };

    while (!iox::posix::hasTerminationRequested())
    {
        std::cout << "Waiting for service <" << service << ", " << instance << ", " << event << "> ..." << std::endl;
        bool serviceWasAvailable = discovery.waitUntil(query);

        // did we wake up due to an unblock or because the service was available?
        if (serviceWasAvailable)
        {
            std::cout << "<" << service << ", " << instance << ", " << event << "> was available" << std::endl;

            // service was available, but we can never be sure the service is still available
            // if this is important we need to monitor it (see discovery monitor example)

            std::cout << "Waiting for any discovery change ..." << std::endl;
            discovery.waitUntilChange();

            std::cout << "Discovery changed. Searching <" << service << ", " << instance << ", " << event << "> ..."
                      << std::endl;

            // we will usually find nothing here unless offer and stop offering very fast
            auto result = discovery.findService(service, instance, event);
            printSearchResult(result);
            break;
        }
    }


    return (EXIT_SUCCESS);
}
