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

TEST_F(Helplets_test, maxAlignment)
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

TEST_F(Helplets_test, bestFittingTypeUsesUint8WhenValueSmaller256)
{
    ::testing::Test::RecordProperty("TEST_ID", "6704aaf9-c0a4-495c-8128-15c126cbcd9b");
    EXPECT_TRUE((std::is_same<BestFittingType_t<123U>, uint8_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint8WhenValueEqualTo255)
{
    ::testing::Test::RecordProperty("TEST_ID", "10bbca50-95a7-436b-ab54-43b37cc7048f");
    EXPECT_TRUE((std::is_same<BestFittingType_t<255U>, uint8_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueEqualTo256)
{
    ::testing::Test::RecordProperty("TEST_ID", "d67306ff-c0cc-4769-9160-ef14e9f482dc");
    EXPECT_TRUE((std::is_same<BestFittingType_t<256U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueBetween256And65535)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff50f669-d9d3-454f-9994-a4dd3a19029d");
    EXPECT_TRUE((std::is_same<BestFittingType_t<8172U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint16WhenValueEqualTo65535)
{
    ::testing::Test::RecordProperty("TEST_ID", "b71d99b4-bd4e-46d6-8b22-6e796b611824");
    EXPECT_TRUE((std::is_same<BestFittingType_t<65535U>, uint16_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueEqualTo65536)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe53df8e-a797-4547-8503-0ff5850ab22e");
    EXPECT_TRUE((std::is_same<BestFittingType_t<65536U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueBetween2p16And2p32)
{
    ::testing::Test::RecordProperty("TEST_ID", "f07b1301-faf1-4945-aab0-a7af0ac967d7");
    EXPECT_TRUE((std::is_same<BestFittingType_t<81721U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueEqualTo4294967295)
{
    ::testing::Test::RecordProperty("TEST_ID", "f63335ef-c29f-49f0-bd77-ea9a548ef9fa");
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967295U>, uint32_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint64WhenValueEqualTo4294967296)
{
    ::testing::Test::RecordProperty("TEST_ID", "23f6ff5c-4cad-440c-839f-bd6cde5fa5d4");
    EXPECT_TRUE((std::is_same<BestFittingType_t<4294967296U>, uint64_t>::value));
}

TEST_F(Helplets_test, bestFittingTypeUsesUint32WhenValueGreater2p32)
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

TYPED_TEST_SUITE(Helplets_test_isPowerOfTwo, HelpletsIsPowerOfTwoTypes);


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
    ::testing::Test::RecordProperty("TEST_ID", "067ddf95-8a5c-442b-8022-ecab580b5a7d");
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
    ::testing::Test::RecordProperty("TEST_ID", "e0eecf9b-6f2f-4da2-8a18-466504348c50");
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("//")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("../")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("////")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/fu/bla/far/")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/schnappa/di/puppa//")));
}

TEST(Helplets_test_isValidFilePath, MultipleSlashsAreValidFilePath)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7621d88-d128-4239-8acc-b18f47c92b62");
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("//beginning/double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/middle//double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("middle//double/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/multi////slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("////multi/slash")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("//multi///slash////hypno")));
}

TEST(Helplets_test_isValidFilePath, RelativePathElementsAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec7d682f-ac7b-4173-a3f6-55969696ee92");
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("../some.file")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./another_file")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./dir/../../fuu-bar")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./././gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("./../.././gimme-blubb")));
}

TEST(Helplets_test_isValidFilePath, RelativePathBeginningFromRootIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "30c24356-1777-42a0-906b-73890fd19830");
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/./././gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/../../../gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/../some/dir/gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/./blubb/dir/gimme-blubb")));
}

TEST(Helplets_test_isValidFilePath, SingleFileIsValidPath)
{
    ::testing::Test::RecordProperty("TEST_ID", "264d792f-34cb-4bc0-886c-ac9de05bb1f9");
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("gimme-blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("a")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("fuu:blubb")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/blarbi")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/x")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/fuu:-012")));
}

TEST(Helplets_test_isValidFilePath, ValidPathsWithNoRelativeComponentAreValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "5556ef38-b028-4155-86c7-dda9530e8611");
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/fuu/bla/blubb/balaa")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("/a/b/c/d/1/2/4")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("asd/fuu/asdaaas/1")));
    EXPECT_TRUE(isValidFilePath(string<FILE_PATH_LENGTH>("123/456")));
}

TEST(Helplets_test_isValidFilePath, EndingWithRelativePathComponentIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3a5c3e6-840d-4ed5-8064-fede7404391d");
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
    ::testing::Test::RecordProperty("TEST_ID", "2b0dd948-49a0-4eb6-9c78-bad6e6933833");
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("a.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/asda.")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/bla/../fuu/asda..")));
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("/bla/./.././xa..")));
}

TEST(Helplets_test_isValidFilePath, PathWhichContainsAllValidCharactersIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2667afd7-f60c-4d1a-8eff-bf272c68b47a");
    EXPECT_TRUE(isValidFilePath(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/0123456789/-.:_")));
    EXPECT_TRUE(isValidFilePath(
        string<FILE_PATH_LENGTH>("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.:_")));
}

TEST(Helplets_test_isValidFilePath, WhenOneInvalidCharacterIsContainedPathIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "a764cff3-2607-47bb-952b-4ca75f326721");
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
    ::testing::Test::RecordProperty("TEST_ID", "a045581c-3a66-4d0e-b2e2-6ed5a97d4f89");
    EXPECT_FALSE(isValidFilePath(string<FILE_PATH_LENGTH>("")));
}

TEST(Helplets_test_from, fromWorksAsConstexpr)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b7cac32-c0ef-4f29-8314-59ed8850d1f5");
    constexpr A FROM_VALUE{A::A1};
    constexpr B TO_VALUE{B::B1};
    constexpr B SUT = iox::cxx::from<A, B>(FROM_VALUE);
    EXPECT_EQ(SUT, TO_VALUE);
}

TEST(Helplets_test_into, intoWorksWhenFromIsSpecialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d4331e5-f603-4e50-bdb2-75df57b0b517");
    constexpr A FROM_VALUE{A::A2};
    constexpr B TO_VALUE{B::B2};
    constexpr B SUT = iox::cxx::into<B>(FROM_VALUE);
    EXPECT_EQ(SUT, TO_VALUE);
}

} // namespace
