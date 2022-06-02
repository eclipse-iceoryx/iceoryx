// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/cli/option.hpp"
#include "test.hpp"


namespace
{
using namespace ::testing;
using namespace iox::cli::internal;

template <typename T>
class OptionTest : public Test
{
  public:
    typename T::Type_t createEmpty()
    {
        return T::createEmpty();
    }
};

struct OptionFactory
{
    using Type_t = Option;
    static Type_t createEmpty()
    {
        return Type_t();
    }
};

struct OptionWithDetailsFactory
{
    using Type_t = OptionWithDetails;
    static Type_t createEmpty()
    {
        return Type_t{{}, "", iox::cli::OptionType::SWITCH, ""};
    }
};


using Implementations_t = Types<OptionFactory, OptionWithDetailsFactory>;
TYPED_TEST_SUITE(OptionTest, Implementations_t);

TYPED_TEST(OptionTest, emptyValueIsASwitch)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a599a9a-4594-4290-96a6-1281305b94bf");
    EXPECT_TRUE(this->createEmpty().isSwitch());
}

TYPED_TEST(OptionTest, nonEmptyValueIsNotASwitch)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf62d4fc-1b38-479d-bfa2-fe2764f7102c");
    auto sut = this->createEmpty();
    sut.value = iox::cli::Argument_t(iox::cxx::TruncateToCapacity, "WeLoveTheLeafSheep");
    EXPECT_FALSE(sut.isSwitch());
}

TYPED_TEST(OptionTest, emptyOptionIsDetected)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d6d45f3-4fb4-45d2-b968-a5723665d356");
    EXPECT_TRUE(this->createEmpty().isEmpty());
}

TYPED_TEST(OptionTest, optionWithLongOptionIsNotEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "03557f6a-38b6-42ab-8660-ae21aa218da3");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "TheLeafSheepWantsToBeYourFriend");
    EXPECT_FALSE(sut.isEmpty());
}

TYPED_TEST(OptionTest, optionWithShortOptionIsNotEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "9fe49b82-05e7-47fe-bd5f-286878cd4198");
    auto sut = this->createEmpty();
    sut.shortOption = 'a';
    EXPECT_FALSE(sut.isEmpty());
}

TYPED_TEST(OptionTest, optionWithShortAndLongOptionIsNotEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "b25ddf2d-e105-44e5-a313-3cc13bc9ad06");
    auto sut = this->createEmpty();
    sut.shortOption = 'b';
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "PleaseDoNotPetTheHypnotoad");
    EXPECT_FALSE(sut.isEmpty());
}

TYPED_TEST(OptionTest, emptyLongOptionDoesNotStartWithDash)
{
    ::testing::Test::RecordProperty("TEST_ID", "533a66ac-0955-4f9b-8243-02e89257a89f");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.longOptionNameDoesStartWithDash());
}

TYPED_TEST(OptionTest, longOptionWithoutDashDoesNotStartWithDash)
{
    ::testing::Test::RecordProperty("TEST_ID", "88cafc0a-2d20-4d13-9ba0-01549064cad3");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "WhyDoesDonaldDuckNeverWearsPants?");
    EXPECT_FALSE(sut.longOptionNameDoesStartWithDash());
}

TYPED_TEST(OptionTest, longOptionWithLeadingDashIsDetected)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ca0fd4f-dfc3-45f3-9ff7-0ed83acf410c");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "-minusOhNo");
    EXPECT_TRUE(sut.longOptionNameDoesStartWithDash());
}

TYPED_TEST(OptionTest, emptyShortOptionDoesNotStartWithDash)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a90616d-8cc6-4513-b67d-23ac0a676ab1");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.shortOptionNameIsEqualDash());
}

TYPED_TEST(OptionTest, shortOptionWithDashIsDetected)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c666bd8-250b-4ca1-b7a5-f633086088f5");
    auto sut = this->createEmpty();
    sut.shortOption = '-';
    EXPECT_TRUE(sut.shortOptionNameIsEqualDash());
}

TYPED_TEST(OptionTest, hasSameLongOptionNameFailsWithWhenBothAreEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8c92dbf-fef7-4766-917a-9bd9f5dece7b");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.hasLongOptionName(""));
}

TYPED_TEST(OptionTest, hasSameLongOptionNameFailsWhenBothAreDifferent)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe0d1aca-57fb-4841-b5dd-14d97a15c871");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "ChemistryIsTheArt...");
    EXPECT_FALSE(sut.hasLongOptionName("...OfTastingAPlum"));
}

TYPED_TEST(OptionTest, hasSameLongOptionNameWorksWhenBothAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "65b2adc8-c35b-422f-bf57-7816faf3b18f");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "IWouldBeMoreProductiveOnHawaii");
    EXPECT_TRUE(sut.hasLongOptionName("IWouldBeMoreProductiveOnHawaii"));
}

