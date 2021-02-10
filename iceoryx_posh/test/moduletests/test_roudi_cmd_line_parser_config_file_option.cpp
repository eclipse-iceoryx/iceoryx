// Copyright (c) 2021 by  Apex.AI Inc. All rights reserved.
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
#include "iceoryx_utils/cxx/string.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;

using namespace iox::cxx;
using namespace iox::config;

namespace iox
{
namespace test
{
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
    constexpr uint8_t numberOfArgs{1U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    args[0] = &appName[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(""));
}

TEST_F(CmdLineParserConfigFileOption_test, ConfigPathShortOptionIsCorrectlyRead)
{
    constexpr uint8_t numberOfArgs{3U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "-c";
    char path[] = "/foo/bar.toml";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &path[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(path));
}

TEST_F(CmdLineParserConfigFileOption_test, ConfigPathLongOptionIsCorrectlyRead)
{
    constexpr uint8_t numberOfArgs{3U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "--config-file";
    char path[] = "/foo/bar/baz.toml";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &path[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().configFilePath.c_str(), StrEq(path));
}

TEST_F(CmdLineParserConfigFileOption_test, HelpLongOptionLeadsProgrammNotRunning)
{
    constexpr uint8_t numberOfArgs{2U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "--help";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().run, Eq(false));
}

TEST_F(CmdLineParserConfigFileOption_test, WrongOptionLeadsUnkownOptionResult)
{
    constexpr uint8_t numberOfArgs{2U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "--unknown";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(true));
    EXPECT_THAT(result.get_error(), Eq(CmdLineParserResult::UNKNOWN_OPTION_USED));
}

TEST_F(CmdLineParserConfigFileOption_test, UnknownOptionLeadsCallingCmdLineParserParseReturningNoError)
{
    constexpr uint8_t numberOfArgs{3U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "-u";
    char value[] = "4242";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().uniqueRouDiId.has_value(), Eq(true));
    EXPECT_THAT(result.value().uniqueRouDiId.value(), Eq(4242));
}

TEST_F(CmdLineParserConfigFileOption_test, CmdLineParsingModeEqualToOneReturnNoError)
{
    constexpr uint8_t numberOfArgs{2U};
    char* args[numberOfArgs];
    char appName[] = "./foo";
    char option[] = "--help";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParserConfigFileOption sut;
    auto result = sut.parse(numberOfArgs, args, CmdLineParser::CmdLineArgumentParsingMode::ONE);

    EXPECT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result.value().run, Eq(false));
}

} // namespace test
} // namespace iox

#endif
