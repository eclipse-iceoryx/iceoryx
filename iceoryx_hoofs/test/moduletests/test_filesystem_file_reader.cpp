// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/detail/hoofs_error_reporting.hpp"
#include "iox/file_reader.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/testing_logger.hpp"
#include "test.hpp"


#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
using namespace ::testing;
using namespace ::iox::testing;

const std::string TestFile = "FileReader_test.tmp";
#ifndef _WIN32
const std::string TempPath = "/tmp";
const std::string CrapPath = "/All/Hail/Hypnotoad";
const std::string TestFilePath = TempPath + "/" + TestFile;
#else
const std::string TempPath = std::getenv("TEMP");
const std::string CrapPath = "C:\\All\\Hail\\Hypnotoad";
const std::string TestFilePath = TempPath + "\\" + TestFile;
#endif

const char* TestFileContent = "This is a test file.\n"
                              "It consists of more than one line.\n\n"
                              "It does even contain empty lines, wow.";

class FileReader_test : public Test
{
  public:
    void SetUp() override
    {
        std::fstream fs(TestFilePath, std::fstream::out | std::fstream::trunc);
        if (fs.std::fstream::is_open())
        {
            fs << TestFileContent;
        }
        else
        {
            ASSERT_STREQ("expected open fstream", "fstream not open");
        }
        fs.close();
    }
    void TearDown() override
    {
        if (std::remove(TestFilePath.c_str()) != 0)
        {
            std::cerr << "Failed to remove temporary file '" << TestFilePath
                      << "'. You'll have to remove it by yourself.";
        }
    }
};

TEST_F(FileReader_test, openNonExisting)
{
    ::testing::Test::RecordProperty("TEST_ID", "2696e939-e5b8-4fd0-aed5-f1aae55b2c38");
    iox::FileReader reader("a_file_that_wasn't_there.txt");
    EXPECT_FALSE(reader.isOpen());
}

TEST_F(FileReader_test, openExisting)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6dd5e3e-32fd-4b6e-910e-65d70493e9d5");
    iox::FileReader reader(TestFilePath);
    EXPECT_TRUE(reader.isOpen());
}

TEST_F(FileReader_test, openWithPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "424bb5c8-6226-4a74-a1cd-2194787fabdd");
    iox::FileReader reader(TestFile, TempPath);
    EXPECT_TRUE(reader.isOpen());

    iox::FileReader almostTheSameReader(TestFile, TempPath);
    EXPECT_TRUE(almostTheSameReader.isOpen());
}

TEST_F(FileReader_test, openWithWrongPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "00933c79-94dd-48f5-9d7a-bc178ffd4222");
    iox::FileReader reader(TestFile, CrapPath);
    EXPECT_FALSE(reader.isOpen());
}

TEST_F(FileReader_test, readLines)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6c9fde1-4878-42aa-8388-320468d51b62");
    iox::FileReader reader(TestFilePath);
    std::string stringLine;

    bool isLineCorrect = reader.readLine(stringLine);
    EXPECT_TRUE(isLineCorrect);
    EXPECT_EQ(stringLine.compare("This is a test file."), 0);

    isLineCorrect = reader.readLine(stringLine);
    EXPECT_TRUE(isLineCorrect);
    EXPECT_EQ(stringLine.compare("It consists of more than one line."), 0);
}

TEST_F(FileReader_test, readAllLines)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f2bda7a-88f8-4459-8a3e-a3b3d3809b7c");
    iox::FileReader reader(TestFilePath);
    std::string stringLine;
    int numLines = 0;
    while (reader.readLine(stringLine))
    {
        ++numLines;
    }

    EXPECT_EQ(numLines, 4);
    EXPECT_EQ(stringLine.compare("It does even contain empty lines, wow."), 0);
}

TEST_F(FileReader_test, errorIgnoreMode)
{
    ::testing::Test::RecordProperty("TEST_ID", "4155a17f-2ac3-4240-b0e5-f9bb704cc03d");

    iox::FileReader reader("FileNotAvailable.readme", "PathThatNeverHasBeen", iox::FileReader::ErrorMode::Ignore);

    iox::testing::TestingLogger::checkLogMessageIfLogLevelIsSupported(
        iox::log::LogLevel::ERROR, [&](const auto& logMessages) { ASSERT_THAT(logMessages.size(), Eq(0U)); });
}

TEST_F(FileReader_test, errorInformMode)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5dd405e-e8cc-4c86-a4a2-02d38830a4d6");

    iox::FileReader reader("FileNotFound.abc", "TheInfamousPath", iox::FileReader::ErrorMode::Inform);

    const std::string expectedOutput = "Could not open file 'FileNotFound.abc' from path 'TheInfamousPath'.";
    iox::testing::TestingLogger::checkLogMessageIfLogLevelIsSupported(
        iox::log::LogLevel::ERROR, [&](const auto& logMessages) {
            ASSERT_THAT(logMessages.size(), Eq(1U));
            EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
        });
}

TEST_F(FileReader_test, errorTerminateMode)
{
    ::testing::Test::RecordProperty("TEST_ID", "146e3109-6d98-44ee-a3a9-5d151616a212");

    const std::string fileName{"ISaidNo!"};
    const std::string filePath{"InTheMiddleOfNowhere"};

    IOX_EXPECT_FATAL_FAILURE([&] { iox::FileReader reader(fileName, filePath, iox::FileReader::ErrorMode::Terminate); },
                             iox::er::FATAL);

    const std::string expectedOutput = "Could not open file 'ISaidNo!' from path 'InTheMiddleOfNowhere'!";
    iox::testing::TestingLogger::checkLogMessageIfLogLevelIsSupported(
        iox::log::LogLevel::FATAL, [&](const auto& logMessages) {
            ASSERT_THAT(logMessages.size(), Gt(1U));
            EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
        });
}
} // namespace
