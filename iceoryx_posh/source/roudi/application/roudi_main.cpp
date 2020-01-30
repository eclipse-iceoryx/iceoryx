// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/roudi/roudi_config_file_parser.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"

#ifdef TOML_CONFIG_FILE
#include "cpptoml.h"

class TomlRouDiConfigFileParser : public iox::roudi::RouDiConfigFileParser
{
  public:
    iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError>
    parse(const iox::ConfigFilePathString_t& configFilePath) override
    {
        auto groupOfCurrentProcess = iox::posix::PosixGroup::getGroupOfCurrentProcess().getName();

        std::shared_ptr<cpptoml::table> parsedFile = cpptoml::parse_file(configFilePath.c_str());

        auto general = parsedFile->get_table("general");
        if (!general)
        {
            return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION);
        }
        auto configFileVersion = general->get_as<uint32_t>("version");
        if (!configFileVersion || *configFileVersion != 1)
        {
            return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION);
        }

        auto segments = parsedFile->get_table_array("segment");
        if (!segments)
        {
            return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS);
        }

        if (segments->get().size() > iox::MAX_SHM_SEGMENTS)
        {
            return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED);
        }

        iox::RouDiConfig_t parsedConfig;
        for (auto segment : *segments)
        {
            auto writer = segment->get_as<std::string>("writer").value_or(groupOfCurrentProcess);
            auto reader = segment->get_as<std::string>("reader").value_or(groupOfCurrentProcess);
            iox::mepoo::MePooConfig mempoolConfig;
            auto mempools = segment->get_table_array("mempool");
            if (!mempools)
            {
                return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                    iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL);
            }

            if (mempools->get().size() > iox::MAX_NUMBER_OF_MEMPOOLS)
            {
                return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                    iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED);
            }

            for (auto mempool : *mempools)
            {
                auto chunkSize = mempool->get_as<uint32_t>("size");
                auto chunkCount = mempool->get_as<uint32_t>("count");
                if (!chunkSize)
                {
                    return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                        iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE);
                }
                if (!chunkCount)
                {
                    return iox::cxx::error<iox::roudi::RouDiConfigFileParseError>(
                        iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT);
                }
                mempoolConfig.addMemPool({*chunkSize, *chunkCount});
            }
            parsedConfig.m_sharedMemorySegments.push_back({reader, writer, mempoolConfig});
        }

        return iox::cxx::success<iox::RouDiConfig_t>(parsedConfig);
    }
};
#endif

int main(int argc, char* argv[])
{
    using iox::roudi::IceOryxRouDiApp;

#ifdef TOML_CONFIG_FILE
    TomlRouDiConfigFileParser configFileParser;

    IceOryxRouDiApp roudi(argc, argv, &configFileParser);
#else
    iox::RouDiConfig_t roudiConfig;
    roudiConfig.setDefaults();

    IceOryxRouDiApp roudi(argc, argv, roudiConfig);
#endif

    roudi.run();

    return 0;
}