TYPED_TEST(OptionTest, hasSameShortOptionNameFailsWhenBothAreEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "91c0c062-71c5-4e32-9545-ad93e7dd0f66");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.hasShortOptionName(iox::cli::NO_SHORT_OPTION));
}

TYPED_TEST(OptionTest, hasSameShortOptionNameFailsWhenBothAreDifferent)
{
    ::testing::Test::RecordProperty("TEST_ID", "767ad490-c3b5-4ec3-b7fa-73da203dac96");
    auto sut = this->createEmpty();
    sut.shortOption = 'x';
    EXPECT_FALSE(sut.hasShortOptionName('9'));
}

TYPED_TEST(OptionTest, hasSameShortOptionNameWorksWhenBothAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "498702db-7603-4051-b150-e4b0155faa79");
    auto sut = this->createEmpty();
    sut.shortOption = '3';
    EXPECT_TRUE(sut.hasShortOptionName('3'));
}

TYPED_TEST(OptionTest, hasOptionNameFailsWhenBothAreEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "37396c79-683b-4923-a133-a678f03c484c");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.hasOptionName(""));
}

TYPED_TEST(OptionTest, hasOptionNameWorksWhenEqualToLongOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "b55c180c-bb2d-470c-91bf-a1c0c06c3aab");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "AskYourselfWhatWouldHypnotoadDo");
    EXPECT_TRUE(sut.hasOptionName("AskYourselfWhatWouldHypnotoadDo"));
}

TYPED_TEST(OptionTest, hasOptionNameWorksWhenEqualToShortOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc6e5c05-87d3-406a-b27b-16e9fe9ea73e");
    auto sut = this->createEmpty();
    sut.shortOption = 'j';
    EXPECT_TRUE(sut.hasOptionName("j"));
}

TYPED_TEST(OptionTest, sameShortAndLongOptionsWithDifferentValueAreTheSameOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5a14fc2-d1af-49ee-8fbc-a5850361cf4e");
    auto sut = this->createEmpty();
    sut.shortOption = 'k';
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "IHateMeetings");
    sut.value = "bla";

    auto sut2 = this->createEmpty();
    sut2.shortOption = sut.shortOption;
    sut2.longOption = sut.longOption;
    sut2.value = "WhoCaresAboutLifetime";

    EXPECT_TRUE(sut.isSameOption(sut2));
    EXPECT_TRUE(sut.isSameOption(sut));
}

TYPED_TEST(OptionTest, sameShortOptionDifferentLongOptionAreNotTheSameOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b30a170-65cd-4c83-acca-28531acc7167");
    auto sut = this->createEmpty();
    sut.shortOption = 'k';
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "BlueberrysAreNice");
    sut.value = "bla";

    auto sut2 = this->createEmpty();
    sut2.shortOption = sut.shortOption;
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "ButWhatAboutTheSwedishWhitebeam");
    sut2.value = "WhoCaresAboutOwnership";

    EXPECT_FALSE(sut.isSameOption(sut2));
}

TYPED_TEST(OptionTest, sameLongOptionDifferentShortOptionAreNotTheSameOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "e158eb85-ed58-45e3-bf35-e64c4a4e83ea");
    auto sut = this->createEmpty();
    sut.shortOption = 'k';
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "ArnoldSchwarzeneggerIsMozart");
    sut.value = "bla";

    auto sut2 = this->createEmpty();
    sut2.shortOption = 'c';
    sut.longOption = sut2.longOption;
    sut2.value = "LookOverThereAThreeHeadedMonkey";

    EXPECT_FALSE(sut.isSameOption(sut2));
}

TYPED_TEST(OptionTest, emptyOptionHasNoShortOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ea16b12-8d8c-43a9-b09b-fe9cc3094cc8");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.hasShortOption());
}

TYPED_TEST(OptionTest, setupShortOptionHasShortOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5e4dd07-a63e-4aae-90dd-a1c9ee1145db");
    auto sut = this->createEmpty();
    sut.shortOption = 'p';
    EXPECT_TRUE(sut.hasShortOption());
}

TYPED_TEST(OptionTest, emptyOptionHasNoLongOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "55b459dd-bdc9-41db-8b1f-55ffb0b94bee");
    auto sut = this->createEmpty();
    EXPECT_FALSE(sut.hasLongOption());
}

TYPED_TEST(OptionTest, setupLongOptionHasLongOption)
{
    ::testing::Test::RecordProperty("TEST_ID", "02b4a288-9e52-4668-bb16-0a87706c5095");
    auto sut = this->createEmpty();
    sut.longOption = iox::cli::OptionName_t(iox::cxx::TruncateToCapacity, "MozartHadASon");
    EXPECT_TRUE(sut.hasLongOption());
}

} // namespace