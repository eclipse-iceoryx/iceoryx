// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace ::iox::cxx;

namespace
{
class MockCallables
{
  public:
    MockCallables() = default;
    MOCK_METHOD0(onSuccess, void());
    MOCK_METHOD0(onEmpty, void());
    MOCK_METHOD0(onError, void());
};


struct TestClass
{
    TestClass(int a, int b)
        : m_a(a)
        , m_b(b)
    {
    }

    int gimme()
    {
        return m_a + m_b;
    }

    int constGimme() const
    {
        return m_a + m_b;
    }

    bool operator==(const TestClass& rhs) const
    {
        return (m_a == rhs.m_a) && (m_b == rhs.m_b);
    }

    int m_a;
    int m_b;
};

struct NonTrivialTestClass
{
    NonTrivialTestClass(int a, int b)
        : m_a(a)
        , m_b(b)
    {
    }

    NonTrivialTestClass(const NonTrivialTestClass& other)
    {
        *this = other;
    }
    NonTrivialTestClass(NonTrivialTestClass&& other)
    {
        *this = std::move(other);
    }

    NonTrivialTestClass& operator=(const NonTrivialTestClass& rhs)
    {
        if (this != &rhs)
        {
            m_a = rhs.m_a;
            m_b = rhs.m_b;
            m_moved = rhs.m_moved;
        }
        return *this;
    }

    NonTrivialTestClass& operator=(NonTrivialTestClass&& rhs)
    {
        if (this != &rhs)
        {
            m_a = rhs.m_a;
            m_b = rhs.m_b;
            m_moved = rhs.m_moved;
            rhs.m_moved = true;
        }
        return *this;
    }

    int m_a{0};
    int m_b{0};
    bool m_moved{false};
};

class expected_test : public Test
{
};

enum class TestError : uint8_t
{
    ERROR1,
    ERROR2,
    ERROR3
};

TEST_F(expected_test, CreateWithPODTypeIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b91db8c-5d2e-44a4-8cac-4ee436b5fe8e");
    auto sut = expected<int, TestError>::create_value(123);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(123));
}

TEST_F(expected_test, CreateWithErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2d10c89-6fc8-4c08-9e2d-9f61988ebb3f");
    auto sut = expected<int, TestError>::create_error(TestError::ERROR1);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR1));
}

TEST_F(expected_test, ErrorTypeOnlyConstCreateWithErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "581447a6-0705-494b-8159-cf3434080a06");
    const auto sut = expected<TestError>::create_error(TestError::ERROR2);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR2));
}

TEST_F(expected_test, ErrorTypeOnlyCreateWithErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b01b2217-e67a-4bbf-b1a8-95d9b348d66e");
    auto sut = expected<TestError>::create_error(TestError::ERROR1);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR1));
}

TEST_F(expected_test, CreateFromConstErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e4324ad-f221-4038-91ad-61a1567545dd");
    auto constError = error<TestError>(TestError::ERROR3);
    auto sut = expected<int, TestError>(constError);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR3));
}

TEST_F(expected_test, ErrorTypeOnlyCreateFromConstErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7c3fdd5-7384-4173-85a3-e3127261baa7");
    auto constError = error<TestError>(TestError::ERROR1);
    auto sut = expected<TestError>(constError);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR1));
}

TEST_F(expected_test, CreateFromConstSuccessResultsInCorrectValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb20f217-6617-4c9e-8185-35cbf2bb8f3e");
    auto constSuccess = success<int>(424242);
    auto sut = expected<int, TestError>(constSuccess);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(424242));
}

TEST_F(expected_test, CreateWithComplexTypeIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "508a39f7-905a-4d9a-a61b-43145e546eca");
    auto sut = expected<TestClass, TestError>::create_value(12, 222);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value().m_a, Eq(12));
}

TEST_F(expected_test, CreateWithSTLTypeIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "24fddc69-64ca-4b69-baab-a58293657cac");
    auto sut = expected<int, std::string>::create_error("RedAlert");
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq("RedAlert"));
}

TEST_F(expected_test, CreateWithComplexErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "71e6ea31-d6e3-42a0-a63d-4bbd39c7341c");
    auto sut = expected<int, TestClass>::create_error(313, 212);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error().m_b, Eq(212));
}

TEST_F(expected_test, CreateRValueAndGetErrorResultsInCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b032400a-cd08-4ae7-af0c-5ae0362b4dc0");
    auto sut = expected<int, TestClass>::create_error(131, 121).get_error();
    EXPECT_THAT(sut.m_b, Eq(121));
}

