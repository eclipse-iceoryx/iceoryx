// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iox/filesystem.hpp"
#include "test.hpp"

#include <array>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::internal;

using iox::testing::Logger_Mock;


// BEGIN file and directory path tests

constexpr uint64_t FILE_PATH_LENGTH = 128U;

bool isValidFileCharacter(const int32_t i) noexcept
{
    return ((ASCII_A <= i && i <= ASCII_Z) || (ASCII_CAPITAL_A <= i && i <= ASCII_CAPITAL_Z)
            || (ASCII_0 <= i && i <= ASCII_9) || i == ASCII_DASH || i == ASCII_DOT || i == ASCII_COLON
            || i == ASCII_UNDERSCORE);
}

TEST(filesystem_test_isValidFileName, CorrectInternalAsciiAliases)
{
    ::testing::Test::RecordProperty("TEST_ID", "e729a0a1-e3c4-4d97-a948-d88017f6ac1e");
    EXPECT_EQ(ASCII_A, 'a');
    EXPECT_EQ(ASCII_Z, 'z');
    EXPECT_EQ(ASCII_CAPITAL_A, 'A');
    EXPECT_EQ(ASCII_CAPITAL_Z, 'Z');
    EXPECT_EQ(ASCII_0, '0');
    EXPECT_EQ(ASCII_9, '9');
    EXPECT_EQ(ASCII_DASH, '-');
    EXPECT_EQ(ASCII_DOT, '.');
    EXPECT_EQ(ASCII_COLON, ':');
    EXPECT_EQ(ASCII_UNDERSCORE, '_');
}

TEST(filesystem_test_isValidFileName, EmptyNameIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2b7aa63-c67e-4915-a906-e3b4779ab772");
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("")));
}

TEST(filesystem_test_isValidFileName, RelativePathComponentsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b33b4534-f134-499f-ac72-65a3fecaef12");
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(".")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("..")));
}

// this restriction ensures that we are compatible with the windows
// api which does not support dots and spaces at the end
TEST(filesystem_test_isValidFileName, DotsAndSpacesAreNotValidAtTheEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "436b8146-6386-4b03-9fd0-939d2c91eed3");
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("dot.")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("dotdot..")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("dotdotdot...")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(" ")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(" .")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(" . ")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(". .")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("space ")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("more space  ")));
}

TEST(filesystem_test_isValidFileName, FileNameWithValidSymbolsAndDotsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "1455491c-1fc3-4843-a72b-2f51f8f2fadc");
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna.blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".bla.b.a.sla.a")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("...fuu...man...schmu")));
}

TEST(filesystem_test_isValidFileName, ValidLetterCombinationsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a8661ad-4511-4e54-8cd9-16f21074c332");
    constexpr uint32_t COMBINATION_CAPACITY = 3U;
    std::array<std::string, COMBINATION_CAPACITY> combinations;

    constexpr int32_t MAX_ASCII_CODE = 255;
    for (int32_t i = 0; i <= MAX_ASCII_CODE; ++i)
    {
        // for simplicity we exclude the valid dot here, since it is
        // invalid when it occurs alone.
        // it is tested separately
        if (i != ASCII_DOT && isValidFileCharacter(i))
        {
            uint32_t index = static_cast<uint32_t>(i) % COMBINATION_CAPACITY;

            // index is always in the range of [0, COMBINATION_CAPACITY] since we calculate % COMBINATION_CAPACITY
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            auto& s = combinations[index];
            s.append(1, static_cast<char>(i));

            EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(iox::TruncateToCapacity, s.c_str(), s.size())));
        }
    }
}

