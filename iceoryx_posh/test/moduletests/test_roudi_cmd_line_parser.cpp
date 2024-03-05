// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"

using namespace ::testing;

using namespace iox::roudi;
using namespace iox::config;
using namespace iox::log;
using namespace iox::units;
using namespace iox::version;

namespace iox
{
namespace config
{
bool operator==(const CmdLineArgs_t& lhs, const CmdLineArgs_t& rhs)
{
    return (lhs.roudiConfig.monitoringMode == rhs.roudiConfig.monitoringMode)
           && (lhs.roudiConfig.logLevel == rhs.roudiConfig.logLevel)
           && (lhs.roudiConfig.compatibilityCheckLevel == rhs.roudiConfig.compatibilityCheckLevel)
           && (lhs.roudiConfig.processTerminationDelay == rhs.roudiConfig.processTerminationDelay)
           && (lhs.roudiConfig.processKillDelay == rhs.roudiConfig.processKillDelay)
           && (lhs.roudiConfig.domainId == rhs.roudiConfig.domainId)
           && (lhs.roudiConfig.uniqueRouDiId == rhs.roudiConfig.uniqueRouDiId) && (lhs.run == rhs.run)
           && (lhs.configFilePath == rhs.configFilePath);
}
} // namespace config
} // namespace iox

namespace
{
class CmdLineParser_test : public Test
{
  public:
    void SetUp(){};
    void TearDown()
    {
        // Reset optind to be able to parse again
        optind = 0;
    };

    void testLogLevel(uint8_t numberOfArgs, char* args[], LogLevel level)
    {
        CmdLineParser sut;
        auto result = sut.parse(numberOfArgs, args);

        ASSERT_FALSE(result.has_error());
        EXPECT_EQ(result.value().roudiConfig.logLevel, level);
        EXPECT_TRUE(result.value().run);

        // Reset optind to be able to parse again
        optind = 0;
    }

    void testMonitoringMode(uint8_t numberOfArgs, char* args[], MonitoringMode mode)
    {
        CmdLineParser sut;
        auto result = sut.parse(numberOfArgs, args);

        ASSERT_FALSE(result.has_error());
        EXPECT_EQ(result.value().roudiConfig.monitoringMode, mode);
        EXPECT_TRUE(result.value().run);

        // Reset optind to be able to parse again
        optind = 0;
    }

    void testCompatibilityLevel(uint8_t numberOfArgs, char* args[], CompatibilityCheckLevel level)
    {
        CmdLineParser sut;
        auto result = sut.parse(numberOfArgs, args);

        ASSERT_FALSE(result.has_error());
        EXPECT_EQ(result.value().roudiConfig.compatibilityCheckLevel, level);
        EXPECT_TRUE(result.value().run);

        // Reset optind to be able to parse again
        optind = 0;
    }
};

TEST_F(CmdLineParser_test, NoOptionLeadsToDefaultValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "2da1d849-a652-4c23-ab74-af0dd618a552");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];
    const CmdLineArgs_t defaultValues;

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value(), defaultValues);
}

TEST_F(CmdLineParser_test, WrongOptionLeadsUnkownOptionResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "11c1ff6b-1676-41e8-aed8-97fc2eb88b06");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--ICanHazLulz";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), CmdLineParserResult::UNKNOWN_OPTION_USED);
}

TEST_F(CmdLineParser_test, HelpLongOptionLeadsToProgrammNotRunning)
{
    ::testing::Test::RecordProperty("TEST_ID", "b91d25db-31b7-4d3c-85cb-971236cff6a3");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--help";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(result.value().run);
}

TEST_F(CmdLineParser_test, HelpShortOptionLeadsToProgrammNotRunning)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5f9bf26-99e2-4cae-b9b4-5ff1da4afc86");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-h";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(result.value().run);
}

TEST_F(CmdLineParser_test, VersionShortOptionLeadsToProgrammNotRunning)
{
    ::testing::Test::RecordProperty("TEST_ID", "66465334-ca98-4260-9a32-0269bc415a22");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-v";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(result.value().run);
}

TEST_F(CmdLineParser_test, VersionLongOptionLeadsToProgrammNotRunning)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa82d46e-6474-4158-b0f9-c970e5895827");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--version";
    args[0] = &appName[0];
    args[1] = &option[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(result.value().run);
}

