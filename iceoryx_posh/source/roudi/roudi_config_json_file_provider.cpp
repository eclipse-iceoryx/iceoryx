// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_config_json_file_provider.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/file_reader/file_reader.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"

#include "iceoryx_hoofs/platform/getopt.hpp"
#include <string>

namespace iox
{
namespace roudi
{
JsonRouDiConfigFileProvider::JsonRouDiConfigFileProvider(iox::config::CmdLineArgs_t& cmdLineArgs)
{
    /// don't print additional output if not running
    if (cmdLineArgs.run)
    {
        if (cmdLineArgs.configFilePath.empty())
        {
            /// @todo Replace with C++17 std::filesystem::exists()
            cxx::FileReader configFile(defaultConfigJsonPath, "", cxx::FileReader::ErrorMode::Ignore);

            if (configFile.IsOpen())
            {
                LogWarn() << "No config file provided. Using '" << defaultConfigJsonPath << "'";
                m_customConfigFilePath = defaultConfigJsonPath;
            }
            else
            {
                LogWarn() << "No config file provided and also not found at '" << defaultConfigJsonPath
                          << "'. Falling back to built-in config.";
            }
        }
        else
        {
            m_customConfigFilePath = cmdLineArgs.configFilePath;
        }
    }
}

iox::cxx::expected<iox::mepoo::MePooConfig, iox::roudi::RouDiConfigFileParseError>
JsonRouDiConfigFileProvider::getMempool(json_t const* segment)
{
    iox::mepoo::MePooConfig mempoolConfig;
    json_t const* mempools = json_getProperty(segment, "mempool");
    if (!(mempools && JSON_ARRAY == json_getType(mempools)))
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL);
    }
    json_t const* mempool;
    uint32_t poolCount = 0;
    for (mempool = json_getChild(mempools); mempool != 0; mempool = json_getSibling(mempool))
    {
        if (JSON_OBJ == json_getType(mempool) && poolCount < iox::MAX_NUMBER_OF_MEMPOOLS)
        {
            json_t const* chunkSize = json_getProperty(mempool, "size");
            if (!(chunkSize && JSON_INTEGER == json_getType(chunkSize)))
            {
                return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                    iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE);
            }
            json_t const* chunkCount = json_getProperty(mempool, "count");
            if (!(chunkCount && JSON_INTEGER == json_getType(chunkCount)))
            {
                return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                    iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT);
            }
            uint32_t size = static_cast<uint32_t>(json_getInteger(chunkSize));
            uint32_t count = static_cast<uint32_t>(json_getInteger(chunkCount));
            mempoolConfig.addMemPool({size, count});
        }
        poolCount++;
    }
    if (poolCount > iox::MAX_NUMBER_OF_MEMPOOLS)
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED);
    }
    return iox::cxx::success<iox::mepoo::MePooConfig>(mempoolConfig);
}

iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError> JsonRouDiConfigFileProvider::parse()
{
    json_t mem[NUMBER_OF_JSON_NODES];
    std::string content, line;

    // Early exit in case no config file path was provided
    if (m_customConfigFilePath.empty())
    {
        iox::RouDiConfig_t defaultConfig;
        defaultConfig.setDefaults();
        return iox::cxx::success<iox::RouDiConfig_t>(defaultConfig);
    }

    iox::cxx::FileReader reader(m_customConfigFilePath, "", iox::cxx::FileReader::ErrorMode::Inform);
    while (reader.ReadLine(line))
    {
        content.append(line);
    }
    json_t const* json = json_create(&content[0], mem, NUMBER_OF_JSON_NODES);
    auto groupOfCurrentProcess = iox::posix::PosixGroup::getGroupOfCurrentProcess().getName();

    json_t const* general;
    general = json_getProperty(json, "general");
    if (!(general && JSON_OBJ == json_getType(general)))
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION);
    }
    json_t const* version = json_getProperty(general, "version");
    if (!(version && JSON_INTEGER == json_getType(version) && json_getInteger(version) == 1))
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION);
    }
    json_t const* segments = json_getProperty(json, "segment");
    if (!(segments && JSON_ARRAY == json_getType(segments)))
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS);
    }
    iox::RouDiConfig_t parsedConfig;
    json_t const* segment;
    uint32_t segCount = 0;
    for (segment = json_getChild(segments); segment != 0; segment = json_getSibling(segment))
    {
        if (JSON_OBJ == json_getType(segment) && segCount < iox::MAX_SHM_SEGMENTS)
        {
            char const* writer = json_getPropertyValue(segment, "writer");
            char const* reader = json_getPropertyValue(segment, "reader");
            if (!writer)
            {
                writer = groupOfCurrentProcess.c_str();
            }
            if (!reader)
            {
                reader = groupOfCurrentProcess.c_str();
            }
            iox::cxx::expected<iox::mepoo::MePooConfig, iox::roudi::RouDiConfigFileParseError> meepooConfig =
                getMempool(segment);
            if (meepooConfig.has_error())
            {
                return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(meepooConfig.get_error());
            }
            parsedConfig.m_sharedMemorySegments.push_back(
                {iox::posix::PosixGroup::string_t(iox::cxx::TruncateToCapacity, reader),
                 iox::posix::PosixGroup::string_t(iox::cxx::TruncateToCapacity, writer),
                 meepooConfig.value()});
        }
        segCount++;
    }
    if (segCount > iox::MAX_SHM_SEGMENTS)
    {
        return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
            iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED);
    }
    return iox::cxx::success<iox::RouDiConfig_t>(parsedConfig);
}


} // namespace roudi
} // namespace iox
