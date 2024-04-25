// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_dds/gateway/dds_to_iox.hpp"
#include "iceoryx_dds/gateway/iox_to_dds.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/platform/signal.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

class ShutdownManager
{
  public:
    static void scheduleShutdown(int)
    {
        s_semaphore.post().or_else([](auto) {
            std::cerr << "failed to call post on shutdown semaphore" << std::endl;
            std::terminate();
        });
    }
    static void waitUntilShutdown()
    {
        s_semaphore.wait().or_else([](auto) {
            std::cerr << "failed to call wait on shutdown semaphore" << std::endl;
            std::terminate();
        });
    }

  private:
    static iox::posix::Semaphore s_semaphore;
    ShutdownManager() = default;
};
iox::posix::Semaphore ShutdownManager::s_semaphore =
    iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0u).value();

int main()
{
    // Set OS signal handlers
    auto signalGuardInt = iox::posix::registerSignalHandler(iox::posix::Signal::INT, ShutdownManager::scheduleShutdown);
    auto signalGuardTerm =
        iox::posix::registerSignalHandler(iox::posix::Signal::TERM, ShutdownManager::scheduleShutdown);

    // Start application
    iox::runtime::PoshRuntime::initRuntime("iox-dds-gateway");

    iox::config::GatewayConfig gatewayConfig;
    iox::dds::Iceoryx2DDSGateway<> iox2ddsGateway;
    iox::dds::DDS2IceoryxGateway<> dds2ioxGateway;

    iox::config::TomlGatewayConfigParser::parse()
        .and_then([&](auto config) { gatewayConfig = config; })
        .or_else([&](auto err) {
            iox::dds::LogWarn() << "[Main] Failed to parse gateway config with error: "
                                << iox::config::TOML_GATEWAY_CONFIG_FILE_PARSE_ERROR_STRINGS[err];
            iox::dds::LogWarn() << "[Main] Using default configuration.";
            gatewayConfig.setDefaults();
        });

    iox2ddsGateway.loadConfiguration(gatewayConfig);
    dds2ioxGateway.loadConfiguration(gatewayConfig);

    iox2ddsGateway.runMultithreaded();
    dds2ioxGateway.runMultithreaded();

    // Run until SIGINT or SIGTERM
    ShutdownManager::waitUntilShutdown();

    return 0;
}