TEST_F(CmdLineParser_test, MonitoringModeOptionsLeadToCorrectMode)
{
    ::testing::Test::RecordProperty("TEST_ID", "362435bb-c35b-4617-b08b-17c359543c69");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    MonitoringMode modeArray[] = {MonitoringMode::ON, MonitoringMode::OFF};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char optionArray[][20] = {"-m", "--monitoring-mode"};
    char valueArray[][10] = {"on", "off"};
    args[0] = &appName[0];

    for (auto optionValue : optionArray)
    {
        args[1] = optionValue;
        uint8_t i{0U};
        for (auto expectedValue : modeArray)
        {
            args[2] = valueArray[i];
            testMonitoringMode(NUMBER_OF_ARGS, args, expectedValue);
            i++;
        }
    }
}

TEST_F(CmdLineParser_test, WrongMonitoringModeOptionLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c2b81f2-1ba6-486f-8ec3-4cafbd0cdb3c");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-m";
    char wrongValue[] = "DontBlink";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &wrongValue[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, LogLevelOptionsLeadToCorrectLogLevel)
{
    ::testing::Test::RecordProperty("TEST_ID", "25799d7a-9f34-4bcd-bb01-f6dbe270fac3");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    LogLevel loglevelArray[] = {LogLevel::OFF,
                                LogLevel::FATAL,
                                LogLevel::ERROR,
                                LogLevel::WARN,
                                LogLevel::INFO,
                                LogLevel::DEBUG,
                                LogLevel::TRACE};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char optionArray[][20] = {"-l", "--log-level"};
    char valueArray[][10] = {"off", "fatal", "error", "warning", "info", "debug", "trace"};
    args[0] = &appName[0];

    for (auto optionValue : optionArray)
    {
        args[1] = optionValue;
        uint8_t i{0U};
        for (auto expectedValue : loglevelArray)
        {
            args[2] = valueArray[i];
            testLogLevel(NUMBER_OF_ARGS, args, expectedValue);
            i++;
        }
    }
}

TEST_F(CmdLineParser_test, WrongLogLevelOptionLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "00a08b20-bdf9-436a-9ead-a65e19c89f91");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-l";
    char wrongValue[] = "TimeyWimey";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &wrongValue[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, KillDelayLongOptionLeadsToCorrectDelay)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5f328a0-96e8-4ca2-8b35-efa0d9c55538");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--kill-delay";
    char value[] = "73";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.processKillDelay, Duration::fromSeconds(73));
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, KillDelayShortOptionLeadsToCorrectDelay)
{
    ::testing::Test::RecordProperty("TEST_ID", "73d31293-a428-48e6-9c04-a22a276117ab");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-k";
    char value[] = "42";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.processKillDelay, Duration::fromSeconds(42));
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, KillDelayOptionOutOfBoundsLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb6a67cd-4e5a-41df-bf79-ef5dcdb13fbf");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--kill-delay";
    char value[] = "4294967296"; // MAX_PROCESS_KILL_DELAY + 1
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, TerminationDelayLongOptionLeadsToCorrectDelay)
{
    ::testing::Test::RecordProperty("TEST_ID", "9125f775-93b6-4560-a535-f8ecf77671b5");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--termination-delay";
    char value[] = "73";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.processTerminationDelay, 73_s);
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, TerminationDelayShortOptionLeadsToCorrectDelay)
{
    ::testing::Test::RecordProperty("TEST_ID", "05b24a16-334b-4b42-8846-766b0c99943b");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-t";
    char value[] = "42";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.processTerminationDelay, 42_s);
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, TerminationDelayOptionOutOfBoundsLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "7eaffeb5-a0e5-4cdd-a898-d9d64c999d16");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--termination-delay";
    char value[] = "4294967296"; // MAX_PROCESS_TERMINATION_DELAY + 1
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, CompatibilityLevelOptionsLeadToCorrectCompatibilityLevel)
{
    ::testing::Test::RecordProperty("TEST_ID", "62b7d5c9-0638-4314-b4f7-c622ef101045");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    CompatibilityCheckLevel loglevelArray[] = {CompatibilityCheckLevel::OFF,
                                               CompatibilityCheckLevel::MAJOR,
                                               CompatibilityCheckLevel::MINOR,
                                               CompatibilityCheckLevel::PATCH,
                                               CompatibilityCheckLevel::COMMIT_ID,
                                               CompatibilityCheckLevel::BUILD_DATE};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char optionArray[][20] = {"-x", "--compatibility"};
    char valueArray[][10] = {"off", "major", "minor", "patch", "commitId", "buildDate"};
    args[0] = &appName[0];

    for (auto optionValue : optionArray)
    {
        args[1] = optionValue;
        uint8_t i{0U};
        for (auto expectedValue : loglevelArray)
        {
            args[2] = valueArray[i];
            testCompatibilityLevel(NUMBER_OF_ARGS, args, expectedValue);
            i++;
        }
    }
}