TEST(filesystem_test_isValidFileName, WhenOneInvalidCharacterIsContainedFileNameIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "067ddf95-8a5c-442b-8022-ecab580b5a7d");
    std::string validName1 = "summon";
    std::string validName2 = "TheHolyToad";

    constexpr int32_t MAX_ASCII_CODE = 255;
    for (int32_t i = 0; i <= MAX_ASCII_CODE; ++i)
    {
        if (isValidFileCharacter(i))
        {
            continue;
        }

        std::string invalidCharacterFront;
        invalidCharacterFront.append(1, static_cast<char>(i));
        invalidCharacterFront += validName1 + validName2;

        std::string invalidCharacterMiddle = validName1;
        invalidCharacterMiddle.append(1, static_cast<char>(i));

        std::string invalidCharacterEnd = validName1 + validName2;
        invalidCharacterEnd.append(1, static_cast<char>(i));

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(
            iox::TruncateToCapacity, invalidCharacterFront.c_str(), invalidCharacterFront.size());
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(
            iox::TruncateToCapacity, invalidCharacterMiddle.c_str(), invalidCharacterMiddle.size());
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(
            iox::TruncateToCapacity, invalidCharacterEnd.c_str(), invalidCharacterEnd.size());

        EXPECT_FALSE(isValidFileName(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterEndTest));
    }
}

TEST(filesystem_test_isValidPathToFile, StringWithEndingSlashIsNotAFilePath)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0eecf9b-6f2f-4da2-8a18-466504348c50");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("//")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("../")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("////")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fu/bla/far/")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/schnappa/di/puppa//")));
}

TEST(filesystem_test_isValidPathToFile, MultipleSlashsAreValidFilePath)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7621d88-d128-4239-8acc-b18f47c92b62");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("//beginning/double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/middle//double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("middle//double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/multi////slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("////multi/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("//multi///slash////hypno")));
}

TEST(filesystem_test_isValidPathToFile, RelativePathComponentsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec7d682f-ac7b-4173-a3f6-55969696ee92");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("../some.file")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./another_file")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./../.././gimme-blubb")));
}

TEST(filesystem_test_isValidPathToFile, RelativePathBeginningFromRootIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "30c24356-1777-42a0-906b-73890fd19830");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/../../../gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb")));
}

TEST(filesystem_test_isValidPathToFile, SingleFileIsValidPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "264d792f-34cb-4bc0-886c-ac9de05bb1f9");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("a")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("fuu:blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/blarbi")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/x")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fuu:-012")));
}

TEST(filesystem_test_isValidPathToFile, ValidPathsWithNoRelativeComponentAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "5556ef38-b028-4155-86c7-dda9530e8611");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/balaa")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/4")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/1")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("123/456")));
}

TEST(filesystem_test_isValidPathToFile, EndingWithRelativePathComponentIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3a5c3e6-840d-4ed5-8064-fede7404391d");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/..")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("./..")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("../.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("some/path/to/..")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/another/path/to/.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("../bla/fuu/../blubb/.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("./blubb/fuu/../bla/..")));
}

TEST(filesystem_test_isValidPathToFile, FilePathsWithEndingDotsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b0dd948-49a0-4eb6-9c78-bad6e6933833");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(filesystem_test_isValidPathToFile, PathWhichContainsAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2667afd7-f60c-4d1a-8eff-bf272c68b47a");
    EXPECT_TRUE(isValidPathToFile(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidPathToFile(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(filesystem_test_isValidPathToFile, EmptyFilePathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "a045581c-3a66-4d0e-b2e2-6ed5a97d4f89");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("")));
}

