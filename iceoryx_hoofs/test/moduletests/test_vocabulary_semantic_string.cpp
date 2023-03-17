// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iox/semantic_string.hpp"
#include "iox/user_name.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

// NOLINTBEGIN(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
template <typename T>
struct TestValues;

template <>
struct TestValues<UserName>
{
    static constexpr uint64_t CAPACITY = platform::MAX_USER_NAME_LENGTH;
    static constexpr const char VALID_VALUES[][CAPACITY]{{"some-user"}, {"user2"}};
    static constexpr const char INVALID_CHARACTER_VALUES[][CAPACITY]{
        {"some-!user"}, {"*kasjd"}, {"_fuuuas"}, {"asd/asd"}, {";'1'fuuuu"}, {"argh/"}};
    static constexpr const char INVALID_CONTENT_VALUES[][CAPACITY]{
        {""}, {"-do-not-start-with-dash"}, {"5do-not-start-with-a-number"}};
    static constexpr const char TOO_LONG_CONTENT_VALUES[][CAPACITY * 2]{{"i-am-waaaaay-toooooooo-loooooooong"}};
};
constexpr const char TestValues<UserName>::VALID_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::INVALID_CHARACTER_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::INVALID_CONTENT_VALUES[][TestValues<UserName>::CAPACITY];
constexpr const char TestValues<UserName>::TOO_LONG_CONTENT_VALUES[][TestValues<UserName>::CAPACITY * 2];
// NOLINTEND(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)

template <typename T>
class SemanticString_test : public Test
{
  protected:
    using SutType = T;
};

using Implementations = Types<UserName>;

TYPED_TEST_SUITE(SemanticString_test, Implementations, );

TYPED_TEST(SemanticString_test, InitializeWithValidStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "31a2cd17-ca02-486a-b173-3f1f219d8ca3");
    using SutType = typename TestFixture::SutType;

    auto sut = SutType::create("alwaysvalid");

    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->size(), Eq(11));
    EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));
    EXPECT_TRUE(sut->as_string() == "alwaysvalid");
}

TYPED_TEST(SemanticString_test, InitializeWithValidStringValueWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "14483f4e-d556-4770-89df-84d873428eee");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::VALID_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(false));
        EXPECT_THAT(sut->size(), Eq(strnlen(value, TestValues<SutType>::CAPACITY)));
        EXPECT_THAT(sut->capacity(), Eq(TestValues<SutType>::CAPACITY));
        EXPECT_THAT(sut->as_string(), Eq(string<SutType::capacity()>(TruncateToCapacity, value)));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithStringContainingIllegalCharactersFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "14483f4e-d556-4770-89df-84d873428eee");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::INVALID_CHARACTER_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ContainsInvalidCharacters));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithStringContainingIllegalContentFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "9380f932-527f-4116-bd4f-dc8078b63330");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::INVALID_CONTENT_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity()>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ContainsInvalidContent));
    }
}

TYPED_TEST(SemanticString_test, InitializeWithTooLongContentFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5597825-c559-48e7-96f3-5136fffc55d7");
    using SutType = typename TestFixture::SutType;

    for (auto value : TestValues<SutType>::TOO_LONG_CONTENT_VALUES)
    {
        auto sut = SutType::create(string<SutType::capacity() * 2>(TruncateToCapacity, value));

        ASSERT_THAT(sut.has_error(), Eq(true));
        ASSERT_THAT(sut.get_error(), Eq(SemanticStringError::ExceedsMaximumLength));
    }
}
} // namespace
