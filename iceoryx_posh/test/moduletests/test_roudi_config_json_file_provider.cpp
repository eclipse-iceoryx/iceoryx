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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iceoryx_posh/roudi/roudi_config_json_file_provider.hpp"
#include "test.hpp"
#include <string>

using namespace iox::roudi;
using namespace iox::config;
using namespace iox;
using namespace ::testing;
using namespace std;

#if !defined(_WIN32)
const std::string TestFile = "JSON_CONFIG.tmp";
const std::string CrapFile = "CRAP_FILE.tmp";

const std::string TempPath = "/tmp";
const std::string TestFilePath = TempPath + "/" + TestFile;

class JsonConfig_test : public Test
{
  public:
    JsonConfig_test()
        : Test()
    {
    }
    virtual void SetUp()
    {
        char str[] = "{\n"
                     "\t\"general\":{\n"
                     "\t\t\"version\" : 1\n"
                     "\t},"
                     "\t\"segment\":[\n"
                     "\t\t{\n"
                     "\t\t\t\"mempool\":[\n"
                     "\t\t\t\t{\n"
                     "\t\t\t\t\t\"size\":32,\n"
                     "\t\t\t\t\t\"count\":10000\n"
                     "\t\t\t\t},\n"
                     "\t\t\t\t{\n"
                     "\t\t\t\t\t\"size\":128,\n"
                     "\t\t\t\t\t\"count\":10000\n"
                     "\t\t\t\t}\n"
                     "\t\t\t]\n"
                     "\t\t}\n"
                     "\t]\n"
                     "}\n";

        ofstream tmpFile;
        tmpFile.open(TestFilePath.c_str());
        tmpFile << str;
        tmpFile.close();
    }

    virtual void TearDown()
    {
        optind = 0;
    }
};

class JsonConfig_Failure_test : public Test
{
  public:
    JsonConfig_Failure_test()
        : Test()
    {
    }
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
        optind = 0;
    }

    iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError> parseJson(std::string json)
    {
        iox::config::CmdLineParserConfigFileOption cmdLineParser;
        int argc = 3;
        std::string arg1("executable");
        std::string arg2("--config-file");
        std::string arg3("/tmp/JSON_CONFIG.tmp");
        char* argv[3] = {&arg1[0], &arg2[0], &arg3[0]};

        ofstream tmpFile;
        tmpFile.open(TestFilePath.c_str());
        tmpFile << json;
        tmpFile.close();

        auto cmdLineArgs = cmdLineParser.parse(argc, argv);

        JsonRouDiConfigFileProvider reader(cmdLineArgs.value());
        return reader.parse();
    }
};

TEST_F(JsonConfig_test, TestReader)
{
    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    int argc = 3;
    std::string arg1("executable");
    std::string arg2("-c");
    std::string arg3(TestFilePath);
    char* argv[3] = {&arg1[0], &arg2[0], &arg3[0]};

    auto cmdLineArgs = cmdLineParser.parse(argc, argv);

    JsonRouDiConfigFileProvider reader(cmdLineArgs.value());

    iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError> config = reader.parse();
    ASSERT_FALSE(config.has_error());
    iox::RouDiConfig_t roudiConf = config.value();
    ASSERT_EQ(roudiConf.m_sharedMemorySegments.size(), 1);
    iox::mepoo::SegmentConfig::SegmentEntry segmentEntry = roudiConf.m_sharedMemorySegments[0];
    ASSERT_EQ(segmentEntry.m_mempoolConfig.m_mempoolConfig.size(), 2);
    iox::mepoo::MePooConfig::Entry entry1 = segmentEntry.m_mempoolConfig.m_mempoolConfig[0];
    ASSERT_EQ(entry1.m_size, 32);
    ASSERT_EQ(entry1.m_chunkCount, 10000);
    iox::mepoo::MePooConfig::Entry entry2 = segmentEntry.m_mempoolConfig.m_mempoolConfig[1];
    ASSERT_EQ(entry2.m_size, 128);
    ASSERT_EQ(entry2.m_chunkCount, 10000);
}

TEST_F(JsonConfig_test, No_Config_File)
{
    int argc = 3;
    std::string arg1("executable");
    std::string arg2("");
    std::string arg3("");
    char* argv[3] = {&arg1[0], &arg2[0], &arg3[0]};
    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    testing::internal::CaptureStderr();

    auto cmdLineArgs = cmdLineParser.parse(argc, argv);

    JsonRouDiConfigFileProvider reader(cmdLineArgs.value());
    std::string output = testing::internal::GetCapturedStderr();
    ASSERT_NE(output.find("No config file provided and also not found at"), string::npos);
}