TEST(filesystem_test_isValidPathToFile_isValidPathToDirectory_isValidPathEntry,
     WhenOneInvalidCharacterIsContainedPathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "a764cff3-2607-47bb-952b-4ca75f326721");
    std::string validPath1 = "/hello";
    std::string validPath2 = "fuu/world";

    // begin at 1 since 0 is string termination
    constexpr int32_t MAX_ASCII_CODE = 255;
    for (int32_t i = 1; i <= MAX_ASCII_CODE; ++i)
    {
        // ignore valid characters
        if (isValidFileCharacter(i))
        {
            continue;
        }

        // ignore path separators since they are valid path characters
        bool isPathSeparator = false;
        for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
        {
            if (static_cast<char>(i) == separator)
            {
                isPathSeparator = true;
                break;
            }
        }

        if (isPathSeparator)
        {
            continue;
        }

        // test
        std::string invalidCharacterFront;
        invalidCharacterFront.resize(1);
        invalidCharacterFront[0] = static_cast<char>(i);
        invalidCharacterFront += validPath1 + validPath2;

        std::string invalidCharacterMiddle = validPath1;
        invalidCharacterMiddle.resize(invalidCharacterMiddle.size() + 1);
        invalidCharacterMiddle[invalidCharacterMiddle.size() - 1] = static_cast<char>(i);

        std::string invalidCharacterEnd = validPath1 + validPath2;
        invalidCharacterEnd.resize(invalidCharacterEnd.size() + 1);
        invalidCharacterEnd[invalidCharacterEnd.size() - 1] = static_cast<char>(i);

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(
            iox::TruncateToCapacity, invalidCharacterFront.c_str(), invalidCharacterFront.size());
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(
            iox::TruncateToCapacity, invalidCharacterMiddle.c_str(), invalidCharacterMiddle.size());
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(
            iox::TruncateToCapacity, invalidCharacterEnd.c_str(), invalidCharacterEnd.size());

        EXPECT_FALSE(isValidPathToFile(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidPathToFile(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidPathToFile(invalidCharacterEndTest));

        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterEndTest));

        EXPECT_FALSE(isValidPathEntry(invalidCharacterFrontTest, iox::RelativePathComponents::ACCEPT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterMiddleTest, iox::RelativePathComponents::ACCEPT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterEndTest, iox::RelativePathComponents::ACCEPT));

        EXPECT_FALSE(isValidPathEntry(invalidCharacterFrontTest, iox::RelativePathComponents::REJECT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterMiddleTest, iox::RelativePathComponents::REJECT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterEndTest, iox::RelativePathComponents::REJECT));
    }
}

TEST(filesystem_test_isValidPathToDirectory, MultipleSlashsAreValidPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "14c6f67f-486a-4b08-a91a-6ef30af84cce");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("//beginning/double/slash")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("//beginning/double/slash//")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/middle//double/slash")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("middle//double/slash")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("middle//double/slash//")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/multi////slash")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/multi////slash////")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("////multi/slash")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("//multi///slash////hypno")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("//multi///slash////hypno////")));
}

TEST(filesystem_test_isValidPathToDirectory, RelativePathComponentsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "97c215ca-7f67-4ec1-9b17-d98b219a804d");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../some.file")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../some.dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./another_file")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./another_dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar/dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./././gimme-blubb/dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./../.././gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./../.././gimme-blubb/dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("all/glory/to/the/hypnotoad")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./all/glory/to/the/hypnotoad/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../all/glory/to/the/hypnotoad/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../all/glory/to/the/hypnotoad/../")));
}

TEST(filesystem_test_isValidPathToDirectory, RelativePathBeginningFromRootIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d2b2656-19ad-4ea0-9ade-77419af849ba");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/./././gimme-blubb/dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/../../../gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/../../../gimme-blubb/dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb/./dir/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb/../dir/")));
}

TEST(filesystem_test_isValidPathToDirectory, SingleEntryIsValidPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "6983ab77-d658-408d-97aa-bd1d218560fb");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("gimme-blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("gimme-blubb/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("a")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("a/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("fuu:blubb")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("fuu:blubb/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/blarbi")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/blarbi/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/x")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/x/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/fuu:-012")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/fuu:-012/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./hypnotoad")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./hypnotoad/")));
}

TEST(filesystem_test_isValidPathToDirectory, ValidPathsWithNoRelativeComponentAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf7a0a75-c59e-46a8-96f1-1f848e1c3e43");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/balaa")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/4")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/1")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("123/456")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("123/456/")));
}

TEST(filesystem_test_isValidPathToDirectory, EndingWithRelativePathComponentIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "506f9823-39cc-4cbc-b064-84d45b2311e8");
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/..")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/.")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./..")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../.")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("some/path/to/..")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/another/path/to/.")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("../bla/fuu/../blubb/.")));
    EXPECT_TRUE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("./blubb/fuu/../bla/..")));
}

TEST(filesystem_test_isValidPathToDirectory, PathsWithEndingDotsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "f79660e6-12b5-4ad0-bc26-766da34898b8");
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(filesystem_test_isValidPathToDirectory, PathWhichContainsAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "8052b601-c9ad-4cb8-9a87-c301f213d8c4");
    EXPECT_TRUE(isValidPathToDirectory(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidPathToDirectory(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(filesystem_test_isValidPathToDirectory, EmptyPathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "9724b52e-2e5a-425f-853d-a0b43e553f8b");
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("")));
}

