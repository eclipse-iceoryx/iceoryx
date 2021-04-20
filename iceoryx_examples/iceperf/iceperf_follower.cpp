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

#include "iceperf_follower.hpp"
#include "iceoryx.hpp"
#include "iceoryx_c.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "mq.hpp"
#include "topic_data.hpp"
#include "uds.hpp"

#include <iostream>

constexpr const char APP_NAME[]{"iceperf-bench-follower"};
constexpr const char PUBLISHER[]{"Laurel"};
constexpr const char SUBSCRIBER[]{"Hardy"};

void IcePerfFollower::doTest(IcePerfBase& ipcTechnology) noexcept
{
    ipcTechnology.initFollower();

    ipcTechnology.latencyPerfTestFollower();

    ipcTechnology.shutdown();
}

PerfSettings IcePerfFollower::getSettings(iox::popo::Subscriber<PerfSettings>& subscriber) noexcept
{
    // wait for settings from leader application
    constexpr bool WAIT_FOR_SETTINGS{true};
    while (WAIT_FOR_SETTINGS)
    {
        static bool waitMessagePrinted{false};
        if (!waitMessagePrinted)
        {
            std::cout << "Waiting for PerfSettings from leader application!" << std::endl;
            waitMessagePrinted = true;
        }

        auto sample = subscriber.take();
        if (sample)
        {
            return *(sample.value());
        }
        else
        {
            constexpr std::chrono::milliseconds POLLING_INTERVAL{100};
            std::this_thread::sleep_for(POLLING_INTERVAL);
        }
    }
}

int IcePerfFollower::run() noexcept
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::capro::ServiceDescription serviceDescription{"IcePerf", "Settings", "Comedians"};
    iox::popo::SubscriberOptions options;
    options.historyRequest = 1U;
    iox::popo::Subscriber<PerfSettings> settingsSubscriber{serviceDescription, options};

    m_settings = getSettings(settingsSubscriber);

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::POSIX_MESSAGE_QUEUE)
    {
#ifndef __APPLE__
        std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
        MQ mq("/" + std::string(PUBLISHER), "/" + std::string(SUBSCRIBER));
        doTest(mq);
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
        UDS uds("/tmp/" + std::string(PUBLISHER), "/tmp/" + std::string(SUBSCRIBER));
        doTest(uds);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_CPP_API)
    {
        std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
        Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);
        doTest(iceoryx);
    }

    if (m_settings.technology == Technology::ALL || m_settings.technology == Technology::ICEORYX_C_API)
    {
        std::cout << std::endl << "******   ICEORYX C API    ********" << std::endl;
        IceoryxC iceoryxc(PUBLISHER, SUBSCRIBER);
        doTest(iceoryxc);
    }

    return EXIT_SUCCESS;
}
