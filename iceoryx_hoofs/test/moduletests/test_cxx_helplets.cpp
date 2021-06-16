// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "test.hpp"

#include <array>
#include <string>
#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::cxx;
using namespace iox::cxx::internal;

namespace
{
struct Bar
{
    alignas(8) uint8_t m_dummy[73];
};
struct Foo
{
    uint8_t m_dummy[73];
};
struct FooBar
{
    alignas(32) uint8_t m_dummy[73];
};
struct FuBar
{
    alignas(32) uint8_t m_dummy[73];
};
} // namespace

class Helplets_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

bool isValidFileCharacter(const int32_t i) noexcept
{
    return ((ASCII_A <= i && i <= ASCII_Z) || (ASCII_CAPITAL_A <= i && i <= ASCII_CAPITAL_Z)
            || (ASCII_0 <= i && i <= ASCII_9) || i == ASCII_MINUS || i == ASCII_DOT || i == ASCII_COLON
            || i == ASCII_UNDERSCORE);
}

constexpr uint64_t FILE_PATH_LENGTH = 128U;

TEST_F(Helplets_test, maxSize)
{
    EXPECT_THAT(iox::cxx::maxSize<Foo>(), Eq(sizeof(Foo)));

    EXPECT_THAT(sizeof(Bar), Ne(sizeof(Foo)));
    EXPECT_THAT((iox::cxx::maxSize<Bar, Foo>()), Eq(sizeof(Bar)));

    EXPECT_THAT(sizeof(Bar), Ne(sizeof(FooBar)));
    EXPECT_THAT(sizeof(Foo), Ne(sizeof(FooBar)));
    EXPECT_THAT((iox::cxx::maxSize<Bar, Foo, FooBar>()), Eq(sizeof(FooBar)));

    EXPECT_THAT(sizeof(FooBar), Eq(sizeof(FuBar)));
    EXPECT_THAT((iox::cxx::maxSize<FooBar, FuBar>()), Eq(sizeof(FooBar)));
}

TEST_F(Helplets_test, maxAlignment)
{
    EXPECT_THAT(iox::cxx::maxAlignment<Foo>(), Eq(alignof(Foo)));

    EXPECT_THAT(alignof(Bar), Ne(alignof(Foo)));
    EXPECT_THAT((iox::cxx::maxAlignment<Bar, Foo>()), Eq(alignof(Bar)));

    EXPECT_THAT(alignof(Bar), Ne(alignof(FooBar)));
    EXPECT_THAT(alignof(Foo), Ne(alignof(FooBar)));
    EXPECT_THAT((iox::cxx::maxAlignment<Bar, Foo, FooBar>()), Eq(alignof(FooBar)));

    EXPECT_THAT(alignof(FooBar), Eq(alignof(FuBar)));
    EXPECT_THAT((iox::cxx::maxAlignment<FooBar, FuBar>()), Eq(alignof(FooBar)));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint8WhenValueSmaller256)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<123U>, uint8_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint8WhenValueEqualTo255)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<255U>, uint8_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueEqualTo256)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<256U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueBetween256And65535)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<8172U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueEqualTo65535)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<65535U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueEqualTo65536)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<65536U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueBetween2p16And2p32)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<81721U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueEqualTo4294967295)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967295U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint64WhenValueEqualTo4294967296)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967296U>, uint64_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueGreater2p32)
{
    EXPECT_TRUE((std::is_same<BestFittingType_t<42949672961U>, uint64_t>::value));
}

template <class T>
class Helplets_test_isPowerOfTwo : public Helplets_test
{
  public:
    using CurrentType = T;

    static constexpr T MAX = std::numeric_limits<T>::max();
    static constexpr T MAX_POWER_OF_TWO = MAX / 2U + 1U;
};

using HelpletsIsPowerOfTwoTypes = Types<uint8_t, uint16_t, uint32_t, uint64_t, size_t>;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(Helplets_test_isPowerOfTwo, HelpletsIsPowerOfTwoTypes);
#pragma GCC diagnostic pop

TYPED_TEST(Helplets_test_isPowerOfTwo, OneIsPowerOfTwo)
{
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(1)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, TwoIsPowerOfTwo)
{
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(2)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, FourIsPowerOfTwo)
{
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(4)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, MaxPossiblePowerOfTwoForTypeIsPowerOfTwo)
{
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(TestFixture::MAX_POWER_OF_TWO)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, ZeroIsNotPowerOfTwo)
{
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(0)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, FourtyTwoIsNotPowerOfTwo)
{
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(42)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, MaxValueForTypeIsNotPowerOfTwo)
{
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(TestFixture::MAX)));
}