TEST(filesystem_test_doesEndWithPathSeparator, EmptyPathDoesNotEndWithPathSeparator)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe0be1e0-fdd5-4d56-841c-83826c40c3d2");
    EXPECT_FALSE(doesEndWithPathSeparator(string<FILE_PATH_LENGTH>("")));
}

TEST(filesystem_test_doesEndWithPathSeparator, NonEmptyPathWithNoPathSeparatorAtTheEndDoesNotEndWithPathSeparator)
{
    ::testing::Test::RecordProperty("TEST_ID", "a6d10202-aea0-4b1c-b9d9-704545102a2e");

    string<FILE_PATH_LENGTH> sut = "isThereOnlyOneHypnotoad";
    EXPECT_FALSE(doesEndWithPathSeparator(sut));

    sut.unsafe_append(iox::platform::IOX_PATH_SEPARATORS);
    sut.unsafe_append("thereIsOnlyOne");
    EXPECT_FALSE(doesEndWithPathSeparator(sut));
}

TEST(filesystem_test_doesEndWithPathSeparator, SingleCharacterStringOnlyWithPathSeparatorAsOneAtTheEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "18bf45aa-9b65-4351-956a-8ddc98fa0296");

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        string<FILE_PATH_LENGTH> sut = " ";
        sut[0] = separator;
        EXPECT_TRUE(doesEndWithPathSeparator(sut));
    }
}

TEST(filesystem_test_doesEndWithPathSeparator, MultiCharacterStringEndingWithPathSeparatorAsOneAtTheEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "c702ec34-8f7f-4220-b50e-6b231ac4e736");

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        string<FILE_PATH_LENGTH> sut = "HypnotoadAteTheSpagettiMonster";
        ASSERT_TRUE(sut.unsafe_append(separator));
        EXPECT_TRUE(doesEndWithPathSeparator(sut));
    }
}

TEST(filesystem_test_isValidPathEntry, EmptyPathEntryIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "1280b360-f26c-4ddf-8305-e01a99d58178");
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(""), iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, PathEntryWithOnlyValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "166fb334-05c6-4b8c-a117-223d6cadb29b");
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("a"), iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("agc"), iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("a.213jkgc"),
                                 iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, RelativePathEntriesAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3432692-7cee-416a-a3f3-c246a02ad1a2");
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("."), iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".."), iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, EntriesWithEndingDotAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "f937de46-19fc-48da-bce6-51292cd9d75e");
    EXPECT_FALSE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("abc."), iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("19283912asdb.."),
                                  iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..19283912asdb.."),
                                  iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..192839.12a.sdb.."),
                                  iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, EntriesWithDotsNotAtTheEndAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "569aa328-2c47-418d-96e2-ddf73925e52f");
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".abc"), iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".19283912asdb"),
                                 iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..19283912asdb"),
                                 iox::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..192839.12a.sdb"),
                                 iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, StringContainingAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2c19516-e8fb-4fb8-a366-2b7b5fd9a84b");
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(
                                     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_"),
                                 iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, StringWithSlashIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1119db1-f897-48a5-af92-9a92eb3f9832");
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("/fuuuu/"),
                                  iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("fuu/uu"),
                                  iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("/fuuuu"),
                                  iox::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("uuuubbuu/"),
                                  iox::RelativePathComponents::ACCEPT));
}

TEST(filesystem_test_isValidPathEntry, StringWithRelativeComponentsIsInvalidWhenItContainsRelativeComponents)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c73e08e-3b42-446e-b8d4-a4ed7685f28e");
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../to/be"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../../or/not"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/../be"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("that/../../is/the/question"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("whether/tis/nobler/.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("in/the/mind/to/suffer//../.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../the/slings/and/arrows/../.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../of/../outrageous/fortune/../.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./or/to/take/../arms/../.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./agains/a/see/./of/troubles/../.."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./and/by/../opposing/./."),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./end/them"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/./die"),
                                  iox::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/./sleep/."),
                                  iox::RelativePathComponents::REJECT));
}

// END file and directory path tests


// BEGIN AccessMode and OpenMode tests

constexpr AccessMode INVALID_ACCESS_MODE =
    static_cast<AccessMode>(std::numeric_limits<std::underlying_type_t<AccessMode>>::max());
