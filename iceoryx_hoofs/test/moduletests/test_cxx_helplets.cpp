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

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "test.hpp"

#include <array>
#include <type_traits>

namespace
{
enum class A
{
    A1 = 13,
    A2
};

enum class B
{
    B1 = 42,
    B2
};
} // namespace

namespace iox
{
namespace cxx
{
template <>
constexpr B from<A, B>(A e) noexcept
{
    switch (e)
    {
    case A::A1:
        return B::B1;
    case A::A2:
        return B::B2;
    }
}

} // namespace cxx
} // namespace iox

namespace
{
using namespace ::testing;
using namespace iox::cxx;
using namespace iox::cxx::internal;

namespace
{
struct Bar
{
    // required for testing, a struct with a defined size
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    alignas(8) uint8_t m_dummy[73];
};
struct Foo
{
    // required for testing, a struct with a defined size
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    uint8_t m_dummy[73];
};
struct FooBar
{
    // required for testing, a struct with a defined size
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    alignas(32) uint8_t m_dummy[73];
};
struct FuBar
{
    // required for testing, a struct with a defined size
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
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

TEST_F(Helplets_test, MaxSizeWorksAsExpected)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b3e938d-aec5-478d-b1c1-49ff2cc4e3ef");
    EXPECT_THAT(iox::cxx::maxSize<Foo>(), Eq(sizeof(Foo)));

    EXPECT_THAT(sizeof(Bar), Ne(sizeof(Foo)));
    EXPECT_THAT((iox::cxx::maxSize<Bar, Foo>()), Eq(sizeof(Bar)));

    EXPECT_THAT(sizeof(Bar), Ne(sizeof(FooBar)));
    EXPECT_THAT(sizeof(Foo), Ne(sizeof(FooBar)));
    EXPECT_THAT((iox::cxx::maxSize<Bar, Foo, FooBar>()), Eq(sizeof(FooBar)));

    EXPECT_THAT(sizeof(FooBar), Eq(sizeof(FuBar)));
    EXPECT_THAT((iox::cxx::maxSize<FooBar, FuBar>()), Eq(sizeof(FooBar)));
}

TEST_F(Helplets_test, MaxAlignmentWorksAsExpected)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d5d3de1-f22c-47c1-b7fd-cacc35eef13c");
    EXPECT_THAT(iox::cxx::maxAlignment<Foo>(), Eq(alignof(Foo)));

    EXPECT_THAT(alignof(Bar), Ne(alignof(Foo)));
    EXPECT_THAT((iox::cxx::maxAlignment<Bar, Foo>()), Eq(alignof(Bar)));

    EXPECT_THAT(alignof(Bar), Ne(alignof(FooBar)));
    EXPECT_THAT(alignof(Foo), Ne(alignof(FooBar)));
    EXPECT_THAT((iox::cxx::maxAlignment<Bar, Foo, FooBar>()), Eq(alignof(FooBar)));

    EXPECT_THAT(alignof(FooBar), Eq(alignof(FuBar)));
    EXPECT_THAT((iox::cxx::maxAlignment<FooBar, FuBar>()), Eq(alignof(FooBar)));
}

TEST_F(Helplets_test, ArrayCapacityReturnsCorrectValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "8392b2ba-04ef-45e6-8b47-4c0c90d98f61");
    constexpr uint64_t CAPACITY{42};
    // NOLINTJUSTIFICATION Used only for test purposes
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    constexpr uint32_t SUT[CAPACITY]{};

    EXPECT_EQ(iox::cxx::arrayCapacity(SUT), CAPACITY);
}

