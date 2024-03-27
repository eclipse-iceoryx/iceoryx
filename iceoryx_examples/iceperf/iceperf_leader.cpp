// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iceperf_leader.hpp"
#include "iceoryx.hpp"
#include "iceoryx_c.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_wait.hpp"
#include "iox/detail/convert.hpp"
#include "mq.hpp"
#include "topic_data.hpp"
#include "uds.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

//! [use constants instead of magic values]
constexpr const char APP_NAME[]{"iceperf-bench-leader"};
constexpr const char PUBLISHER[]{"Leader"};
constexpr const char SUBSCRIBER[]{"Follower"};
//! [use constants instead of magic values]

IcePerfLeader::IcePerfLeader(const PerfSettings settings) noexcept
    : m_settings(settings)
{
    //! [cleanup outdated resources]
#ifndef __APPLE__
    MQ::cleanupOutdatedResources(PUBLISHER, SUBSCRIBER);
#endif
    UDS::cleanupOutdatedResources(PUBLISHER, SUBSCRIBER);
    //! [cleanup outdated resources]
}

//! [do the measurement for a single technology]
void IcePerfLeader::doMeasurement(IcePerfBase& ipcTechnology) noexcept
{
    ipcTechnology.initLeader();

    auto humanReadableMemorySize = [](const uint64_t memorySize) {
        constexpr const uint64_t UNIT_DIVIDER{1024};
        auto humanReadalbeMemorySize = memorySize;
        for (const auto& unit : {iox::string<2>("B"),
                                 iox::string<2>("kB"),
                                 iox::string<2>("MB"),
                                 iox::string<2>("GB"),
                                 iox::string<2>("TB")})
        {
            if (humanReadalbeMemorySize >= UNIT_DIVIDER)
            {
                humanReadalbeMemorySize /= UNIT_DIVIDER;
                continue;
            }
            return std::make_tuple(humanReadalbeMemorySize, unit);
        }
        return (std::make_tuple(memorySize, iox::string<2>("B")));
    };

    std::vector<std::tuple<uint32_t, iox::units::Duration>> latencyMeasurements;
    const std::vector<uint32_t> payloadSizes{16,
                                             32,
                                             64,
                                             128,
                                             256,
                                             512,
                                             1 * IcePerfBase::ONE_KILOBYTE,
                                             2 * IcePerfBase::ONE_KILOBYTE,
                                             4 * IcePerfBase::ONE_KILOBYTE,
                                             8 * IcePerfBase::ONE_KILOBYTE,
                                             16 * IcePerfBase::ONE_KILOBYTE,
                                             32 * IcePerfBase::ONE_KILOBYTE,
                                             64 * IcePerfBase::ONE_KILOBYTE,
                                             128 * IcePerfBase::ONE_KILOBYTE,
                                             256 * IcePerfBase::ONE_KILOBYTE,
                                             512 * IcePerfBase::ONE_KILOBYTE,
                                             1024 * IcePerfBase::ONE_KILOBYTE,
                                             2048 * IcePerfBase::ONE_KILOBYTE,
                                             4096 * IcePerfBase::ONE_KILOBYTE};
    std::cout << "Measurement for:";
    const char* separator = " ";
    for (const auto payloadSize : payloadSizes)
    {
        uint64_t humanReadablePayloadSize{0};
        iox::string<2> memorySizeUnit{};
        std::tie(humanReadablePayloadSize, memorySizeUnit) = humanReadableMemorySize(payloadSize);
        std::cout << separator << humanReadablePayloadSize << " [" << memorySizeUnit << "]" << std::flush;
        separator = ", ";

        ipcTechnology.preLatencyPerfTestLeader(payloadSize);

        auto latency = ipcTechnology.latencyPerfTestLeader(m_settings.numberOfSamples);

        latencyMeasurements.push_back(std::make_tuple(payloadSize, latency));

        ipcTechnology.postLatencyPerfTestLeader();
    }
    std::cout << std::endl;

    ipcTechnology.releaseFollower();

    ipcTechnology.shutdown();

    std::cout << std::endl;
    std::cout << "#### Measurement Result ####" << std::endl;
    std::cout << m_settings.numberOfSamples << " round trips for each payload." << std::endl;
    std::cout << std::endl;
    std::cout << "| Payload Size | Average Latency [µs] |" << std::endl;
    std::cout << "|-------------:|---------------------:|" << std::endl;
    for (const auto& latencyMeasuement : latencyMeasurements)
    {
        uint64_t humanReadablePayloadSize{0};
        iox::string<2> memorySizeUnit{};
        std::tie(humanReadablePayloadSize, memorySizeUnit) = humanReadableMemorySize(std::get<0>(latencyMeasuement));
        auto latencyInMicroseconds = static_cast<double>(std::get<1>(latencyMeasuement).toNanoseconds()) / 1000.0;
        iox::string<10> unitString{"["};
        unitString.append(iox::TruncateToCapacity, memorySizeUnit);
        unitString.append(iox::TruncateToCapacity, "]");
        std::cout << "| " << std::setw(7) << humanReadablePayloadSize << " " << std::setw(4) << std::left << unitString
                  << std::right << " | " << std::setw(20) << std::setprecision(2) << latencyInMicroseconds << " |"
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Finished!" << std::endl;
}
//! [do the measurement for a single technology]

//! [run all technologies]
int IcePerfLeader::run() noexcept
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [send setting to follower application]
    iox::capro::ServiceDescription serviceDescription{"IcePerf", "Settings", "Generic"};
    iox::popo::PublisherOptions options;
    options.historyCapacity = 1U;
    iox::popo::Publisher<PerfSettings> settingsPublisher{serviceDescription, options};
    if (!settingsPublisher.publishCopyOf(m_settings))
    {
        std::cerr << "Could not send settings to follower!" << std::endl;
        return EXIT_FAILURE;
    }
    //! [send setting to follower application]

    //! [create an run technologies]
    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::POSIX_MESSAGE_QUEUE)
    {
#ifndef __APPLE__
        std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
        MQ mq(PUBLISHER, SUBSCRIBER);
        doMeasurement(mq);
#else
        if (m_settings.technology == Technology::POSIX_MESSAGE_QUEUE)
        {
            std::cout << "The message queue is not supported on macOS and will be skipped!" << std::endl;
        }
#endif
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::UNIX_DOMAIN_SOCKET)
    {
        std::cout << std::endl << "****** UNIX DOMAIN SOCKET ********" << std::endl;
        UDS uds(PUBLISHER, SUBSCRIBER);
        doMeasurement(uds);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_CPP_API)
    {
        std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
        Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);
        doMeasurement(iceoryx);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_C_API)
    {
        std::cout << std::endl << "******   ICEORYX C API    ********" << std::endl;
        IceoryxC iceoryxc(PUBLISHER, SUBSCRIBER);
        doMeasurement(iceoryxc);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_CPP_WAIT_API)
    {
        std::cout << std::endl << "******   ICEORYX WAITSET  ********" << std::endl;
        IceoryxWait iceoryxwait(PUBLISHER, SUBSCRIBER);
        doMeasurement(iceoryxwait);
    }
    //! [create an run technologies]

    return EXIT_SUCCESS;
}
//! [run all technologies]
