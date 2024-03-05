// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"
#include "iceoryx_platform/getopt.hpp"
#include "iox/file_reader.hpp"
#include "iox/into.hpp"
#include "iox/logging.hpp"
#include "iox/posix_group.hpp"
#include "iox/std_string_support.hpp"
#include "iox/string.hpp"
#include "iox/vector.hpp"

#include <cpptoml.h>
#include <fstream>
#include <limits> // workaround for missing include in cpptoml.h
#include <string>

namespace iox
{
namespace config
{
TomlRouDiConfigFileProvider::TomlRouDiConfigFileProvider(config::CmdLineArgs_t& cmdLineArgs) noexcept
{
    /// don't print additional output if not running
    if (cmdLineArgs.run)
    {
        m_roudiConfig = cmdLineArgs.roudiConfig;
        if (cmdLineArgs.configFilePath.empty())
        {
            FileReader configFile(defaultConfigFilePath, "", FileReader::ErrorMode::Ignore);
            if (configFile.isOpen())
            {
                IOX_LOG(INFO, "No config file provided. Using '" << defaultConfigFilePath << "'");
                m_customConfigFilePath = defaultConfigFilePath;
            }
            else
            {
                IOX_LOG(INFO,
                        "No config file provided and also not found at '" << defaultConfigFilePath
                                                                          << "'. Falling back to built-in config.");
            }
        }
        else
        {
            m_customConfigFilePath = cmdLineArgs.configFilePath;
        }
    }
}

iox::expected<iox::IceoryxConfig, iox::roudi::RouDiConfigFileParseError> TomlRouDiConfigFileProvider::parse() noexcept
{
    // Early exit in case no config file path was provided
    if (m_customConfigFilePath.empty())
    {
        iox::IceoryxConfig defaultConfig;
        defaultConfig.setDefaults();
        static_cast<RouDiConfig&>(defaultConfig) = m_roudiConfig;
        return iox::ok(defaultConfig);
    }

    std::ifstream fileStream{m_customConfigFilePath.c_str()};
    if (!fileStream.is_open())
    {
        IOX_LOG(ERROR, "Could not open config file from path '" << m_customConfigFilePath << "'");
        return iox::err(iox::roudi::RouDiConfigFileParseError::FILE_OPEN_FAILED);
    }

    return TomlRouDiConfigFileProvider::parse(fileStream).and_then([this](auto& config) {
        static_cast<RouDiConfig&>(config) = m_roudiConfig;
    });
}


iox::expected<iox::IceoryxConfig, iox::roudi::RouDiConfigFileParseError>
TomlRouDiConfigFileProvider::parse(std::istream& stream) noexcept
{
    std::shared_ptr<cpptoml::table> parsedFile{nullptr};
    try
    {
        cpptoml::parser p{stream};
        parsedFile = p.parse();
    }
    catch (const std::exception& parserException)
    {
        auto parserError = iox::roudi::RouDiConfigFileParseError::EXCEPTION_IN_PARSER;
        auto errorStringIndex = static_cast<uint64_t>(parserError);
        IOX_LOG(WARN,
                iox::roudi::ROUDI_CONFIG_FILE_PARSE_ERROR_STRINGS[errorStringIndex] << ": " << parserException.what());

        return iox::err(parserError);
    }

    auto general = parsedFile->get_table("general");
    if (!general)
    {
        return iox::err(iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION);
    }
    auto configFileVersion = general->get_as<uint32_t>("version");
    if (!configFileVersion || *configFileVersion != 1)
    {
        return iox::err(iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION);
    }

    auto segments = parsedFile->get_table_array("segment");
    if (!segments)
    {
        return iox::err(iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS);
    }

    if (segments->get().size() > iox::MAX_SHM_SEGMENTS)
    {
        return iox::err(iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED);
    }

    auto groupOfCurrentProcess = PosixGroup::getGroupOfCurrentProcess().getName();
    iox::IceoryxConfig parsedConfig;
    for (auto segment : *segments)
    {
        auto writer = segment->get_as<std::string>("writer").value_or(into<std::string>(groupOfCurrentProcess));
        auto reader = segment->get_as<std::string>("reader").value_or(into<std::string>(groupOfCurrentProcess));
        iox::mepoo::MePooConfig mempoolConfig;
        auto mempools = segment->get_table_array("mempool");
        if (!mempools)
        {
            return iox::err(iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL);
        }

        if (mempools->get().size() > iox::MAX_NUMBER_OF_MEMPOOLS)
        {
            return iox::err(iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED);
        }

        for (auto mempool : *mempools)
        {
            auto chunkSize = mempool->get_as<uint64_t>("size");
            auto chunkCount = mempool->get_as<uint32_t>("count");
            if (!chunkSize)
            {
                return iox::err(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE);
            }
            if (!chunkCount)
            {
                return iox::err(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT);
            }
            mempoolConfig.addMemPool({*chunkSize, *chunkCount});
        }
        parsedConfig.m_sharedMemorySegments.push_back(
            {PosixGroup::groupName_t(iox::TruncateToCapacity, reader.c_str(), reader.size()),
             PosixGroup::groupName_t(iox::TruncateToCapacity, writer.c_str(), writer.size()),
             mempoolConfig});
    }

    return iox::ok(parsedConfig);
}
} // namespace config
} // namespace iox
