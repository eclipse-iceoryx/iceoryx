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

#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"
#include "iox/std_string_support.hpp"

#include "test.hpp"

#include <filesystem>

#include <fstream>
#include <string>

namespace
{
using namespace ::testing;

using ParseErrorInputFile_t = std::pair<iox::roudi::RouDiConfigFileParseError, std::string>;

class RoudiConfigTomlFileProvider_test : public TestWithParam<ParseErrorInputFile_t>
{
  public:
    iox::config::CmdLineArgs_t cmdLineArgs;
};

TEST_F(RoudiConfigTomlFileProvider_test, ParseDefaultConfigIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "37a5ad49-1af2-4e4e-a1ba-85826589d560");
    iox::roudi::ConfigFilePathString_t emptyConfigFilePath;
    cmdLineArgs.configFilePath = emptyConfigFilePath;

    iox::config::TomlRouDiConfigFileProvider sut(cmdLineArgs);

    auto result = sut.parse();

    EXPECT_FALSE(result.has_error());
}

TEST_F(RoudiConfigTomlFileProvider_test, InvalidPathResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca2cc3bd-bf39-451c-9dc6-ae90ec0b8ab7");
    iox::roudi::ConfigFilePathString_t invalidConfigFilePath{"/nowhere/to/find/config.toml"};
    cmdLineArgs.configFilePath = invalidConfigFilePath;

    iox::config::TomlRouDiConfigFileProvider sut(cmdLineArgs);

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

    cmdLineArgs.configFilePath =
        iox::roudi::ConfigFilePathString_t(iox::TruncateToCapacity, tempFilePath.u8string().c_str());

    iox::config::TomlRouDiConfigFileProvider sut(cmdLineArgs);

    sut.parse().and_then([](const auto&) { GTEST_SUCCEED() << "We got a config!"; }).or_else([](const auto& error) {
        GTEST_FAIL() << "Expected a config but got error: "
                     << iox::roudi::ROUDI_CONFIG_FILE_PARSE_ERROR_STRINGS[static_cast<uint64_t>(error)];
    });
}

constexpr const char* CONFIG_NO_GENERAL_SECTION = R"(
    [[segment]]

    [[segment.mempool]]
    size = 128
    count = 10000
)";

constexpr const char* CONFIG_INVALID_CONFIG_FILE_VERSION = R"(
    [general]
    version = 0

    [[segment]]

    [[segment.mempool]]
    size = 128
    count = 10000
)";

constexpr const char* CONFIG_NO_SEGMENTS = R"(
    [general]
    version = 1
)";

const std::string CONFIG_MAX_NUMBER_OF_SEGMENTS_EXCEEDED = [] {
    std::string config = R"(
    [general]
    version = 1

)";
    for (uint64_t i = 0; i <= iox::MAX_SHM_SEGMENTS + 1; ++i)
    {
        config.append("[[segment]]\n");
    }

    return config;
}();

constexpr const char* CONFIG_SEGMENT_WITHOUT_MEMPOOL = R"(
    [general]
    version = 1

    [[segment]]
)";

const std::string CONFIG_MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED = [] {
    std::string config = R"(
        [general]
        version = 1

        [[segment]]

    )";
    for (uint64_t i = 0; i <= iox::MAX_NUMBER_OF_MEMPOOLS + 1; ++i)
    {
        config.append("[[segment.mempool]]\n");
        config.append("size = 128\n");
        config.append("count = 1\n");
    }

    return config;
}();

constexpr const char* CONFIG_MEMPOOL_WITHOUT_CHUNK_SIZE = R"(
    [general]
    version = 1

    [[segment]]

    [[segment.mempool]]
    count = 10000
)";

constexpr const char* CONFIG_MEMPOOL_WITHOUT_CHUNK_COUNT = R"(
    [general]
    version = 1

    [[segment]]

    [[segment.mempool]]
    size = 128
)";

constexpr const char* CONFIG_EXCEPTION_IN_PARSER = R"(🐔)";

INSTANTIATE_TEST_SUITE_P(
    ParseAllMalformedInputConfigFiles,
    RoudiConfigTomlFileProvider_test,
    Values(ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION, CONFIG_NO_GENERAL_SECTION},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION,
                                 CONFIG_INVALID_CONFIG_FILE_VERSION},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS, CONFIG_NO_SEGMENTS},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED,
                                 CONFIG_MAX_NUMBER_OF_SEGMENTS_EXCEEDED},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL,
                                 CONFIG_SEGMENT_WITHOUT_MEMPOOL},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED,
                                 CONFIG_MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE,
                                 CONFIG_MEMPOOL_WITHOUT_CHUNK_SIZE},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT,
                                 CONFIG_MEMPOOL_WITHOUT_CHUNK_COUNT},
           ParseErrorInputFile_t{iox::roudi::RouDiConfigFileParseError::EXCEPTION_IN_PARSER,
                                 CONFIG_EXCEPTION_IN_PARSER}));


TEST_P(RoudiConfigTomlFileProvider_test, ParseMalformedInputFileCausesError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a49e2732-df35-4e4d-b312-bb8b9b9fef52");
    const auto params = GetParam();
    const auto expectedErrorCode = params.first;
    const auto& serializedConfig = params.second;

    std::istringstream stream(serializedConfig);
    auto result = iox::config::TomlRouDiConfigFileProvider::parse(stream);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(expectedErrorCode, result.error());
}

} // namespace
