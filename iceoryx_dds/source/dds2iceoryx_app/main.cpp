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

#include "iceoryx_dds/dds/data_reader.hpp"

#include "iceoryx_dds/gateway/dds_to_iox.hpp"
#include "iceoryx_dds/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

#include <iostream>
#include <thread>
#include <chrono>

class ShutdownManager
{
  public:
    static void scheduleShutdown(int num)
    {
        char reason;
        psignal(num, &reason);
        s_shutdownRequested.store(true, std::memory_order_relaxed);
        s_semaphore.post();
    }
    static void waitUntilShutdown()
    {
        s_semaphore.wait();
    }
    static bool shouldShutdown()
    {
        return s_shutdownRequested.load(std::memory_order_relaxed);
    }
  private:
    static iox::posix::Semaphore s_semaphore;
    static std::atomic_bool s_shutdownRequested;
    ShutdownManager() = default;
};
iox::posix::Semaphore ShutdownManager::s_semaphore = iox::posix::Semaphore::create(0u).get_value();
std::atomic_bool ShutdownManager::s_shutdownRequested{false};

int main(int argc, char* argv[])
{
    // Set OS signal handlers
    signal(SIGINT, ShutdownManager::scheduleShutdown);
    signal(SIGTERM, ShutdownManager::scheduleShutdown);

    // Start application
    iox::runtime::PoshRuntime::getInstance("/gateway_dds2iceoryx");

    // ===== DEBUG ===== //
    iox::dds::CycloneDataReader reader{"Radar", "FrontRight", "Counter"};
    reader.connect();

    while(!ShutdownManager::shouldShutdown())
    {
        uint64_t size = 1024;
        uint8_t buffer[size];

        auto result = reader.read(buffer, size, sizeof(uint64_t));
        if(!result.has_error())
        {
            auto numSamples = static_cast<int>(result.get_value());
            std::cout << "Total samples received: " << numSamples << std::endl;
            for(int i=0; i<numSamples; ++i)
            {
                auto cursor = i * sizeof(uint64_t);
                uint64_t* sample = reinterpret_cast<uint64_t*>(&buffer[i * sizeof(sample)]);
                //std::copy(&buffer[cursor], &buffer[cursor] + sizeof(sample), &sample);
                std::cout << "[" << i << "] " << *sample << std::endl;
            }
        }
        else
        {
            std::cout << "Failure to read from data reader" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    // ===== DEBUG ===== //

//    iox::dds::DDS2IceoryxGateway<> gw;
//    auto result = iox::dds::TomlGatewayConfigParser::parse();
//    if (!result.has_error())
//    {
//        gw.loadConfiguration(result.get_value());
//    }
//    gw.runMultithreaded();

    // Run until SIGINT or SIGTERM
    ShutdownManager::waitUntilShutdown();

    return 0;
}