TEST_F(Helplets_test, BestFittingTypeUsesUint8WhenValueSmaller256)
{
    ::testing::Test::RecordProperty("TEST_ID", "6704aaf9-c0a4-495c-8128-15c126cbcd9b");
    EXPECT_TRUE((std::is_same<BestFittingType_t<123U>, uint8_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint8WhenValueEqualTo255)
{
    ::testing::Test::RecordProperty("TEST_ID", "10bbca50-95a7-436b-ab54-43b37cc7048f");
    EXPECT_TRUE((std::is_same<BestFittingType_t<255U>, uint8_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint16WhenValueEqualTo256)
{
    ::testing::Test::RecordProperty("TEST_ID", "d67306ff-c0cc-4769-9160-ef14e9f482dc");
    EXPECT_TRUE((std::is_same<BestFittingType_t<256U>, uint16_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint16WhenValueBetween256And65535)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff50f669-d9d3-454f-9994-a4dd3a19029d");
    EXPECT_TRUE((std::is_same<BestFittingType_t<8172U>, uint16_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint16WhenValueEqualTo65535)
{
    ::testing::Test::RecordProperty("TEST_ID", "b71d99b4-bd4e-46d6-8b22-6e796b611824");
    EXPECT_TRUE((std::is_same<BestFittingType_t<65535U>, uint16_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint32WhenValueEqualTo65536)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe53df8e-a797-4547-8503-0ff5850ab22e");
    EXPECT_TRUE((std::is_same<BestFittingType_t<65536U>, uint32_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint32WhenValueBetween2p16And2p32)
{
    ::testing::Test::RecordProperty("TEST_ID", "f07b1301-faf1-4945-aab0-a7af0ac967d7");
    EXPECT_TRUE((std::is_same<BestFittingType_t<81721U>, uint32_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint32WhenValueEqualTo4294967295)
{
    ::testing::Test::RecordProperty("TEST_ID", "f63335ef-c29f-49f0-bd77-ea9a548ef9fa");
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967295U>, uint32_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint64WhenValueEqualTo4294967296)
{
    ::testing::Test::RecordProperty("TEST_ID", "23f6ff5c-4cad-440c-839f-bd6cde5fa5d4");
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967296U>, uint64_t>::value));
}

TEST_F(Helplets_test, BestFittingTypeUsesUint32WhenValueGreater2p32)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fddfb4c-0efb-4b21-9b15-8f49af779f84");
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

TYPED_TEST_SUITE(Helplets_test_isPowerOfTwo, HelpletsIsPowerOfTwoTypes, );

TYPED_TEST(Helplets_test_isPowerOfTwo, OneIsPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "c85e1998-436c-4789-95c5-895fe7b2edf0");
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(1)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, TwoIsPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d314d4b-1206-4779-9035-fa544cfee798");
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(2)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, FourIsPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb2ad241-4515-4bfb-8078-157ed8c0e18d");
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(4)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, MaxPossiblePowerOfTwoForTypeIsPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "b92311dd-aa33-489d-8544-6054028c35a4");
    EXPECT_TRUE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(TestFixture::MAX_POWER_OF_TWO)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, ZeroIsNotPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a8295cd-664d-4b1f-8a20-ac814c7f75c5");
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(0)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, FourtyTwoIsNotPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "0570fc10-eb72-4a34-b8a6-5084c7737866");
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(42)));
}

TYPED_TEST(Helplets_test_isPowerOfTwo, MaxValueForTypeIsNotPowerOfTwo)
{
    ::testing::Test::RecordProperty("TEST_ID", "2abdb27d-58de-4e3d-b8fb-8e5f1f3e6327");
    EXPECT_FALSE(isPowerOfTwo(static_cast<typename TestFixture::CurrentType>(TestFixture::MAX)));
}

TEST(Helplets_test_isValidFileName, CorrectInternalAsciiAliases)
{
    ::testing::Test::RecordProperty("TEST_ID", "e729a0a1-e3c4-4d97-a948-d88017f6ac1e");
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
    ::testing::Test::RecordProperty("TEST_ID", "b2b7aa63-c67e-4915-a906-e3b4779ab772");
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_isValidFileName, RelativePathComponentsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b33b4534-f134-499f-ac72-65a3fecaef12");
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>(".")));
    EXPECT_FALSE(isValidFileName(string<FILE_PATH_LENGTH>("..")));
}

// this restriction ensures that we are compatible with the windows
// api which does not support dots and spaces at the end
TEST(Helplets_test_isValidFileName, DotsAndSpacesAreNotValidAtTheEnd)
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

TEST(Helplets_test_isValidFileName, FileNameWithValidSymbolsAndDotsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "1455491c-1fc3-4843-a72b-2f51f8f2fadc");
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna..bla")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("scna.blubb")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(".bla.b.a.sla.a")));
    EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>("...fuu...man...schmu")));
}

TEST(Helplets_test_isValidFileName, ValidLetterCombinationsAreValid)
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

            EXPECT_TRUE(isValidFileName(string<FILE_PATH_LENGTH>(TruncateToCapacity, s)));
        }
    }
}

TEST(Helplets_test_isValidFileName, WhenOneInvalidCharacterIsContainedFileNameIsInvalid)
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

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(TruncateToCapacity, invalidCharacterFront);
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(TruncateToCapacity, invalidCharacterMiddle);
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(TruncateToCapacity, invalidCharacterEnd);

        EXPECT_FALSE(isValidFileName(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidFileName(invalidCharacterEndTest));
    }
}

TEST(Helplets_test_isValidPathToFile, StringWithEndingSlashIsNotAFilePath)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0eecf9b-6f2f-4da2-8a18-466504348c50");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("//")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("../")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("////")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fu/bla/far/")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/schnappa/di/puppa//")));
}

