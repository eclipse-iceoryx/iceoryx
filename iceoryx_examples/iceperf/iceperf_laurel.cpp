// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "example_common.hpp"
#include "iceoryx.hpp"
#include "iceoryx_c.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "mq.hpp"
#include "topic_data.hpp"
#include "uds.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

constexpr int64_t NUMBER_OF_ROUNDTRIPS{10000};
constexpr char APP_NAME[] = "laurel";
constexpr char PUBLISHER[] = "Laurel";
constexpr char SUBSCRIBER[] = "Hardy";


void leaderDo(IcePerfBase& ipcTechnology, uint64_t numRoundtrips)
{
    ipcTechnology.initLeader();

    std::vector<double> latencyInMicroSeconds;
    const std::vector<uint32_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    std::cout << "Measurement for: ";
    for (const auto payloadSizeInKB : payloadSizesInKB)
    {
        std::cout << payloadSizeInKB << " kB, " << std::flush;
        auto payloadSizeInBytes = payloadSizeInKB * IcePerfBase::ONE_KILOBYTE;

        ipcTechnology.prePingPongLeader(payloadSizeInBytes);

        auto latency = ipcTechnology.pingPongLeader(numRoundtrips);

        latencyInMicroSeconds.push_back(latency);

        ipcTechnology.postPingPongLeader();
    }
    std::cout << std::endl;

    ipcTechnology.releaseFollower();

    ipcTechnology.shutdown();

    std::cout << std::endl;
    std::cout << "#### Measurement Result ####" << std::endl;
    std::cout << numRoundtrips << " round trips for each payload." << std::endl;
    std::cout << std::endl;
    std::cout << "| Payload Size [kB] | Average Latency [µs] |" << std::endl;
    std::cout << "|------------------:|---------------------:|" << std::endl;
    for (size_t i = 0U; i < latencyInMicroSeconds.size(); ++i)
    {
        std::cout << "| " << std::setw(17) << payloadSizesInKB.at(i) << " | " << std::setw(20) << std::setprecision(2)
                  << latencyInMicroSeconds.at(i) << " |" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Finished!" << std::endl;
}

int main(int argc, char* argv[])
{
    Benchmarks benchmark = Benchmarks::ALL;
    uint64_t numRoundtrips = NUMBER_OF_ROUNDTRIPS;
    if (argc > 1)
    {
        if (!iox::cxx::convert::fromString(argv[1], numRoundtrips))
        {
            std::cout << "first parameter must be the number of roundtrips" << std::endl;
            exit(1);
        }
    }
    if (argc > 2)
    {
        benchmark = getBenchmarkFromString(argv[2]);
    }

    if (benchmark == Benchmarks::ALL)
    {
#ifndef __APPLE__
        std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
        MQ mq("/" + std::string(PUBLISHER), "/" + std::string(SUBSCRIBER));
        leaderDo(mq, numRoundtrips);
#endif

        std::cout << std::endl << "****** UNIX DOMAIN SOCKET ********" << std::endl;
        UDS uds("/tmp/" + std::string(PUBLISHER), "/tmp/" + std::string(SUBSCRIBER));
        leaderDo(uds, numRoundtrips);
    }

    iox::runtime::PoshRuntime::initRuntime(APP_NAME); // runtime for registering with the RouDi daemon
    if (benchmark == Benchmarks::ALL || benchmark == Benchmarks::CPP_API)
    {
        std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
        Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);
        leaderDo(iceoryx, numRoundtrips);
    }

    if (benchmark == Benchmarks::ALL || benchmark == Benchmarks::C_API)
    {
        std::cout << std::endl << "******   ICEORYX C API    ********" << std::endl;
        IceoryxC iceoryxc(PUBLISHER, SUBSCRIBER);
        leaderDo(iceoryxc, numRoundtrips);
    }


    return (EXIT_SUCCESS);
}