constexpr OpenMode INVALID_OPEN_MODE =
    static_cast<OpenMode>(std::numeric_limits<std::underlying_type_t<OpenMode>>::max());

TEST(TypesTest, ConvertToOflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb74e8c-7498-4400-9248-92aa6bd15142");
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE), Eq(0U));
}

TEST(TypesTest, ConvertToProtflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a5c699e-16e6-471f-80b6-a325644e60d3");
    EXPECT_THAT(convertToProtFlags(AccessMode::READ_ONLY), Eq(PROT_READ));
    EXPECT_THAT(convertToProtFlags(AccessMode::READ_WRITE), Eq(PROT_READ | PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(AccessMode::WRITE_ONLY), Eq(PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(INVALID_ACCESS_MODE), Eq(PROT_NONE));
}

TEST(TypesTest, ConvertToOflagFromOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "95fa55c9-2d64-4296-8bbb-41ff3c9dac3f");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, ConvertToOflagFromAccessAndOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ea6823c-2ecd-48a5-bcea-0ea0585bee72");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::EXCLUSIVE_CREATE), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::PURGE_AND_CREATE), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::OPEN_OR_CREATE), Eq(O_RDONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::OPEN_EXISTING), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, INVALID_OPEN_MODE), Eq(O_RDONLY));

    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::EXCLUSIVE_CREATE), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::PURGE_AND_CREATE), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::OPEN_OR_CREATE), Eq(O_RDWR | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::OPEN_EXISTING), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, INVALID_OPEN_MODE), Eq(O_RDWR));

    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::EXCLUSIVE_CREATE), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::PURGE_AND_CREATE), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::OPEN_OR_CREATE), Eq(O_WRONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::OPEN_EXISTING), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, INVALID_OPEN_MODE), Eq(O_WRONLY));

    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, OpenModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "830756de-b3c9-4285-b42a-e0c6c5a315a9");
    EXPECT_THAT(asStringLiteral(OpenMode::EXCLUSIVE_CREATE), StrEq("OpenMode::EXCLUSIVE_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::PURGE_AND_CREATE), StrEq("OpenMode::PURGE_AND_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::OPEN_OR_CREATE), StrEq("OpenMode::OPEN_OR_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::OPEN_EXISTING), StrEq("OpenMode::OPEN_EXISTING"));
    EXPECT_THAT(asStringLiteral(INVALID_OPEN_MODE), StrEq("OpenMode::UNDEFINED_VALUE"));
}

TEST(TypesTest, AccessModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5a09ee7-df2c-4a28-929c-7de743f1e423");
    EXPECT_THAT(asStringLiteral(AccessMode::READ_ONLY), StrEq("AccessMode::READ_ONLY"));
    EXPECT_THAT(asStringLiteral(AccessMode::READ_WRITE), StrEq("AccessMode::READ_WRITE"));
    EXPECT_THAT(asStringLiteral(AccessMode::WRITE_ONLY), StrEq("AccessMode::WRITE_ONLY"));
    EXPECT_THAT(asStringLiteral(INVALID_ACCESS_MODE), StrEq("AccessMode::UNDEFINED_VALUE"));
}

// END AccessMode and OpenMode tests


// BEGIN access_rights tests

TEST(filesystem_test, accessRightsFromValueSanitizedWorksForValueInRangeOfPermsMask)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a6c2ece-9cd7-4779-ad0b-16372aebd407");
    constexpr auto TEST_VALUE = access_rights::detail::OWNER_READ;

    EXPECT_THAT(access_rights::from_value_sanitized(TEST_VALUE).value(), Eq(TEST_VALUE));
}

TEST(filesystem_test, accessRightsFromValueSanitizedWorksForValueOutOfRangeOfPermsMask)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a291709-a75c-4afa-96d8-d57a0d40696c");
    constexpr auto TEST_VALUE_SANITIZED = access_rights::detail::OWNER_WRITE;
    constexpr auto TEST_VALUE = TEST_VALUE_SANITIZED | static_cast<access_rights::value_type>(010000);

    EXPECT_THAT(access_rights::from_value_sanitized(TEST_VALUE).value(), Eq(TEST_VALUE_SANITIZED));
}

