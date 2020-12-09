// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/popo/modern_api/typed_publisher.hpp"
#include "iceoryx_posh/popo/modern_api/typed_subscriber.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "iceoryx_posh/runtime/posh_runtime_single_process.hpp"
#include "iceoryx_utils/log/logmanager.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

std::atomic_bool keepRunning{true};

struct TransmissionData_t
{
    uint64_t counter;
};

void consoleOutput(const std::string& output)
{
    static std::mutex consoleOutputMutex;

    std::lock_guard<std::mutex> lock(consoleOutputMutex);
    std::cout << output << std::endl;
}

void sender()
{
    iox::popo::TypedPublisher<TransmissionData_t> publisher({"Single", "Process", "Demo"});
    publisher.offer();

    uint64_t counter{0};
    while (keepRunning.load())
    {
        publisher.loan().and_then([&](auto& sample) {
            consoleOutput(std::string("Sending: " + std::to_string(++sample->counter)));
            sample.publish();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void receiver()
{
    iox::popo::TypedSubscriber<TransmissionData_t> subscriber({"Single", "Process", "Demo"});

    uint64_t cacheQueueSize = 10;
    subscriber.subscribe(cacheQueueSize);

    while (keepRunning.load())
    {
        if (iox::SubscribeState::SUBSCRIBED == subscriber.getSubscriptionState())
        {
            bool hasMoreSamples{true};

            do
            {
                subscriber.take()
                    .and_then([&](iox::popo::Sample<const TransmissionData_t>& sample) {
                        consoleOutput(std::string("Receiving : " + std::to_string(sample->counter)));
                    })
                    .if_empty([&] { hasMoreSamples = false; });
            } while (hasMoreSamples);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    // set the log level to error to see the essence of the example
    iox::log::LogManager::GetLogManager().SetDefaultLogLevel(iox::log::LogLevel::kError);

    iox::RouDiConfig_t defaultRouDiConfig = iox::RouDiConfig_t().setDefaults();
    iox::roudi::IceOryxRouDiComponents roudiComponents(defaultRouDiConfig);

    iox::roudi::RouDi roudi(roudiComponents.m_rouDiMemoryManager,
                            roudiComponents.m_portManager,
                            iox::roudi::RouDi::RoudiStartupParameters{iox::config::MonitoringMode::OFF, false});

    // create a single process runtime for inter thread communication
    iox::runtime::PoshRuntimeSingleProcess runtime("/singleProcessDemo");

    std::thread receiverThread(receiver), senderThread(sender);

    // communicate for 2 seconds and then stop the example
    std::this_thread::sleep_for(std::chrono::seconds(2));
    keepRunning.store(false);

    senderThread.join();
    receiverThread.join();
}
