// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#if !defined(_WIN32)
#include "test.hpp"

#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iox/string.hpp"

#include <memory>

namespace
{
using namespace ::testing;

using namespace iox::config;

class CmdLineParserConfigFileOption_test : public Test
{
  public:
    void SetUp(){};
    void TearDown()
    {
        // Reset optind to be able to parse again
        optind = 0;
    };
};

TEST_F(CmdLineParserConfigFileOption_test, NoConfigPathOptionLeadsToEmptyPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "5848b80b-7dd6-4d64-8487-cbbec90de2b0");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(""));
}

TEST_F(CmdLineParserConfigFileOption_test, ConfigPathShortOptionIsCorrectlyRead)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3c791ec-0927-458e-9214-1b30ee32aac1");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-c";
    char path[] = "/foo/bar.toml";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &path[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(path));
}

TEST_F(CmdLineParserConfigFileOption_test, ConfigPathLongOptionIsCorrectlyRead)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8231f57-ea8a-4909-8744-a05d066ddb90");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--config-file";
    char path[] = "/foo/bar/baz.toml";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &path[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(path));
}

TEST_F(CmdLineParserConfigFileOption_test, HelpLongOptionLeadsProgrammNotRunning)
{
    ::testing::Test::RecordProperty("TEST_ID", "81d991ce-5591-404a-8731-c6cd13de1841");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--help";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(result.value().run);
}

TEST_F(CmdLineParserConfigFileOption_test, WrongOptionLeadsUnkownOptionResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "783406f2-26f3-4041-9b1f-3845dfb37ca8");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char unknownOption[] = "--unknown";
    args[0] = &appName[0];
    args[1] = &unknownOption[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), CmdLineParserResult::UNKNOWN_OPTION_USED);
}

TEST_F(CmdLineParserConfigFileOption_test, UniqueIdOptionLeadsCallingCmdLineParserParseReturningNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e709ad5-61a7-44d1-a656-0d29ed1078fb");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-u";
    char value[] = "4242";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
}

TEST_F(CmdLineParserConfigFileOption_test, CmdLineParsingModeEqualToOneHandleOnlyTheFirstOptionReturningNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b55d7d0b-7dbe-4a86-9ece-9614ebc3d188");
    constexpr uint8_t NUMBER_OF_ARGS{5U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char uniqueIdOption[] = "-u";
    char value[] = "4242";
    char customOption[] = "-c";
    char path[] = "/foo/bar.toml";
    args[0] = &appName[0];
    args[1] = &uniqueIdOption[0];
    args[2] = &value[0];
    args[3] = &customOption[0];
    args[4] = &path[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args, CmdLineParser::CmdLineArgumentParsingMode::ONE);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(""));

    optind = 0;

    auto res = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(res.has_error());
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
    EXPECT_THAT(res.value().configFilePath.c_str(), StrEq(path));
}

} // namespace

#endif
