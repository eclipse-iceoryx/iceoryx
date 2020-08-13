// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

constexpr int64_t NUMBER_OF_ROUNDTRIPS{100}; // 1000000};
constexpr char APP_NAME[] = "/laurel";
constexpr char PUBLISHER[] = "Laurel";
constexpr char SUBSCRIBER[] = "Hardy";


void leaderDo(IcePerfBase& ipcTechnology)
{
    ipcTechnology.init();

    std::vector<double> latencyInMicroSeconds;
    const std::vector<uint32_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    for (const auto payloadSizeInKB : payloadSizesInKB)
    {
        std::cout << "Measurement for " << payloadSizeInKB << " kB payload ... " << std::flush;
        auto payloadSizeInBytes = payloadSizeInKB * 1024;

        ipcTechnology.prePingPongLeader(payloadSizeInBytes);

        auto latency = ipcTechnology.pingPongLeader(NUMBER_OF_ROUNDTRIPS);

        latencyInMicroSeconds.push_back(latency);

        ipcTechnology.postPingPongLeader();
    }

    ipcTechnology.triggerEnd();

    ipcTechnology.shutdown();

    std::cout << std::endl;
    std::cout << "#### Measurement Result ####" << std::endl;
    std::cout << NUMBER_OF_ROUNDTRIPS << " round trips for each payload." << std::endl;
    std::cout << std::endl;
    std::cout << "| Payload Size [kB] | Average Latency [µs] |" << std::endl;
    std::cout << "|------------------:|---------------------:|" << std::endl;
    for (size_t i = 0; i < latencyInMicroSeconds.size(); ++i)
    {
        std::cout << "| " << std::setw(17) << payloadSizesInKB.at(i) << " | " << std::setw(20) << std::setprecision(2)
                  << latencyInMicroSeconds.at(i) << " |" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Finished!" << std::endl;
}

int main()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance(APP_NAME);

    Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);

    leaderDo(iceoryx);

    return (EXIT_SUCCESS);
}