TEST(Helplets_test_isValidPathToFile, MultipleSlashsAreValidFilePath)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7621d88-d128-4239-8acc-b18f47c92b62");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("//beginning/double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/middle//double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("middle//double/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/multi////slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("////multi/slash")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("//multi///slash////hypno")));
}

TEST(Helplets_test_isValidPathToFile, RelativePathComponentsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec7d682f-ac7b-4173-a3f6-55969696ee92");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("../some.file")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./another_file")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("./../.././gimme-blubb")));
}

TEST(Helplets_test_isValidPathToFile, RelativePathBeginningFromRootIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "30c24356-1777-42a0-906b-73890fd19830");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/./././gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/../../../gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb")));
}

TEST(Helplets_test_isValidPathToFile, SingleFileIsValidPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "264d792f-34cb-4bc0-886c-ac9de05bb1f9");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("gimme-blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("a")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("fuu:blubb")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/blarbi")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/x")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fuu:-012")));
}

TEST(Helplets_test_isValidPathToFile, ValidPathsWithNoRelativeComponentAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "5556ef38-b028-4155-86c7-dda9530e8611");
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/balaa")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/4")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/1")));
    EXPECT_TRUE(isValidPathToFile(string<FILE_PATH_LENGTH>("123/456")));
}

TEST(Helplets_test_isValidPathToFile, EndingWithRelativePathComponentIsInvalid)
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

TEST(Helplets_test_isValidPathToFile, FilePathsWithEndingDotsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b0dd948-49a0-4eb6-9c78-bad6e6933833");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(Helplets_test_isValidPathToFile, PathWhichContainsAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2667afd7-f60c-4d1a-8eff-bf272c68b47a");
    EXPECT_TRUE(isValidPathToFile(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidPathToFile(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(Helplets_test_isValidPathToFile, EmptyFilePathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "a045581c-3a66-4d0e-b2e2-6ed5a97d4f89");
    EXPECT_FALSE(isValidPathToFile(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_isValidPathToFile_isValidPathToDirectory_isValidPathEntry,
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

        string<FILE_PATH_LENGTH> invalidCharacterFrontTest(TruncateToCapacity, invalidCharacterFront);
        string<FILE_PATH_LENGTH> invalidCharacterMiddleTest(TruncateToCapacity, invalidCharacterMiddle);
        string<FILE_PATH_LENGTH> invalidCharacterEndTest(TruncateToCapacity, invalidCharacterEnd);

        EXPECT_FALSE(isValidPathToFile(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidPathToFile(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidPathToFile(invalidCharacterEndTest));

        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterFrontTest));
        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterMiddleTest));
        EXPECT_FALSE(isValidPathToDirectory(invalidCharacterEndTest));

        EXPECT_FALSE(isValidPathEntry(invalidCharacterFrontTest, iox::cxx::RelativePathComponents::ACCEPT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterMiddleTest, iox::cxx::RelativePathComponents::ACCEPT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterEndTest, iox::cxx::RelativePathComponents::ACCEPT));

        EXPECT_FALSE(isValidPathEntry(invalidCharacterFrontTest, iox::cxx::RelativePathComponents::REJECT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterMiddleTest, iox::cxx::RelativePathComponents::REJECT));
        EXPECT_FALSE(isValidPathEntry(invalidCharacterEndTest, iox::cxx::RelativePathComponents::REJECT));
    }
}

TEST(Helplets_test_isValidPathToDirectory, MultipleSlashsAreValidPath)
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

TEST(Helplets_test_isValidPathToDirectory, RelativePathComponentsAreValid)
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

TEST(Helplets_test_isValidPathToDirectory, RelativePathBeginningFromRootIsValid)
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

TEST(Helplets_test_isValidPathToDirectory, SingleEntryIsValidPath)
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

TEST(Helplets_test_isValidPathToDirectory, ValidPathsWithNoRelativeComponentAreValid)
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

TEST(Helplets_test_isValidPathToDirectory, EndingWithRelativePathComponentIsValid)
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

TEST(Helplets_test_isValidPathToDirectory, PathsWithEndingDotsAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "f79660e6-12b5-4ad0-bc26-766da34898b8");
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(Helplets_test_isValidPathToDirectory, PathWhichContainsAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "8052b601-c9ad-4cb8-9a87-c301f213d8c4");
    EXPECT_TRUE(isValidPathToDirectory(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidPathToDirectory(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(Helplets_test_isValidPathToDirectory, EmptyPathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "9724b52e-2e5a-425f-853d-a0b43e553f8b");
    EXPECT_FALSE(isValidPathToDirectory(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_doesEndWithPathSeparator, EmptyPathDoesNotEndWithPathSeparator)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe0be1e0-fdd5-4d56-841c-83826c40c3d2");
    EXPECT_FALSE(doesEndWithPathSeparator(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_doesEndWithPathSeparator, NonEmptyPathWithNoPathSeparatorAtTheEndDoesNotEndWithPathSeparator)
{
    ::testing::Test::RecordProperty("TEST_ID", "a6d10202-aea0-4b1c-b9d9-704545102a2e");

    string<FILE_PATH_LENGTH> sut = "isThereOnlyOneHypnotoad";
    EXPECT_FALSE(doesEndWithPathSeparator(sut));

    sut.unsafe_append(iox::platform::IOX_PATH_SEPARATORS);
    sut.unsafe_append("thereIsOnlyOne");
    EXPECT_FALSE(doesEndWithPathSeparator(sut));
}

TEST(Helplets_test_doesEndWithPathSeparator, SingleCharacterStringOnlyWithPathSeparatorAsOneAtTheEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "18bf45aa-9b65-4351-956a-8ddc98fa0296");

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        string<FILE_PATH_LENGTH> sut = " ";
        sut[0] = separator;
        EXPECT_TRUE(doesEndWithPathSeparator(sut));
    }
}

TEST(Helplets_test_doesEndWithPathSeparator, MultiCharacterStringEndingWithPathSeparatorAsOneAtTheEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "c702ec34-8f7f-4220-b50e-6b231ac4e736");

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        string<FILE_PATH_LENGTH> sut = "HypnotoadAteTheSpagettiMonster";
        ASSERT_TRUE(sut.unsafe_append(separator));
        EXPECT_TRUE(doesEndWithPathSeparator(sut));
    }
}

TEST(Helplets_test_isValidPathEntry, EmptyPathEntryIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "1280b360-f26c-4ddf-8305-e01a99d58178");
    EXPECT_TRUE(
        isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(""), iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, PathEntryWithOnlyValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "166fb334-05c6-4b8c-a117-223d6cadb29b");
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("a"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("agc"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("a.213jkgc"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, RelativePathEntriesAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3432692-7cee-416a-a3f3-c246a02ad1a2");
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("."),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".."),
                                 iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, EntriesWithEndingDotAreInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "f937de46-19fc-48da-bce6-51292cd9d75e");
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("abc."),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("19283912asdb.."),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..19283912asdb.."),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..192839.12a.sdb.."),
                                  iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, EntriesWithDotsNotAtTheEndAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "569aa328-2c47-418d-96e2-ddf73925e52f");
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".abc"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(".19283912asdb"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..19283912asdb"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("..192839.12a.sdb"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, StringContainingAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2c19516-e8fb-4fb8-a366-2b7b5fd9a84b");
    EXPECT_TRUE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>(
                                     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_"),
                                 iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, StringWithSlashIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1119db1-f897-48a5-af92-9a92eb3f9832");
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("/fuuuu/"),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("fuu/uu"),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("/fuuuu"),
                                  iox::cxx::RelativePathComponents::ACCEPT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("uuuubbuu/"),
                                  iox::cxx::RelativePathComponents::ACCEPT));
}