TEST_F(expected_test, ConstCreateLValueAndGetErrorResultsInCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "e56063ea-8b7c-4d47-a898-fe609ea3b283");
    const auto& sut = expected<int, TestClass>::create_error(343, 232);
    EXPECT_THAT(sut.get_error().m_b, Eq(232));
}

TEST_F(expected_test, CreateWithValueAndMoveCtorLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "8da72983-3046-4dde-8de5-5eed89de0ccf");
    constexpr int A{177};
    constexpr int B{188};
    auto sutSource = expected<NonTrivialTestClass, int>::create_value(A, B);
    auto sutDestination{std::move(sutSource)};

    ASSERT_FALSE(sutSource.has_error());
    EXPECT_TRUE(sutSource.value().m_moved);
    ASSERT_FALSE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.value().m_moved);
    EXPECT_EQ(sutDestination.value().m_a, A);
    EXPECT_EQ(sutDestination.value().m_b, B);
}

TEST_F(expected_test, CreateWithErrorAndMoveCtorLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7784813-458b-40f3-b6db-01521e57175e");
    constexpr int A{22};
    constexpr int B{33};
    auto sutSource = expected<int, NonTrivialTestClass>::create_error(A, B);
    auto sutDestination{std::move(sutSource)};

    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, CreateWithValueAndMoveAssignmentLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb5f326b-8446-4914-bdca-8d6ba20103fe");
    constexpr int A{73};
    constexpr int B{37};
    auto sutSource = expected<NonTrivialTestClass, int>::create_value(A, B);
    auto sutDestination = std::move(sutSource);

    ASSERT_FALSE(sutSource.has_error());
    EXPECT_TRUE(sutSource.value().m_moved);
    ASSERT_FALSE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.value().m_moved);
    EXPECT_EQ(sutDestination.value().m_a, A);
    EXPECT_EQ(sutDestination.value().m_b, B);
}

TEST_F(expected_test, CreateWithErrorAndMoveAssignmentLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef2a799d-982e-447d-8f93-f7ad63c091e0");
    constexpr int A{44};
    constexpr int B{55};
    auto sutSource = expected<int, NonTrivialTestClass>::create_error(A, B);
    auto sutDestination = std::move(sutSource);

    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, BoolOperatorReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1e30651-a0e9-4c73-b2bf-57f36fc7eddf");
    auto sut = expected<int, TestClass>::create_error(123, 321);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    EXPECT_THAT(sut.get_error().m_b, Eq(321));
}

TEST_F(expected_test, BoolOperatorReturnsNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "aec3e2a3-b7ae-4778-ac1d-d52e64b9b2d3");
    auto sut = expected<TestClass, TestError>::create_value(123, 321);

    ASSERT_THAT(sut.operator bool(), Eq(true));
    EXPECT_THAT(sut.value().m_a, Eq(123));
}

TEST_F(expected_test, ErrorTypeOnlyBoolOperatorReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "7949f68f-c21c-43f1-ad8d-dc51eeee3257");
    auto sut = expected<TestError>::create_error(TestError::ERROR1);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR1));
}

TEST_F(expected_test, ErrorTypeOnlyBoolOperatorReturnsNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "4585b1bf-cd6f-44ac-8409-75dc14fa252a");
    auto sut = expected<TestError>::create_value();
    ASSERT_THAT(sut.operator bool(), Eq(true));
}

TEST_F(expected_test, ValueOrWithErrorReturnsGivenValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "490ddf23-be03-4433-bf6a-43ccae5cde73");
    auto sut = expected<int, TestError>::create_error(TestError::ERROR1);
    EXPECT_THAT(sut.value_or(90), Eq(90));
}

TEST_F(expected_test, ConstValueOrWithErrorReturnsGivenValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5714512-7f75-4b0e-a6ac-fcff09e6a60f");
    const auto sut = expected<int, TestError>::create_error(TestError::ERROR1);
    EXPECT_THAT(sut.value_or(51), Eq(51));
}

TEST_F(expected_test, ValueOrWithSuccessReturnsStoredValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "12e6bf3a-4e99-444f-bf8c-641737a2ee03");
    auto sut = expected<int, TestError>::create_value(999);
    EXPECT_THAT(sut.value_or(15), Eq(999));
}

TEST_F(expected_test, ConstValueOrWithSuccessReturnsStoredValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3fe6982f-64a5-4245-adc9-22c2c9b0f0fe");
    const auto sut = expected<int, TestError>::create_value(652);
    EXPECT_THAT(sut.value_or(15), Eq(652));
}

