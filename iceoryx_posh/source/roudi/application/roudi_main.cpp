// Copyright (c) 2019 - 2020 by Robert Bosch GmbH.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"
#include "iox/logging.hpp"

int main(int argc, char* argv[]) noexcept
{
    using iox::roudi::IceOryxRouDiApp;

    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    auto cmdLineArgs = cmdLineParser.parse(argc, argv);
    if (cmdLineArgs.has_error())
    {
        IOX_LOG(FATAL, "Unable to parse command line arguments!");
        return EXIT_FAILURE;
    }

    if (!cmdLineArgs.value().run)
    {
        return EXIT_SUCCESS;
    }

    iox::config::TomlRouDiConfigFileProvider configFileProvider(cmdLineArgs.value());

    auto config = configFileProvider.parse();

    if (config.has_error())
    {
        auto errorStringIndex = static_cast<uint64_t>(config.error());
        IOX_LOG(FATAL,
                "Couldn't parse config file. Error: "
                    << iox::roudi::ROUDI_CONFIG_FILE_PARSE_ERROR_STRINGS[errorStringIndex]);
        return EXIT_FAILURE;
    }

    IceOryxRouDiApp roudi(config.value());
    return roudi.run();
}
