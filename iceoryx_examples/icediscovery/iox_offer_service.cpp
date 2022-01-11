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
#include "iceoryx_posh/popo/publisher.hpp"
//! [include servicediscovery]
#include "iceoryx_posh/runtime/service_discovery.hpp"
//! [include servicediscovery]
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <iostream>

bool killswitch = false;
constexpr char APP_NAME[] = "iox-cpp-offer";

static void sigHandler(int sig IOX_MAYBE_UNUSED)
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // Register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::runtime::ServiceDiscovery serviceDiscovery;

    //! [offer by creating publisher]
    iox::capro::ServiceDescription radarServiceFrontLeft{"Radar", "FrontLeft", "SequenceCounter"};
    iox::popo::Publisher<uint32_t> publisher(radarServiceFrontLeft);
    //! [offer by creating publisher]
    std::cout << "Created publisher with: " << radarServiceFrontLeft << std::endl;


    //! [direct offer]
    iox::capro::ServiceDescription radarServiceFrontRight{"Radar", "FrontRight", "SequenceCounter"};
    serviceDiscovery.offerService(radarServiceFrontRight);
    //! [direct offer]
    std::cout << "Offered service: " << radarServiceFrontRight << std::endl;

    std::cout << "Once Ctrl-C is pressed, the publisher 'radarServiceFrontLeft' will go out of scope and call "
                 "stopOffer() its related service"
              << std::endl;

    while (!killswitch)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