TEST_F(expected_test, ArrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "39898e81-d4ad-4f27-8c45-d29c80114be2");
    auto sut = expected<TestClass, TestError>::create_value(55, 81);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->gimme(), Eq(136));
}

TEST_F(expected_test, ConstArrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b35a05e9-6dbc-4cfb-94c2-85ca9d214bb4");
    const expected<TestClass, TestError> sut(success<TestClass>(TestClass(55, 81)));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->constGimme(), Eq(136));
}

TEST_F(expected_test, DereferencingOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "11ddbd46-3a2f-43cd-a2d2-ebe2ad4019db");
    auto sut = expected<int, TestError>::create_value(1652);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(1652));
}

TEST_F(expected_test, ConstDereferencingOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f09b9476-a4f6-4f56-9692-3c00146410fd");
    const expected<int, TestError> sut(success<int>(981));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(981));
}

TEST_F(expected_test, ErrorTypeOnlyCreateValueWithoutValueLeadsToValidSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "5baee3cb-4f81-4245-b9f9-d733d14d6d4a");
    auto sut = expected<TestError>::create_value();
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, ErrorTypeOnlyCreateErrorLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7919fef-e127-4b12-86cb-603457688675");
    auto sut = expected<TestError>::create_error(TestError::ERROR2);
    ASSERT_THAT(sut.has_error(), Eq(true));
    ASSERT_THAT(sut.get_error(), Eq(TestError::ERROR2));
}

TEST_F(expected_test, ErrorTypeOnlyCreateValueWithoutValueMoveCtorLeadsToNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b7feb2c-c0bd-4c10-bc0c-d980eec4f0ca");
    auto sutSource = expected<NonTrivialTestClass>::create_value();
    auto sutDestination{std::move(sutSource)};
    EXPECT_FALSE(sutSource.has_error());
    EXPECT_FALSE(sutDestination.has_error());
}

TEST_F(expected_test, ErrorTypeOnlyCreateValueWithoutValueMoveAssignmentLeadsToNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "75d3f30e-d927-46bf-83a4-fb8361542333");
    auto sutSource = expected<NonTrivialTestClass>::create_value();
    auto sutDestination = std::move(sutSource);
    EXPECT_FALSE(sutSource.has_error());
    EXPECT_FALSE(sutDestination.has_error());
}

TEST_F(expected_test, ErrorTypeOnlyMoveCtorLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "4662a154-7cf6-498d-b6a1-08182037fbc9");
    constexpr int A{111};
    constexpr int B{112};
    auto sutSource = expected<NonTrivialTestClass>::create_error(A, B);
    auto sutDestination{std::move(sutSource)};

    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, ErrorTypeOnlyMoveAssignmentLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "117bc7f6-c3d4-4fbb-9af3-9057742f2d2e");
    constexpr int A{222};
    constexpr int B{223};
    auto sutSource = expected<NonTrivialTestClass>::create_error(A, B);
    auto sutDestination = std::move(sutSource);

    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, CreateFromEmptySuccessTypeLeadsToValidSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "0204f08f-fb6d-45bb-aac7-fd14152ab1bf");
    expected<TestError> sut{success<>()};
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, CreateFromSuccessTypeLeadsToValidSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb83b62e-4e17-480b-8425-72181e6dd55d");
    expected<int, TestError> sut{success<int>(55)};
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(55));
}

TEST_F(expected_test, CreateFromErrorConstLeadsToCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b69f337-7994-40f8-aad7-7b6febe8b254");
    const TestError f = TestError::ERROR1;
    expected<TestError> sut{error<TestError>(f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR1));
}

TEST_F(expected_test, ErrorTypeOnlyCreateFromErrorLeadsToCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c55e8a2-8da3-43bd-858a-b9bd19d71b1f");
    expected<TestError> sut{error<TestError>(TestError::ERROR2)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR2));
}

TEST_F(expected_test, CreateFromErrorLeadsToCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb7e783d-0a79-45ce-9ea7-3b6e28631ceb");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR2)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(TestError::ERROR2));
}

TEST_F(expected_test, WhenHavingAnErrorCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "68d359ad-b1be-42ac-b1d3-4ea24b5b1410");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR1)};
    TestError error;
    sut.and_then([&](auto&) { error = TestError::ERROR2; }).or_else([&](auto& r) { error = r; });

    EXPECT_THAT(error, Eq(TestError::ERROR1));
}

