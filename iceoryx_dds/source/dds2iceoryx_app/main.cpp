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
#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

#include <chrono>
#include <iostream>
#include <thread>

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

int main()
{
    // Set OS signal handlers
    signal(SIGINT, ShutdownManager::scheduleShutdown);
    signal(SIGTERM, ShutdownManager::scheduleShutdown);

    // Start application
    iox::runtime::PoshRuntime::getInstance("/iox-gw-dds2iceoryx");

    iox::dds::DDS2IceoryxGateway<> gw;

    iox::config::TomlGatewayConfigParser::parse()
        .and_then([&](iox::config::GatewayConfig config) { gw.loadConfiguration(config); })
        .or_else([&](iox::config::TomlGatewayConfigParseError err) {
            iox::dds::LogWarn() << "[Main] Failed to parse gateway config with error: "
                                << iox::config::TomlGatewayConfigParseErrorString[err];
            iox::dds::LogWarn() << "[Main] Using default configuration.";
            iox::config::GatewayConfig defaultConfig;
            defaultConfig.setDefaults();
            gw.loadConfiguration(defaultConfig);
        });

    gw.runMultithreaded();

    // Run until SIGINT or SIGTERM
    ShutdownManager::waitUntilShutdown();

    return 0;
}