TEST(Helplets_test_isValidPathEntry, StringWithRelativeComponentsIsInvalidWhenItContainsRelativeComponents)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c73e08e-3b42-446e-b8d4-a4ed7685f28e");
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../to/be"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../../or/not"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/../be"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("that/../../is/the/question"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("whether/tis/nobler/.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("in/the/mind/to/suffer//../.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../the/slings/and/arrows/../.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("../of/../outrageous/fortune/../.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./or/to/take/../arms/../.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./agains/a/see/./of/troubles/../.."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./and/by/../opposing/./."),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("./end/them"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/./die"),
                                  iox::cxx::RelativePathComponents::REJECT));
    EXPECT_FALSE(isValidPathEntry(string<iox::platform::IOX_MAX_FILENAME_LENGTH>("to/./sleep/."),
                                  iox::cxx::RelativePathComponents::REJECT));
}


TEST(Helplets_test_from, FromWorksAsConstexpr)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b7cac32-c0ef-4f29-8314-59ed8850d1f5");
    constexpr A FROM_VALUE{A::A1};
    constexpr B TO_VALUE{B::B1};
    constexpr B SUT = iox::cxx::from<A, B>(FROM_VALUE);
    EXPECT_EQ(SUT, TO_VALUE);
}

TEST(Helplets_test_into, IntoWorksWhenFromIsSpecialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d4331e5-f603-4e50-bdb2-75df57b0b517");
    constexpr A FROM_VALUE{A::A2};
    constexpr B TO_VALUE{B::B2};
    constexpr B SUT = iox::cxx::into<B>(FROM_VALUE);
    EXPECT_EQ(SUT, TO_VALUE);
}

} // namespace