TEST_F(expected_test, ConstWhenHavingAnErrorCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "f93447da-16ea-45b1-89bd-3ddd34562c10");
    const expected<int, TestError> sut{error<TestError>(TestError::ERROR2)};
    TestError error;
    sut.and_then([&](auto&) { error = TestError::ERROR1; }).or_else([&](auto& r) { error = r; });

    EXPECT_THAT(error, Eq(TestError::ERROR2));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingAnErrorCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a479d51-324b-4d87-bf36-7a10ca98f1ea");
    expected<TestError> sut{error<TestError>(TestError::ERROR2)};
    TestError error;
    sut.and_then([&]() { error = TestError::ERROR1; }).or_else([&](auto& r) { error = r; });

    EXPECT_THAT(error, Eq(TestError::ERROR2));
}

TEST_F(expected_test, ErrorTypeOnlyConstWhenHavingAnErrorCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "93512987-6cd1-4895-b345-ea6004e5ed13");
    const expected<TestError> sut{error<TestError>(TestError::ERROR1)};
    float a = 55.44f;
    sut.and_then([&]() { a = 91.f; }).or_else([&](auto&) { a = 612.1f; });

    EXPECT_THAT(a, Eq(612.1f));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingSuccessCallsAndThen)
{
    ::testing::Test::RecordProperty("TEST_ID", "a7e6ec36-094d-4d63-ae97-7e712a4fa83e");
    expected<TestError> sut{success<>()};
    int a = 0;
    sut.and_then([&]() { a = 65; }).or_else([&](auto&) { a = 111111; });

    EXPECT_THAT(a, Eq(65));
}

TEST_F(expected_test, WhenHavingSuccessCallsAndThen)
{
    ::testing::Test::RecordProperty("TEST_ID", "010e4cc6-0966-462b-bda0-a0c2c0d680e4");
    expected<int, TestError> sut{success<int>(112)};
    int a = 0;
    sut.and_then([&](auto& r) { a = r; }).or_else([&](auto&) { a = 3; });

    EXPECT_THAT(a, Eq(112));
}

TEST_F(expected_test, ConstWhenHavingSuccessCallsAndThen)
{
    ::testing::Test::RecordProperty("TEST_ID", "5371e909-0ea5-494b-b969-cb382a0189b8");
    const expected<int, TestError> sut{success<int>(1142)};
    int a = 0;
    sut.and_then([&](auto& r) { a = r; }).or_else([&](auto&) { a = 3; });

    EXPECT_THAT(a, Eq(1142));
}

TEST_F(expected_test, WhenHavingSuccessAndMoveAssignmentCallsAndThen)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c30ccec-614d-4ef3-bb60-c187a8679b8d");
    expected<int, TestError> sut{success<int>(1143)};
    auto movedValue = std::move(sut);
    IOX_DISCARD_RESULT(movedValue);

    bool success{false};
    sut.and_then([&](auto&) { success = true; }).or_else([&](auto&) { FAIL() << "'or_else' should not be called"; });
    EXPECT_TRUE(success);
}

TEST_F(expected_test, WhenHavingAnErrorAndMoveAssignmentCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "f55225eb-7f60-4748-8b69-13fde30d6aa3");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR1)};
    auto movedValue = std::move(sut);
    IOX_DISCARD_RESULT(movedValue);

    bool success{false};
    sut.and_then([&](auto&) { FAIL() << "'and_then' should not be called"; }).or_else([&](auto&) { success = true; });
    EXPECT_TRUE(success);
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingSuccessAndMoveAssignmentCallsAndThen)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e4b0e4e-fdd1-49da-98ff-21d71a5178e6");
    expected<TestError> sut{success<>()};
    auto movedValue = std::move(sut);
    IOX_DISCARD_RESULT(movedValue);

    bool success{false};
    sut.and_then([&]() { success = true; }).or_else([&](auto&) { FAIL() << "'or_else' should not be called"; });
    EXPECT_TRUE(success);
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingAnErrorAndMoveAssignmentCallsOrElse)
{
    ::testing::Test::RecordProperty("TEST_ID", "417e4296-9542-4407-9d27-e4a2f2bb306f");
    expected<TestError> sut{error<TestError>(TestError::ERROR1)};
    auto movedValue = std::move(sut);
    IOX_DISCARD_RESULT(movedValue);

    bool success{false};
    sut.and_then([&]() { FAIL() << "'and_then' should not be called"; }).or_else([&](auto&) { success = true; });
    EXPECT_TRUE(success);
}

TEST_F(expected_test, ConvertNonEmptySuccessResultToErrorTypeOnlyResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b14f4aaa-abd0-4b99-84df-d644506712fa");
    expected<int, TestError> sut{success<int>(123)};
    expected<TestError> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(false));
}

