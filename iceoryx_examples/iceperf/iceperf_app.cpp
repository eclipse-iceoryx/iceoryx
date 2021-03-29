// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceperf_app.hpp"
#include "iceoryx.hpp"
#include "iceoryx_c.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "mq.hpp"
#include "topic_data.hpp"
#include "uds.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

iox::cxx::optional<IcePerfApp> IcePerfApp::create(const PerfSettings settings) noexcept
{
    iox::capro::ServiceDescription serviceDescription{"IcePerf", "Settings", "Comedians"};
    if (settings.appType == ApplicationType::LEADER)
    {
        // send setting to follower application

        iox::runtime::PoshRuntime::initRuntime("iceperf-app-hardy");

        iox::popo::PublisherOptions options;
        options.historyCapacity = 1U;
        iox::popo::Publisher<PerfSettings> settingsPublisher{serviceDescription, options};
        if (!settingsPublisher.publishCopyOf(settings))
        {
            std::cerr << "Could not send settings to follower!" << std::endl;
            return iox::cxx::nullopt;
        }

        return IcePerfApp(settings);
    }
    else
    {
        // wait for settings from leader application

        iox::runtime::PoshRuntime::initRuntime("iceperf-app-laurel");

        iox::popo::SubscriberOptions options;
        options.historyRequest = 1U;
        iox::popo::Subscriber<PerfSettings> settingsSubscriber{serviceDescription, options};

        constexpr bool WAIT_FOR_SETTINGS{true};
        while (WAIT_FOR_SETTINGS)
        {
            static bool waitMessagePrinted = false;
            if (!waitMessagePrinted)
            {
                std::cout << "Waiting for PerfSettings from leader application!" << std::endl;
                waitMessagePrinted = true;
            }

            auto sample = settingsSubscriber.take();
            if (sample)
            {
                auto settings = *(sample.value());
                settings.appType = ApplicationType::FOLLOWER;
                return IcePerfApp(settings);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    return iox::cxx::nullopt;
}

IcePerfApp::IcePerfApp(const PerfSettings settings) noexcept
    : m_settings(settings)
{
}

void IcePerfApp::leaderDo(IcePerfBase& ipcTechnology) noexcept
{
    ipcTechnology.initLeader();

    std::vector<double> latencyInMicroSeconds;
    const std::vector<uint32_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    std::cout << "Measurement for: ";
    for (const auto payloadSizeInKB : payloadSizesInKB)
    {
        std::cout << payloadSizeInKB << " kB, " << std::flush;
        auto payloadSizeInBytes = payloadSizeInKB * IcePerfBase::ONE_KILOBYTE;

        ipcTechnology.preLatencyPerfTestLeader(payloadSizeInBytes);

        auto latency = ipcTechnology.latencyPerfTestLeader(m_settings.numberOfSamples);

        latencyInMicroSeconds.push_back(latency);

        ipcTechnology.postLatencyPerfTestLeader();
    }
    std::cout << std::endl;

    ipcTechnology.releaseFollower();

    ipcTechnology.shutdown();

    std::cout << std::endl;
    std::cout << "#### Measurement Result ####" << std::endl;
    std::cout << m_settings.numberOfSamples << " round trips for each payload." << std::endl;
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

void IcePerfApp::followerDo(IcePerfBase& ipcTechnology) noexcept
{
    ipcTechnology.initFollower();

    ipcTechnology.latencyPerfTestFollower();

    ipcTechnology.shutdown();
}

void IcePerfApp::doIt(IcePerfBase& ipcTechnology) noexcept
{
    if (m_settings.appType == ApplicationType::LEADER)
    {
        leaderDo(ipcTechnology);
    }
    else
    {
        followerDo(ipcTechnology);
    }
}


void IcePerfApp::run() noexcept
{
    iox::capro::IdString_t leaderName{"Hardy"};
    iox::capro::IdString_t followerName{"Laurel"};

    if (m_settings.appType == ApplicationType::LEADER)
    {
        run(leaderName, followerName);
    }
    else
    {
        run(followerName, leaderName);
    }
}

void IcePerfApp::run(const iox::capro::IdString_t publisherName, const iox::capro::IdString_t subscriberName) noexcept
{
    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::POSIX_MESSAGE_QUEUE)
    {
#ifndef __APPLE__
        std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
        MQ mq("/" + std::string(publisherName), "/" + std::string(subscriberName));
        doIt(mq);
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
        UDS uds("/tmp/" + std::string(publisherName), "/tmp/" + std::string(subscriberName));
        doIt(uds);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_CPP_API)
    {
        std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
        Iceoryx iceoryx(publisherName, subscriberName);
        doIt(iceoryx);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_C_API)
    {
        std::cout << std::endl << "******   ICEORYX C API    ********" << std::endl;
        IceoryxC iceoryxc(publisherName, subscriberName);
        doIt(iceoryxc);
    }
}