TEST_F(JsonConfig_Failure_test, No_Segment_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t}\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, No_Segment_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},\n"
                 "\t\"segment\":12\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::NO_SEGMENTS, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, No_General_test)
{
    char str[] = "{\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, No_General_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":12\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::NO_GENERAL_SECTION, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Wrong_Version_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 0\n"
                 "\t}\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Wrong_Version_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : {}\n"
                 "\t}\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::INVALID_CONFIG_FILE_VERSION, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, To_Many_Segments_test)
{
    std::string config = "{\n"
                         "\t\"general\":{\n"
                         "\t\t\"version\" : 1\n"
                         "\t},"
                         "\t\"segment\":[\n";

    for (std::uint32_t i = 0; i < MAX_SHM_SEGMENTS + 1; ++i)
    {
        if (i > 0)
        {
            config += ",";
        }
        config += "\t\t{\n"
                  "\t\t\t\"mempool\":[\n"
                  "\t\t\t\t{\n"
                  "\t\t\t\t\t\"size\":32,\n"
                  "\t\t\t\t\t\"count\":10000\n"
                  "\t\t\t\t},\n"
                  "\t\t\t\t{\n"
                  "\t\t\t\t\t\"size\":128,\n"
                  "\t\t\t\t\t\"count\":10000\n"
                  "\t\t\t\t}\n"
                  "\t\t\t]\n"
                  "\t\t}\n";
    }
    config += "\t]\n"
              "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_SEGMENTS_EXCEEDED, parseJson(config).get_error());
}

TEST_F(JsonConfig_Failure_test, To_Many_Nodes_test)
{
    std::string config = "{\n"
                         "\t\"general\":{\n"
                         "\t\t\"version\" : 1\n"
                         "\t},"
                         "\t\"segment\":[\n";

    for (std::uint32_t i = 0; i < MAX_SHM_SEGMENTS + 1; ++i)
    {
        if (i > 0)
        {
            config += ",";
        }
        config += "\t\t{\n"
                  "\t\t\t\"mempool\":[\n";
        for (std::uint32_t j = 0; j < MAX_NUMBER_OF_MEMPOOLS; ++j)
        {
            if (j > 0)
            {
                config += ",";
            }
            config += "\t\t\t\t{\n"
                      "\t\t\t\t\t\"size\":32,\n"
                      "\t\t\t\t\t\"count\":10000\n"
                      "\t\t\t\t}\n";
        }
        config += "\t\t\t]\n"
                   "\t\t}\n";
    }
    config += "\t]\n"
              "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::INVALID_STATE, parseJson(config).get_error());
}

TEST_F(JsonConfig_Failure_test, To_Many_Mempools_test)
{
    std::string config = "{\n"
                         "\t\"general\":{\n"
                         "\t\t\"version\" : 1\n"
                         "\t},"
                         "\t\"segment\":[\n"
                         "\t\t{\n"
                         "\t\t\t\"mempool\":[\n";


    for (std::uint32_t i = 0; i < MAX_NUMBER_OF_MEMPOOLS + 1; ++i)
    {
        if (i > 0)
        {
            config += ",";
        }
        config += "\t\t\t\t{\n"
                  "\t\t\t\t\t\"size\":32,\n"
                  "\t\t\t\t\t\"count\":10000\n"
                  "\t\t\t\t}\n";
    }
    config += "\t\t\t]\n"
              "\t\t}\n"
              "\t]\n"
              "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MAX_NUMBER_OF_MEMPOOLS_PER_SEGMENT_EXCEEDED,
              parseJson(config).get_error());
}

TEST_F(JsonConfig_Failure_test, Segment_Without_Mempool_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Segment_Without_Mempool_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t12\n"
                 "\t\t{\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Segment_Without_Mempool_Wrong_Type2_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t\t\"no_mem\":[\n"
                 "\t\t\t]\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::SEGMENT_WITHOUT_MEMPOOL, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Mempool_Without_Chunk_Size_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t\t\"mempool\":[\n"
                 "\t\t\t\t{\n"
                 "\t\t\t\t\t\"count\":10000\n"
                 "\t\t\t\t}\n"
                 "\t\t\t]\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Mempool_Without_Chunk_Size_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t\t\"mempool\":[\n"
                 "\t\t\t\t{\n"
                 "\t\t\t\t\t\"size\":\"\"\n"
                 "\t\t\t\t\t\"count\":10000\n"
                 "\t\t\t\t}\n"
                 "\t\t\t]\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_SIZE, parseJson(str).get_error());
}


TEST_F(JsonConfig_Failure_test, Mempool_Without_Chunk_Count_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t\t\"mempool\":[\n"
                 "\t\t\t\t{\n"
                 "\t\t\t\t\t\"size\":32\n"
                 "\t\t\t\t}\n"
                 "\t\t\t]\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT, parseJson(str).get_error());
}

TEST_F(JsonConfig_Failure_test, Mempool_Without_Chunk_Count_Wrong_Type_test)
{
    char str[] = "{\n"
                 "\t\"general\":{\n"
                 "\t\t\"version\" : 1\n"
                 "\t},"
                 "\t\"segment\":[\n"
                 "\t\t{\n"
                 "\t\t\t\"mempool\":[\n"
                 "\t\t\t\t{\n"
                 "\t\t\t\t\t\"size\":10000\n"
                 "\t\t\t\t\t\"count\":\"\"\n"
                 "\t\t\t\t}\n"
                 "\t\t\t]\n"
                 "\t\t}\n"
                 "\t]\n"
                 "}\n";
    ASSERT_EQ(iox::roudi::RouDiConfigFileParseError::MEMPOOL_WITHOUT_CHUNK_COUNT, parseJson(str).get_error());
}

#endif