TEST_F(expected_test, ConvertConstNonEmptySuccessResultToErrorTypeOnlyResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ccaf1cf-1b09-4930-ad33-8f961aca4c2e");
    const expected<int, TestError> sut{success<int>(123)};
    expected<TestError> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(false));
}

TEST_F(expected_test, ConvertNonEmptyErrorResultToErrorTypeOnlyResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "5907d318-cf1a-46f1-9016-07096153d7d9");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR2)};
    expected<TestError> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(true));
    EXPECT_THAT(sut2.get_error(), Eq(TestError::ERROR2));
}

TEST_F(expected_test, ExpectedWithValueConvertsToOptionalWithValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a877f9bd-5793-437f-8dee-a109aed9f647");
    expected<int, TestError> sut{success<int>(4711)};
    optional<int> value = sut.to_optional();
    ASSERT_THAT(value.has_value(), Eq(true));
    EXPECT_THAT(*value, Eq(4711));
}

TEST_F(expected_test, ExpectedWithErrorConvertsToOptionalWithoutValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe161275-8fa2-43c9-86e7-0a20d79eb44f");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR1)};
    optional<int> value = sut.to_optional();
    ASSERT_THAT(value.has_value(), Eq(false));
}

TEST_F(expected_test, AndThenUnpacksOptionalWhenNonEmptyOptionalValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b5429f1-3755-4027-ace3-7614640252e5");
    auto sut = expected<iox::cxx::optional<int>, TestError>::create_value(123);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onSuccess).Times(1);

    sut.and_then([&mocks](int& val) {
        mocks.onSuccess();
        ASSERT_THAT(val, Eq(123));
    });
}

TEST_F(expected_test, ConstAndThenUnpacksOptionalWhenNonEmptyOptionalValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cdfc2bf1-a35a-43fc-a049-513085d1a8a6");
    const auto sut = expected<iox::cxx::optional<int>, TestError>::create_value(321);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onSuccess).Times(1);

    sut.and_then([&mocks](int& val) {
        mocks.onSuccess();
        ASSERT_THAT(val, Eq(321));
    });
}

TEST_F(expected_test, AndThenNotCalledWhenEmptyOptionalValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ad22cfe-4341-4947-9b66-89b2615b0877");
    auto sut = expected<iox::cxx::optional<int>, TestError>::create_value(iox::cxx::nullopt);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onSuccess).Times(0);

    sut.and_then([&mocks](int&) { mocks.onSuccess(); });
}

TEST_F(expected_test, AndThenInValueExpectedWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e2e8278-454e-4f17-b295-c418a2972ab1");
    auto sut1 = expected<int, TestError>::create_value(123);
    const auto sut2 = expected<int, TestError>::create_value(123);
    auto sut3 = expected<iox::cxx::optional<int>, TestError>::create_value(123);
    const auto sut4 = expected<iox::cxx::optional<int>, TestError>::create_value(123);

    // we test here that std::terminate is not called from the function_ref
    sut1.and_then(iox::cxx::function_ref<void(int&)>());
    sut2.and_then(iox::cxx::function_ref<void(int&)>());
    sut3.and_then(iox::cxx::function_ref<void(int&)>());
    sut4.and_then(iox::cxx::function_ref<void(int&)>());
}

TEST_F(expected_test, OrElseInValueExpectedWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "a81a57ac-5932-4077-a51f-83939abd0065");
    auto sut1 = expected<int, TestError>::create_error(TestError::ERROR1);
    const auto sut2 = expected<int, TestError>::create_error(TestError::ERROR1);

    // we test here that std::terminate is not called from the function_ref
    sut1.or_else(iox::cxx::function_ref<void(TestError&)>());
    sut2.or_else(iox::cxx::function_ref<void(TestError&)>());
}

TEST_F(expected_test, AndThenInErrorExpectedWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e6b7874-52eb-4029-8b0f-68006a5a244e");
    auto sut1 = expected<TestError>::create_value();
    const auto sut2 = expected<TestError>::create_value();

    // we test here that std::terminate is not called from the function_ref
    sut1.and_then(iox::cxx::function_ref<void()>());
    sut2.and_then(iox::cxx::function_ref<void()>());
}

TEST_F(expected_test, OrElseInErrorExpectedWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "59be2b98-06ee-4c10-867d-deaabd3d113f");
    auto sut1 = expected<TestError>::create_error(TestError::ERROR1);
    const auto sut2 = expected<TestError>::create_error(TestError::ERROR1);

    // we test here that std::terminate is not called from the function_ref
    sut1.or_else(iox::cxx::function_ref<void(TestError&)>());
    sut2.or_else(iox::cxx::function_ref<void(TestError&)>());
}
} // namespace
