// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "test.hpp"


namespace iox
{
namespace cxx
{
namespace test
{
template <typename, typename = void>
struct has_mytype_as_member : std::false_type
{
};

template <typename T>
struct has_mytype_as_member<T, void_t<typename T::myType>> : std::true_type
{
};
} // namespace test
} // namespace cxx
} // namespace iox

namespace
{
using namespace ::testing;
using namespace iox::cxx;

TEST(TypeTraitsTest, IsInvocableResolvesToTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "802f0044-ee40-47b7-9b83-519866c63508");
    auto lambda = [](int) -> void {};
    auto sut = is_invocable<decltype(lambda), int>::value;
    EXPECT_TRUE(sut);
}

TEST(TypeTraitsTest, IsInvocableResolvesToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "c862c84f-d31b-4060-9e11-3a4d850c59f2");
    int beeblebrox{42};
    auto sut = is_invocable<decltype(beeblebrox), void>::value;
    EXPECT_FALSE(sut);
}

TEST(TypeTraitsTest, IsInvocableRResolvesToTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "15f2d85e-a68f-4a3c-93bd-8b30e87903dc");
    auto lambda = [](int foo) -> int { return foo++; };
    auto sut = is_invocable_r<int, decltype(lambda), int>::value;
    EXPECT_TRUE(sut);
}

TEST(TypeTraitsTest, IsInvocableRResolvesToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae967e0c-7f55-435a-8161-bd0bc7ada6f7");
    auto lambda = [](float foo) -> float { return foo++; };
    auto sut = is_invocable_r<void, decltype(lambda), int>::value;
    EXPECT_FALSE(sut);
}

TEST(TypeTraitsTest, NoTypeAsMemberIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "244b424c-98da-4da5-a793-3bd3606acc01");
    struct Sut
    {
    };

    EXPECT_FALSE(iox::cxx::test::has_mytype_as_member<Sut>::value);
}

TEST(TypeTraitsTest, MyTypeAsMemberIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b233e3a-f9c2-4f6a-8ed4-0ace56894576");
    struct Sut
    {
        using myType = int;
    };

    EXPECT_TRUE(iox::cxx::test::has_mytype_as_member<Sut>::value);
}

TEST(TypeTraitsTest, AddConstConditionallyAddsConstIfConditionTypeIsConst)
{
    ::testing::Test::RecordProperty("TEST_ID", "021cb188-8d85-46e2-8e35-5916daf43ad3");
    using SutType = uint8_t;
    using ConditionType = bool;

    using SutTypeResult = iox::cxx::add_const_conditionally<SutType, const ConditionType>::type;

    EXPECT_TRUE(std::is_const<SutTypeResult>::value);
    // EXPECT_TRUE macro is broken when std::is_same is used directly
    auto typesIsNotChanged = std::is_same<SutType, std::remove_const_t<SutTypeResult>>::value;
    EXPECT_TRUE(typesIsNotChanged);
}

TEST(TypeTraitsTest, AddConstConditionallyDoesNotAddsConstIfConditionTypeIsNotConst)
{
    ::testing::Test::RecordProperty("TEST_ID", "01a7a26f-e988-4cd1-867b-88002623097c");
    using SutType = uint8_t;
    using ConditionType = bool;

    using SutTypeResult = iox::cxx::add_const_conditionally<SutType, ConditionType>::type;

    EXPECT_FALSE(std::is_const<SutTypeResult>::value);
    // EXPECT_TRUE macro is broken when std::is_same is used directly
    auto typesIsNotChanged = std::is_same<SutType, SutTypeResult>::value;
    EXPECT_TRUE(typesIsNotChanged);
}

TEST(TypeTraitsTest, AddConstConditionallyTypeAliasWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0034c4c7-80d1-45c0-bc02-a3e89ea13d45");
    using SutType = uint8_t;
    using ConditionType = bool;

    using SutTypeResult = iox::cxx::add_const_conditionally_t<SutType, const ConditionType>;

    EXPECT_TRUE(std::is_const<SutTypeResult>::value);
}

TEST(TypeTraitsTest, AlwaysFalseWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "41ba2959-d7ed-45fa-b2bb-467bbf7cbb38");
    struct Foo
    {
    };
    EXPECT_FALSE(always_false_v<Foo>);
}

TEST(TypeTraitsTest, IsFunctionPointerResolvesToTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2106163-92c3-4263-a706-b2f5bd17866a");
    auto result = is_function_pointer<void (*)(double)>::value;
    EXPECT_TRUE(result);

    result = is_function_pointer<int* (*)(double)>::value;
    EXPECT_TRUE(result);

    result = is_function_pointer<void (*)(int, double)>::value;
    EXPECT_TRUE(result);
}

TEST(TypeTraitsTest, IsFunctionPointerResolvesToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9801a871-f27e-4c96-831c-f826b62feac3");
    auto result = is_function_pointer<int*>::value;
    EXPECT_FALSE(result);

    result = is_function_pointer<void*>::value;
    EXPECT_FALSE(result);

    result = is_function_pointer<int>::value;
    EXPECT_FALSE(result);
}

} // namespace
