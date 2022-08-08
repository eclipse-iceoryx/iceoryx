// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/platform/signal.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

int main()
{
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
    iox::posix::waitForTerminationRequest();

    return 0;
}
