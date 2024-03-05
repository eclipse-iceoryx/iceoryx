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
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iox/logging.hpp"

int main(int argc, char* argv[])
{
    using iox::roudi::IceOryxRouDiApp;
    static constexpr uint32_t ONE_KILOBYTE = 1024U;
    static constexpr uint32_t ONE_MEGABYTE = 1024U * 1024;

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

    iox::IceoryxConfig config;
    // config.setDefaults(); can be used if you want to use the default config only.
    static_cast<iox::config::RouDiConfig&>(config) = cmdLineArgs.value().roudiConfig;

    /// @brief Create Mempool Config
    iox::mepoo::MePooConfig mepooConfig;

    /// @details Format: addMemPool({Chunksize(bytes), Amount of Chunks})
    mepooConfig.addMemPool({128, 10000}); // bytes
    mepooConfig.addMemPool({ONE_KILOBYTE, 5000});
    mepooConfig.addMemPool({ONE_KILOBYTE * 16, 1000});
    mepooConfig.addMemPool({ONE_KILOBYTE * 128, 200});
    mepooConfig.addMemPool({ONE_KILOBYTE * 512, 50});
    mepooConfig.addMemPool({ONE_MEGABYTE, 30});
    mepooConfig.addMemPool({ONE_MEGABYTE * 4, 10});

    /// We want to use the Shared Memory Segment for the current user
    auto currentGroup = iox::PosixGroup::getGroupOfCurrentProcess();

    /// Create an Entry for a new Shared Memory Segment from the MempoolConfig and add it to the IceoryxConfig
    config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mepooConfig});

    /// For the case that you want to give accessrights to the shm segments, you need to set groupnames as fixed string.
    /// These names defines groups whose members are either to read/write from/to the respective shared memory segment.
    /// @note the groups needs to be registered in /etc/groups.
    /// @code
    /// iox::PosixGroup::string_t readerGroup{iox::TruncateToCapacity, "readerGroup"};
    /// iox::PosixGroup::string_t writerGroup{iox::TruncateToCapacity, "writerGroup"};
    /// iox::mepoo::SegmentConfig::SegmentEntry segentry({readerGroup, writerGroup, mepooConfig});
    /// config.m_sharedMemorySegments.push_back(
    /// {iox::PosixGroup::string_t(iox::TruncateToCapacity, reader),
    ///  iox::PosixGroup::string_t(iox::TruncateToCapacity, writer),
    ///  mempoolConfig})
    /// @endcode

    /// configure the chunk count for the introspection; each introspection topic gets this number of chunks
    config.introspectionChunkCount = 10;

    /// configure the chunk count for the service discovery
    config.discoveryChunkCount = 10;

    IceOryxRouDiApp roudi(config);

    return roudi.run();
}
