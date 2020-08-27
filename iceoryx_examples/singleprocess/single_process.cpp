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
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
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
    iox::popo::Publisher publisher({"Single", "Process", "Demo"});
    publisher.offer();

    uint64_t counter{0};
    while (keepRunning.load())
    {
        auto sample = static_cast<TransmissionData_t*>(publisher.allocateChunk(sizeof(TransmissionData_t)));
        sample->counter = counter++;
        consoleOutput(std::string("Sending: " + std::to_string(sample->counter)));
        publisher.sendChunk(sample);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void receiver()
{
    iox::popo::Subscriber subscriber({"Single", "Process", "Demo"});

    uint64_t cacheQueueSize = 10;
    subscriber.subscribe(cacheQueueSize);

    while (keepRunning.load())
    {
        if (iox::popo::SubscriptionState::SUBSCRIBED == subscriber.getSubscriptionState())
        {
            const void* rawSample = nullptr;
            while (subscriber.getChunk(&rawSample))
            {
                auto sample = static_cast<const TransmissionData_t*>(rawSample);
                consoleOutput(std::string("Receiving : " + std::to_string(sample->counter)));
                subscriber.releaseChunk(rawSample);
            }
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

    iox::roudi::RouDi roudi(
        roudiComponents.m_rouDiMemoryManager, roudiComponents.m_portManager, iox::roudi::MonitoringMode::OFF, false);

    // create a single process runtime for inter thread communication
    iox::runtime::PoshRuntimeSingleProcess runtime("/singleProcessDemo");

    std::thread receiverThread(receiver), senderThread(sender);

    // communicate for 2 seconds and then stop the example
    std::this_thread::sleep_for(std::chrono::seconds(2));
    keepRunning.store(false);

    senderThread.join();
    receiverThread.join();
}
