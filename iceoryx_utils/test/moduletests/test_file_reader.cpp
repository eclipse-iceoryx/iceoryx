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

#include "test.hpp"
#include "iceoryx_utils/internal/file_reader/file_reader.hpp"

#include <cstdio>
#include <fstream>

using namespace ::testing;

const char* TestFileName = "/tmp/FileReader_test.tmp";

const char* TestFileContent = "This is a test file.\n"
                              "It consists of more than one line.\n\n"
                              "It does even contain empty lines, wow.";

class FileReader_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStdout();

        std::fstream fs(TestFileName, std::fstream::out | std::fstream::trunc);
        fs << TestFileContent;
        fs.close();
    }
    void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
        if (std::remove(TestFileName) != 0)
        {
            std::cerr << "Failed to remove temporary file '" << TestFileName
                      << "'. You'll have to remove it by yourself.";
        }
    }
};

TEST_F(FileReader_test, openNonExisting)
{
    iox::cxx::FileReader reader("a_file_that_wasn't_there.txt");
    EXPECT_FALSE(reader.IsOpen());
}

TEST_F(FileReader_test, openExisting)
{
    iox::cxx::FileReader reader(TestFileName);
    EXPECT_TRUE(reader.IsOpen());
}

TEST_F(FileReader_test, openWithPath)
{
    iox::cxx::FileReader reader("FileReader_test.tmp", "/tmp");
    EXPECT_TRUE(reader.IsOpen());

    iox::cxx::FileReader almostTheSameReader("FileReader_test.tmp", "/tmp/");
    EXPECT_TRUE(almostTheSameReader.IsOpen());
}

TEST_F(FileReader_test, openWithWrongPath)
{
    iox::cxx::FileReader reader("FileReader_test.tmp", "/tmp/what_path_is_this");
    EXPECT_FALSE(reader.IsOpen());
}

TEST_F(FileReader_test, readLines)
{
    iox::cxx::FileReader reader(TestFileName);
    std::string stringLine;

    bool isLineCorrect = reader.ReadLine(stringLine);
    EXPECT_TRUE(isLineCorrect);
    EXPECT_EQ(stringLine.compare("This is a test file."), 0);

    isLineCorrect = reader.ReadLine(stringLine);
    EXPECT_TRUE(isLineCorrect);
    EXPECT_EQ(stringLine.compare("It consists of more than one line."), 0);
}

TEST_F(FileReader_test, readAllLines)
{
    iox::cxx::FileReader reader(TestFileName);
    std::string stringLine;
    int numLines = 0;
    while (reader.ReadLine(stringLine))
    {
        ++numLines;
    }

    EXPECT_EQ(numLines, 4);
    EXPECT_EQ(stringLine.compare("It does even contain empty lines, wow."), 0);
}

TEST_F(FileReader_test, errorIgnoreMode)
{
    internal::CaptureStderr();
    iox::cxx::FileReader reader(
        "FileNotAvailable.readme", "PathThatNeverHasBeen", iox::cxx::FileReader::ErrorMode::Ignore);
    EXPECT_TRUE(internal::GetCapturedStderr().empty());
}

TEST_F(FileReader_test, errorInformMode)
{
    internal::CaptureStderr();
    iox::cxx::FileReader reader("FileNotFound.abc", "TheInfamousPath", iox::cxx::FileReader::ErrorMode::Inform);
    EXPECT_FALSE(internal::GetCapturedStderr().empty());
}

TEST_F(FileReader_test, errorTerminateMode)
{
    std::set_terminate([]() { std::cout << "", std::abort(); });

    EXPECT_DEATH(
        {
            iox::cxx::FileReader reader("ISaidNo!", "InTheMiddleOfNowhere", iox::cxx::FileReader::ErrorMode::Terminate);
        },
        ".*");
}