TEST_F(CmdLineParser_test, WrongCompatibilityLevelOptionLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f270804c-428c-410f-865d-2dc039fcb401");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-x";
    char wrongValue[] = "AmyPond";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &wrongValue[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, DomainIdLongOptionLeadsToCorrectDomainId)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b5d76d1-d935-47e6-aaaa-df5058d35dad");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--domain-id";
    char value[] = "73";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.domainId, iox::DomainId{73});
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, DomainIdShortOptionLeadsToCorrectDomainId)
{
    ::testing::Test::RecordProperty("TEST_ID", "484eb1b5-60c3-4b93-af3a-9a55c4d206f8");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-d";
    char value[] = "73";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.domainId, iox::DomainId{73});
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, OutOfBoundsDomainIdOptionLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "ddca4085-ad28-4309-965a-8ed82a3924bc");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-d";
    char value[] = "65536";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, UniqueIdLongOptionLeadsToCorrectUniqueId)
{
    ::testing::Test::RecordProperty("TEST_ID", "84d0b41d-44ae-498d-81a9-c94b92255f2b");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--unique-roudi-id";
    char value[] = "4242";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, UniqueIdShortOptionLeadsToCorrectUniqueId)
{
    ::testing::Test::RecordProperty("TEST_ID", "a12e391c-c382-4eba-b620-59dcb1a150d9");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-u";
    char value[] = "4242";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
    EXPECT_TRUE(result.value().run);
}

TEST_F(CmdLineParser_test, OutOfBoundsUniqueIdOptionLeadsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "13f6b1fb-3c6d-4dda-87ab-d883533bb1ed");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-u";
    char wrongValue[] = "65536"; // MAX_ROUDI_ID + 1
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &wrongValue[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(CmdLineParserResult::INVALID_PARAMETER));
}

TEST_F(CmdLineParser_test, CmdLineParsingModeEqualToOneHandlesOnlyTheFirstOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e674db9-d71a-4b82-83cc-eea2e04f4601");
    constexpr uint8_t NUMBER_OF_ARGS{9U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char domainIdOption[] = "-d";
    char domainIdValue[] = "73";
    char uniqueRouDiIdOption[] = "-u";
    char uniqueRouDiIdValue[] = "4242";
    char killOption[] = "-k";
    char killValue[] = "42";
    char terminationOption[] = "-t";
    char terminationValue[] = "2";
    args[0] = &appName[0];
    args[1] = &domainIdOption[0];
    args[2] = &domainIdValue[0];
    args[3] = &uniqueRouDiIdOption[0];
    args[4] = &uniqueRouDiIdValue[0];
    args[5] = &killOption[0];
    args[6] = &killValue[0];
    args[7] = &terminationOption[0];
    args[8] = &terminationValue[0];

    CmdLineParser sut;
    auto result = sut.parse(NUMBER_OF_ARGS, args, CmdLineParser::CmdLineArgumentParsingMode::ONE);

    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value().roudiConfig.domainId, iox::DomainId{73});
    EXPECT_EQ(result.value().roudiConfig.uniqueRouDiId, iox::roudi::DEFAULT_UNIQUE_ROUDI_ID);
    EXPECT_EQ(result.value().roudiConfig.processTerminationDelay, iox::roudi::PROCESS_DEFAULT_TERMINATION_DELAY);
    EXPECT_EQ(result.value().roudiConfig.processKillDelay,
              iox::roudi::PROCESS_DEFAULT_KILL_DELAY); // default value for kill delay
    EXPECT_TRUE(result.value().run);

    optind = 0;

    auto res = sut.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(res.has_error());
    EXPECT_EQ(result.value().roudiConfig.domainId, iox::DomainId{73});
    EXPECT_EQ(res.value().roudiConfig.uniqueRouDiId, iox::roudi::UniqueRouDiId{4242});
    EXPECT_EQ(res.value().roudiConfig.processTerminationDelay, 2_s);
    EXPECT_EQ(res.value().roudiConfig.processKillDelay, 42_s);
    EXPECT_TRUE(res.value().run);
}

} // namespace
#endif
