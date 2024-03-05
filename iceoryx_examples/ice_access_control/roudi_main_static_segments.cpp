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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"
#include "iox/logging.hpp"

int main(int argc, char* argv[])
{
    using iox::roudi::IceOryxRouDiApp;

    iox::config::CmdLineParser cmdLineParser;
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

    //! [config]
    iox::IceoryxConfig config;
    static_cast<iox::config::RouDiConfig&>(config) = cmdLineArgs.value().roudiConfig;


    // Create Mempool Config
    iox::mepoo::MePooConfig mepooConfig;

    // We only send very small data, just one mempool per segment
    mepooConfig.addMemPool({128, 1000});

    // Create an entry for a new shared memory segment from the mempooConfig and add it to the iceoryx config
    // Parameters are {"ReaderGroup", "WriterGroup", MemoryPoolConfig}
    config.m_sharedMemorySegments.push_back({"unprivileged", "privileged", mepooConfig});
    config.m_sharedMemorySegments.push_back({"infotainment", "infotainment", mepooConfig});
    //! [config]

    IceOryxRouDiApp roudi(config);

    return roudi.run();
}
