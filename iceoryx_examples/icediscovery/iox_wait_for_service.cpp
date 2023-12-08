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
#include "discovery_blocking.hpp"
//! [include custom discovery]

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

using namespace discovery;

constexpr char APP_NAME[] = "iox-wait-for-service";

volatile bool keepRunning{true};

//! [service to wait for]
iox::capro::IdString_t service{"Camera"};
iox::capro::IdString_t instance{"FrontLeft"};
iox::capro::IdString_t event{"Image"};
//! [service to wait for]

void printSearchResult(const discovery::ServiceContainer& result)
{
    std::cout << APP_NAME << " search result: " << (result.empty() ? "empty" : "") << std::endl;

    for (auto entry : result)
    {
        std::cout << APP_NAME << " <" << entry.getServiceIDString() << ", " << entry.getInstanceIDString() << ", "
                  << entry.getEventIDString() << ">" << std::endl;
    }
}

volatile Discovery* discoverySigHandlerAccess{nullptr};

void sigHandler(int)
{
    //! [unblock wait]
    keepRunning = false;
    if (discoverySigHandlerAccess)
    {
        discoverySigHandlerAccess->unblockWait();
    }
    //! [unblock wait]
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create custom discovery]
    // requires the runtime to be created first
    Discovery discovery;
    //! [create custom discovery]

    discoverySigHandlerAccess = &discovery;

    auto sigTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, sigHandler).expect("failed to register SIGTERM");
    auto sigIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, sigHandler).expect("failed to register SIGINT");

    //! [define search query]
    auto query = [&]() {
        auto result = discovery.findService(service, instance, event);
        return !result.empty();
    };
    //! [define search query]

    std::cout << APP_NAME << " waiting for service <" << service << ", " << instance << ", " << event << "> ...\n"
              << std::endl;

    //! [wait until service was available]
    bool serviceWasAvailable = discovery.waitUntil(query);
    //! [wait until service was available]

    // did we wake up due to an unblock or because the service was available?
    if (serviceWasAvailable)
    {
        std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> was available\n"
                  << std::endl;

        // service was available, but we can never be sure the service is still available
        // if this is important we need to monitor it (see discovery monitor example)

        std::cout << APP_NAME << " waiting for any discovery change ...\n" << std::endl;

        do
        {
            //! [wait until discovery changes]
            discovery.waitUntilChange();
            //! [wait until discovery changes]
            std::cout << APP_NAME << " discovery changed. Searching <" << service << ", " << instance << ", " << event
                      << "> ..." << std::endl;

            //! [check service availability]
            if (discovery.findService(service, instance, event).empty())
            {
                break;
            }
            //! [check service availability]

            std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> was available\n"
                      << std::endl;

            // loop while the service is available (e.g. perform some computation etc.)
        } while (keepRunning);

        std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> was unavailable"
                  << std::endl;
    }

    discoverySigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