TEST(Helplets_test_isValidFileName, CorrectInternalAsciiAliases)
{
    EXPECT_EQ(ASCII_A, 'a');
    EXPECT_EQ(ASCII_Z, 'z');
    EXPECT_EQ(ASCII_CAPITAL_A, 'A');
    EXPECT_EQ(ASCII_CAPITAL_Z, 'Z');
    EXPECT_EQ(ASCII_0, '0');
    EXPECT_EQ(ASCII_9, '9');
    EXPECT_EQ(ASCII_MINUS, '-');
    EXPECT_EQ(ASCII_DOT, '.');
    EXPECT_EQ(ASCII_COLON, ':');
    EXPECT_EQ(ASCII_UNDERSCORE, '_');
}

TEST(Helplets_test_isValidFileName, EmptyNameIsInvalid)
{
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_isValidFileName, RelativePathComponentsAreInvalid)
{
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(".")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("..")));
}

// this restriction ensures that we are compatible with the windows
// api which does not support dots and spaces at the end
TEST(Helplets_test_isValidFileName, DotsAndSpacesAreNotValidAtTheEnd)
{
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

TEST(Helplets_test_isValidFileName, FileNameWithValidSymbolsAndDotsAreValid)
{
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna.blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".bla.b.a.sla.a")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("...fuu...man...schmu")));
}

TEST(Helplets_test_isValidFileName, ValidLetterCombinationsAreValid)
{
    std::array<std::string, 3> combinations;

    for (int32_t i = 0; i <= 255; ++i)
    {
        // for simplicity we exclude the valid dot here, since it is
        // invalid when it occurs alone.
        // it is tested separately
        if (i != ASCII_DOT && isValidFileCharacter(i))
        {
            uint32_t index = i % 3;

            auto& s = combinations[index];
            s.append(1, static_cast<char>(i));

            EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(TruncateToCapacity, s)));
        }
    }
}

TEST(Helplets_test_isValidFileName, WhenOneInvalidCharacterIsContainedFileNameIsInvalid)
{
    std::string validName1 = "summon";
    std::string validName2 = "TheHolyToad";

    for (int32_t i = 0; i <= 255; ++i)
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

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(TruncateToCapacity, invalidCharacterFront);
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(TruncateToCapacity, invalidCharacterMiddle);
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(TruncateToCapacity, invalidCharacterEnd);

        EXPECT_FALSE(isValidFileName(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterEndTest));
    }
}

TEST(Helplets_test_isValidFilePath, StringWithEndingSlashIsNotAFilePath)
{
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("//")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("../")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("////")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/fu/bla/far/")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/schnappa/di/puppa//")));
}

TEST(Helplets_test_isValidFilePath, MultipleSlashsAreValidFilePath)
{
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("//beginning/double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/middle//double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("middle//double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/multi////slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("////multi/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("//multi///slash////hypno")));
}

TEST(Helplets_test_isValidFilePath, RelativePathElementsAreValid)
{
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("../some.file")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./another_file")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./././gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./../.././gimme-blubb")));
}

TEST(Helplets_test_isValidFilePath, RelativePathBeginningFromRootIsValid)
{
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/./././gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/../../../gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb")));
}

TEST(Helplets_test_isValidFilePath, SingleFileIsValidPath)
{
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("a")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("fuu:blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/blarbi")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/x")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/fuu:-012")));
}

TEST(Helplets_test_isValidFilePath, ValidPathsWithNoRelativeComponentAreValid)
{
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/balaa")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/4")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/1")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("123/456")));
}

TEST(Helplets_test_isValidFilePath, EndingWithRelativePathComponentIsInvalid)
{
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/..")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("./..")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("../.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("some/path/to/..")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/another/path/to/.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("../bla/fuu/../blubb/.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("./blubb/fuu/../bla/..")));
}

TEST(Helplets_test_isValidFilePath, FilePathsWithEndingDotsAreInvalid)
{
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(Helplets_test_isValidFilePath, PathWhichContainsAllValidCharactersIsValid)
{
    EXPECT_TRUE(isValidFilePath(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidFilePath(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(Helplets_test_isValidFilePath, WhenOneInvalidCharacterIsContainedPathIsInvalid)
{
    std::string validPath1 = "/hello";
    std::string validPath2 = "fuu/world";

    // begin at 1 since 0 is string termination
    for (int32_t i = 1; i <= 255; ++i)
    {
        // ignore valid characters
        if (isValidFileCharacter(i))
        {
            continue;
        }

        // ignore path separators since they are valid path characters
        bool isPathSeparator = false;
        auto numberOfPathSeparators = strlen(iox::platform::IOX_PATH_SEPARATORS);
        for (uint64_t k = 0; k < numberOfPathSeparators; ++k)
        {
            if (static_cast<char>(i) == iox::platform::IOX_PATH_SEPARATORS[k])
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

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(TruncateToCapacity, invalidCharacterFront);
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(TruncateToCapacity, invalidCharacterMiddle);
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(TruncateToCapacity, invalidCharacterEnd);

        EXPECT_FALSE(isValidFilePath(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidFilePath(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidFilePath(invalidCharacterEndTest));
    }
}

TEST(Helplets_test_isValidFilePath, EmptyFilePathIsInvalid)
{
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("")));
}


} // namespace