TEST(filesystem_test, permsBinaryOrEqualToBinaryOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b72fcec-c2b3-4a45-801f-542ff3195a2f");
    constexpr access_rights TEST_VALUE_LHS = perms::others_write;
    constexpr access_rights TEST_VALUE_RHS = perms::group_all;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::OTHERS_WRITE;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::GROUP_ALL;

    EXPECT_THAT((TEST_VALUE_LHS | TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS | BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndEqualToBinaryAndOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "15a02845-21b0-41fb-80bf-ee2ff9a81427");
    constexpr access_rights TEST_VALUE_LHS = perms::others_read;
    constexpr access_rights TEST_VALUE_RHS = perms::mask;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::OTHERS_READ;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::MASK;

    EXPECT_THAT((TEST_VALUE_LHS & TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS & BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrEqualToBinaryExclusiveOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8094a263-2861-45ad-aecd-9312d477bc2d");
    constexpr access_rights TEST_VALUE_LHS = perms::set_gid;
    constexpr access_rights TEST_VALUE_RHS = perms::set_uid;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::SET_GID;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::SET_UID;

    EXPECT_THAT((TEST_VALUE_LHS ^ TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS ^ BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryComplementEqualToBinaryComplementOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "c313cf42-4cf0-4836-95ff-129111a707b0");
    constexpr access_rights TEST_VALUE = perms::owner_read;

    constexpr auto BASE_VALUE = iox::access_rights::detail::OWNER_READ;
    constexpr auto EXPECTED_VALUE = static_cast<access_rights::value_type>(~BASE_VALUE);

    ASSERT_THAT(TEST_VALUE.value(), Eq(BASE_VALUE));

    EXPECT_THAT((~TEST_VALUE).value(), Eq(EXPECTED_VALUE));
}

TEST(filesystem_test, permsBinaryOrAssignmentEqualToBinaryOrAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3611de8-f932-4485-9e64-6cd8af4526dc");
    constexpr access_rights TEST_VALUE = perms::sticky_bit;
    constexpr access_rights TEST_VALUE_RHS = perms::group_read;

    auto sutBaseValue = iox::access_rights::detail::STICKY_BIT;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::GROUP_READ;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut |= TEST_VALUE_RHS).value(), Eq(sutBaseValue |= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndAssignmentEqualToBinaryAndAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "03c139be-e3ec-477e-8598-5da93699ab75");
    constexpr access_rights TEST_VALUE = perms::others_exec;
    constexpr access_rights TEST_VALUE_RHS = perms::others_all;

    auto sutBaseValue = iox::access_rights::detail::OTHERS_EXEC;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::OTHERS_ALL;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut &= TEST_VALUE_RHS).value(), Eq(sutBaseValue &= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrAssignmentEqualToBinaryExclusiveOrAssignmentOfUnderylingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "dae75205-a635-4535-8e8d-05541bb05b60");
    constexpr access_rights TEST_VALUE = perms::none;
    constexpr access_rights TEST_VALUE_RHS = perms::owner_all;

    auto sutBaseValue = iox::access_rights::detail::NONE;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::OWNER_ALL;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut ^= TEST_VALUE_RHS).value(), Eq(sutBaseValue ^= BASE_VALUE_RHS));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenEverythingIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bb4931f-6ef9-4089-88a1-bf263a931559");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::mask;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
                Eq("owner: {read, write, execute},  group: {read, write, execute},  others: {read, write, execute},  "
                   "special bits: {set_uid, set_git, sticky_bit}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenNothingIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b50cb56-6dae-4514-bd77-791f81f6adca");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::none;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
                Eq("owner: {none},  group: {none},  others: {none},  special bits: {none}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenPartialPermissionsAreSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "94e647b7-242b-4fe3-bccd-2fde9e091e8e");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << (perms::owner_write | perms::owner_exec | perms::group_read
                                           | perms::group_exec | perms::others_all | perms::sticky_bit);
    }
    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
                Eq("owner: {write, execute},  group: {read, execute},  others: {read, write, execute},  special bits: "
                   "{sticky_bit}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenSetToUnknown)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcfd29e1-84d9-11ec-9e17-5405db3a3777");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::unknown;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, Eq("unknown permissions"));
}

// END access_rights tests

} // namespace
