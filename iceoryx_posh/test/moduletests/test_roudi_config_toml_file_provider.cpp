// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_dust/cxx/std_string_support.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"

#include "iceoryx/tests/posh/moduletests/test_input_path.hpp"
#include "test.hpp"

#if __cplusplus >= 201703L
#include <filesystem>
#endif

#include <fstream>
#include <string>

namespace
{
using namespace ::testing;

using ParseErrorInputFile_t = std::pair<iox::roudi::RouDiConfigFileParseError, iox::roudi::ConfigFilePathString_t>;

class RoudiConfigTomlFileProvider_test : public TestWithParam<ParseErrorInputFile_t>
{
  public:
    void SetUp()
    {
        // get file path via cmake
        m_cmdLineArgs.configFilePath = iox::testing::TEST_INPUT_PATH;
    }

    void TearDown()
    {
    }

    iox::config::CmdLineArgs_t m_cmdLineArgs;
};

TEST_F(RoudiConfigTomlFileProvider_test, ParseDefaultConfigIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "37a5ad49-1af2-4e4e-a1ba-85826589d560");
    iox::roudi::ConfigFilePathString_t emptyConfigFilePath;
    m_cmdLineArgs.configFilePath = emptyConfigFilePath;

    iox::config::TomlRouDiConfigFileProvider sut(m_cmdLineArgs);

    auto result = sut.parse();

    EXPECT_FALSE(result.has_error());
}

TEST_F(RoudiConfigTomlFileProvider_test, InvalidPathResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca2cc3bd-bf39-451c-9dc6-ae90ec0b8ab7");
    iox::roudi::ConfigFilePathString_t invalidConfigFilePath{"/nowhere/to/find/config.toml"};
    m_cmdLineArgs.configFilePath = invalidConfigFilePath;

    iox::config::TomlRouDiConfigFileProvider sut(m_cmdLineArgs);

    sut.parse()
        .and_then([](const auto&) {
            GTEST_FAIL() << "Expected 'RouDiConfigFileParseError::FILE_OPEN_FAILED' but got a config!";
        })
        .or_else(
            [](const auto& error) { EXPECT_THAT(error, Eq(iox::roudi::RouDiConfigFileParseError::FILE_OPEN_FAILED)); });
}

TEST_F(RoudiConfigTomlFileProvider_test, ParsingFileIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "37f5a397-a289-4bc1-86a7-d95851a5ab47");

#if __cplusplus < 201703L
    GTEST_SKIP() << "The test uses std::filesystem which is only available with C++17";
#else
    auto tempFilePath = std::filesystem::temp_directory_path();
    tempFilePath.append("test_roudi_config.toml");

    std::fstream tempFile{tempFilePath, std::ios_base::trunc | std::ios_base::out};
    ASSERT_TRUE(tempFile.is_open());
    tempFile << R"([general]
version = 1

[[segment]]

[[segment.mempool]]
size = 128
count = 1
)";
    tempFile.close();

    m_cmdLineArgs.configFilePath = iox::roudi::ConfigFilePathString_t(iox::TruncateToCapacity, tempFilePath.c_str());

    iox::config::TomlRouDiConfigFileProvider sut(m_cmdLineArgs);

    sut.parse().and_then([](const auto&) { GTEST_SUCCEED() << "We got a config!"; }).or_else([](const auto& error) {
        GTEST_FAIL() << "Expected a config but got error: "
                     << iox::roudi::ROUDI_CONFIG_FILE_PARSE_ERROR_STRINGS[static_cast<uint64_t>(error)];
    });
#endif
}

INSTANTIATE_TEST_SUITE_P(
    ParseAllMalformedInputConfigFiles,
    RoudiConfigTomlFileProvider_test,
    Values(ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION,
                                 "roudi_config_error_no_general.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION,
                                 "roudi_config_error_invalid_version.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS,
                                 "roudi_config_error_no_segments.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED,
                                 "roudi_config_error_max_segments_exceeded.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL,
                                 "roudi_config_error_segment_without_mempool.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED,
                                 "roudi_config_error_max_mempools_exceeded.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE,
                                 "roudi_config_error_mempool_without_chunk_size.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT,
                                 "roudi_config_error_mempool_without_chunk_count.toml"},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::EXCEPTION_IN_PARSER,
                                 "toml_parser_exception.toml"}));


TEST_P(RoudiConfigTomlFileProvider_test, ParseMalformedInputFileCausesError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a49e2732-df35-4e4d-b312-bb8b9b9fef52");
    const auto parseErrorInputFile = GetParam();

    m_cmdLineArgs.configFilePath.append(iox::TruncateToCapacity, parseErrorInputFile.second);

    iox::config::TomlRouDiConfigFileProvider sut(m_cmdLineArgs);

    auto result = sut.parse();

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(parseErrorInputFile.first, result.get_error());
}

} // namespace
