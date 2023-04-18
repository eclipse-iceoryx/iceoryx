// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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
#include "iox/expected.hpp"
#include "iox/string.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace ::iox::cxx;
using namespace ::iox;
using namespace ::iox::testing;

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
    // the parameters can be swapped without changing the outcome, methods which
    // work on them are commutative
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    TestClass(int a, int b)
        : m_a(a)
        , m_b(b)
    {
    }

    // should be non const to be usable in tests
    // NOLINTNEXTLINE(readability-make-member-function-const)
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
    // swapped parameters will be directly detected with failing tests
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    NonTrivialTestClass(int a, int b)
        : m_a(a)
        , m_b(b)
    {
    }

    NonTrivialTestClass(const NonTrivialTestClass& other)
    {
        *this = other;
    }
    NonTrivialTestClass(NonTrivialTestClass&& other) noexcept
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

    NonTrivialTestClass& operator=(NonTrivialTestClass&& rhs) noexcept
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

struct ClassWithMoveCtorAndNoMoveAssignment
{
    ClassWithMoveCtorAndNoMoveAssignment() = default;
    ClassWithMoveCtorAndNoMoveAssignment(const ClassWithMoveCtorAndNoMoveAssignment&) = delete;
    ClassWithMoveCtorAndNoMoveAssignment(ClassWithMoveCtorAndNoMoveAssignment&&) = default;
    ~ClassWithMoveCtorAndNoMoveAssignment() = default;

    ClassWithMoveCtorAndNoMoveAssignment& operator=(const ClassWithMoveCtorAndNoMoveAssignment&) = delete;
    ClassWithMoveCtorAndNoMoveAssignment& operator=(ClassWithMoveCtorAndNoMoveAssignment&&) = delete;
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
    constexpr int VALUE = 123;
    auto sut = expected<int, TestError>::create_value(VALUE);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(VALUE));
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
    constexpr int VALUE = 424242;
    auto constSuccess = success<int>(VALUE);
    auto sut = expected<int, TestError>(constSuccess);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(VALUE));
}

TEST_F(expected_test, CreateWithComplexTypeIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "508a39f7-905a-4d9a-a61b-43145e546eca");
    constexpr int VALUE_A = 12;
    constexpr int VALUE_B = 222;
    auto sut = expected<TestClass, TestError>::create_value(VALUE_A, VALUE_B);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value().m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.value().m_b, Eq(VALUE_B));
}

TEST_F(expected_test, CreateWithSTLTypeIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "24fddc69-64ca-4b69-baab-a58293657cac");
    const std::string ERROR_VALUE = "RedAlert";
    auto sut = expected<int, std::string>::create_error(ERROR_VALUE);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(ERROR_VALUE));
}

TEST_F(expected_test, CreateWithComplexErrorResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "71e6ea31-d6e3-42a0-a63d-4bbd39c7341c");
    constexpr int VALUE_A = 313;
    constexpr int VALUE_B = 212;
    auto sut = expected<int, TestClass>::create_error(VALUE_A, VALUE_B);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error().m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.get_error().m_b, Eq(VALUE_B));
}

TEST_F(expected_test, CreateRValueAndGetErrorResultsInCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b032400a-cd08-4ae7-af0c-5ae0362b4dc0");
    constexpr int VALUE_A = 131;
    constexpr int VALUE_B = 121;
    auto sut = expected<int, TestClass>::create_error(VALUE_A, VALUE_B).get_error();
    EXPECT_THAT(sut.m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.m_b, Eq(VALUE_B));
}

TEST_F(expected_test, ConstCreateLValueAndGetErrorResultsInCorrectError)
{
    ::testing::Test::RecordProperty("TEST_ID", "e56063ea-8b7c-4d47-a898-fe609ea3b283");
    constexpr int VALUE_A = 131;
    constexpr int VALUE_B = 121;
    const auto& sut = expected<int, TestClass>::create_error(VALUE_A, VALUE_B);
    EXPECT_THAT(sut.get_error().m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.get_error().m_b, Eq(VALUE_B));
}

TEST_F(expected_test, CreateWithValueAndMoveCtorLeadsToMovedSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "8da72983-3046-4dde-8de5-5eed89de0ccf");
    constexpr int A{177};
    constexpr int B{188};
    auto sutSource = expected<NonTrivialTestClass, int>::create_value(A, B);
    auto sutDestination{std::move(sutSource)};

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_FALSE(sutSource.has_error());
    EXPECT_TRUE(sutSource.value().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
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

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
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

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_FALSE(sutSource.has_error());
    EXPECT_TRUE(sutSource.value().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
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

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, BoolOperatorReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1e30651-a0e9-4c73-b2bf-57f36fc7eddf");
    constexpr int VALUE_A = 55899;
    constexpr int VALUE_B = 11;
    auto sut = expected<int, TestClass>::create_error(VALUE_A, VALUE_B);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    EXPECT_THAT(sut.get_error().m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.get_error().m_b, Eq(VALUE_B));
}

TEST_F(expected_test, BoolOperatorReturnsNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "aec3e2a3-b7ae-4778-ac1d-d52e64b9b2d3");
    constexpr int VALUE_A = 5599;
    constexpr int VALUE_B = 8111;
    auto sut = expected<TestClass, TestError>::create_value(VALUE_A, VALUE_B);

    ASSERT_THAT(sut.operator bool(), Eq(true));
    EXPECT_THAT(sut.value().m_a, Eq(VALUE_A));
    EXPECT_THAT(sut.value().m_b, Eq(VALUE_B));
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

TEST_F(expected_test, ArrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "39898e81-d4ad-4f27-8c45-d29c80114be2");
    constexpr int VALUE_A = 55;
    constexpr int VALUE_B = 81;
    auto sut = expected<TestClass, TestError>::create_value(VALUE_A, VALUE_B);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->gimme(), Eq(VALUE_A + VALUE_B));
}

TEST_F(expected_test, ConstArrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b35a05e9-6dbc-4cfb-94c2-85ca9d214bb4");
    constexpr int VALUE_A = 554;
    constexpr int VALUE_B = 811;
    const expected<TestClass, TestError> sut(success<TestClass>(TestClass(VALUE_A, VALUE_B)));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->constGimme(), Eq(VALUE_A + VALUE_B));
}

TEST_F(expected_test, DereferencingOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "11ddbd46-3a2f-43cd-a2d2-ebe2ad4019db");
    constexpr int VALUE = 1652;
    auto sut = expected<int, TestError>::create_value(VALUE);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(VALUE));
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
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(sutSource.has_error());
    EXPECT_FALSE(sutDestination.has_error());
}

TEST_F(expected_test, ErrorTypeOnlyCreateValueWithoutValueMoveAssignmentLeadsToNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "75d3f30e-d927-46bf-83a4-fb8361542333");
    auto sutSource = expected<NonTrivialTestClass>::create_value();
    auto sutDestination = std::move(sutSource);
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
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

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
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

    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved expected
    // NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutSource.has_error());
    EXPECT_TRUE(sutSource.get_error().m_moved);
    // NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    ASSERT_TRUE(sutDestination.has_error());
    EXPECT_FALSE(sutDestination.get_error().m_moved);
    EXPECT_EQ(sutDestination.get_error().m_a, A);
    EXPECT_EQ(sutDestination.get_error().m_b, B);
}

TEST_F(expected_test, CreateFromInPlaceTypeLeadsToValidErrorOnlySut)
{
    ::testing::Test::RecordProperty("TEST_ID", "91a8ad7f-4843-4bd9-a56b-0561ae6b56cb");
    expected<TestError> sut{in_place};
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, CreateFromInPlaceTypeLeadsToValidSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a527c62-aaea-44ae-9b99-027c19d032b5");
    constexpr int VALUE = 42;
    expected<int, TestError> sut{in_place, VALUE};
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(VALUE));
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
    constexpr int VALUE = 55;
    expected<int, TestError> sut{success<int>(VALUE)};
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(VALUE));
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

TEST_F(expected_test, ConvertNonEmptySuccessResultToErrorTypeOnlyResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b14f4aaa-abd0-4b99-84df-d644506712fa");
    constexpr int VALUE = 91823;
    expected<int, TestError> sut{success<int>(VALUE)};
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
    constexpr int VALUE = 4711;
    expected<int, TestError> sut{success<int>(VALUE)};
    optional<int> value = sut.to_optional();

    ASSERT_THAT(value.has_value(), Eq(true));
    EXPECT_THAT(*value, Eq(VALUE));
}

TEST_F(expected_test, ExpectedWithErrorConvertsToOptionalWithoutValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe161275-8fa2-43c9-86e7-0a20d79eb44f");
    expected<int, TestError> sut{error<TestError>(TestError::ERROR1)};
    optional<int> value = sut.to_optional();

    ASSERT_THAT(value.has_value(), Eq(false));
}

TEST_F(expected_test, MoveAssignmentIsNotEnforcedInMoveConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "71cd336f-798b-4f08-9ab6-be3c429c1674");
    {
        auto sut = expected<ClassWithMoveCtorAndNoMoveAssignment, int>::create_value();
        /// this should compile, if not then we enforce move assignment hidden in the implementation
        expected<ClassWithMoveCtorAndNoMoveAssignment, int> destination{std::move(sut)};
        ASSERT_THAT(destination.has_error(), Eq(false));
    }

    /// same test with the error only expected
    {
        auto sut = expected<ClassWithMoveCtorAndNoMoveAssignment>::create_error();
        /// this should compile, if not then we enforce move assignment hidden in the implementation
        expected<ClassWithMoveCtorAndNoMoveAssignment> destination{std::move(sut)};
        ASSERT_THAT(destination.has_error(), Eq(true));
    }
}

TEST_F(expected_test, AccessingErrorOfLValueErrorOnlyExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "da162edf-06b5-47d2-b35f-361d6004a6c4");

    auto sut = expected<TestError>::create_value();

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.get_error(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingErrorOfConstLValueErrorOnlyExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "324cab7d-ba04-4ff0-870f-79af993c272f");

    const auto sut = expected<TestError>::create_value();

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.get_error(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingErrorOfRValueErrorOnlyExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a4309e8-d9f3-41a9-9c4b-bdcfda917277");

    auto sut = expected<TestError>::create_value();

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { std::move(sut).get_error(); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfLValueExpectedWhichContainsErrorWithArrowOpLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a821c6f-83db-4fe1-8adf-873afa1251a1");

    auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { IOX_DISCARD_RESULT(sut->m_a); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfConstLValueExpectedWhichContainsErrorWithArrowOpLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4f04d7c-9fa3-48f6-a6fd-b8e4e47b7632");

    const auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { IOX_DISCARD_RESULT(sut->m_a); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfLValueExpectedWhichContainsErrorWithDerefOpLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "08ce6a3f-3813-46de-8e1e-3ffe8087521e");

    auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { *sut; }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfConstLValueExpectedWhichContainsErrorWithDerefOpLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "838dd364-f91f-40a7-9720-2b662a045b1e");

    const auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { *sut; }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfLValueExpectedWhichContainsErrorLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "92139583-b8d6-4d83-ae7e-f4109b98d214");

    auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.value(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfConstLValueExpectedWhichContainsErrorLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bcbb835-8b4c-4430-a534-a26573c2380d");

    const auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.value(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingValueOfRValueExpectedWhichContainsErrorLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "32d59b52-81f5-417a-8670-dfb2c54fedfb");

    auto sut = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { std::move(sut).value(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingErrorOfLValueExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "aee85ead-e066-49fd-99fe-6f1a6045756d");

    constexpr int VALID_VALUE{42};
    auto sut = expected<TestClass, TestError>::create_value(VALID_VALUE, VALID_VALUE);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.get_error(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingErrorOfConstLValueExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "a49cf02e-b165-4fd6-9c24-65cedc6cddb9");

    constexpr int VALID_VALUE{42};
    const auto sut = expected<TestClass, TestError>::create_value(VALID_VALUE, VALID_VALUE);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.get_error(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, AccessingErrorOfRValueExpectedWhichContainsValueLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ea90b5d-1af6-494a-b35c-da103bed2331");

    constexpr int VALID_VALUE{42};
    auto sut = expected<TestClass, TestError>::create_value(VALID_VALUE, VALID_VALUE);

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { std::move(sut).get_error(); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TEST_F(expected_test, TwoErrorOnlyExpectedWithEqualErrorAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "471b406d-8dd3-4b82-9d46-00c21d257461");
    auto sut1 = expected<TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestError>::create_error(TestError::ERROR1);

    EXPECT_TRUE(sut1 == sut2);
    EXPECT_FALSE(sut1 != sut2);
}

TEST_F(expected_test, TwoErrorOnlyExpectedWithUnequalErrorAreUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcc2f9f1-72a1-41ed-ac8a-2f48cdcfbc56");
    auto sut1 = expected<TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestError>::create_error(TestError::ERROR2);

    EXPECT_FALSE(sut1 == sut2);
    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(expected_test, TwoErrorOnlyExpectedWithValuesAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "75b25c16-fb79-4589-ab0f-bc73bb9fc2bb");
    auto sut1 = expected<TestError>::create_value();
    auto sut2 = expected<TestError>::create_value();

    EXPECT_TRUE(sut1 == sut2);
    EXPECT_FALSE(sut1 != sut2);
}

TEST_F(expected_test, TwoErrorOnlyExpectedWithErrorAndValueAreUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "2108715f-e71c-4778-bb64-553996e860b4");
    auto sut1 = expected<TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestError>::create_value();

    EXPECT_FALSE(sut1 == sut2);
    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(expected_test, TwoExpectedWithEqualErrorAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1a3b106-06f2-4667-ac25-7a9d9689c219");
    auto sut1 = expected<TestClass, TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestClass, TestError>::create_error(TestError::ERROR1);

    EXPECT_TRUE(sut1 == sut2);
    EXPECT_FALSE(sut1 != sut2);
}

TEST_F(expected_test, TwoExpectedsWithUnequalErrorAreUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "25250c6b-aa8f-40ad-ace9-2c55ce8eeaa2");
    auto sut1 = expected<TestClass, TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestClass, TestError>::create_error(TestError::ERROR2);

    EXPECT_FALSE(sut1 == sut2);
    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(expected_test, TwoExpectedWithEqualValueAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "278c2fd5-2b48-49d1-a8a4-8ca52b99de41");
    constexpr int VAL_1{42};
    constexpr int VAL_2{73};
    auto sut1 = expected<TestClass, TestError>::create_value(VAL_1, VAL_2);
    auto sut2 = expected<TestClass, TestError>::create_value(VAL_1, VAL_2);

    EXPECT_TRUE(sut1 == sut2);
    EXPECT_FALSE(sut1 != sut2);
}

TEST_F(expected_test, TwoExpectedWithUnequalValueAreUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f6a8760-6fdf-4ab8-a7d5-d751390aa672");
    constexpr int VAL_1{42};
    constexpr int VAL_2{73};
    auto sut1 = expected<TestClass, TestError>::create_value(VAL_1, VAL_1);
    auto sut2 = expected<TestClass, TestError>::create_value(VAL_2, VAL_2);

    EXPECT_FALSE(sut1 == sut2);
    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(expected_test, TwoExpectedWithErrorAndValueAreUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa912753-09af-46d5-92d5-52cad69795ad");
    constexpr int VAL{42};
    auto sut1 = expected<TestClass, TestError>::create_error(TestError::ERROR1);
    auto sut2 = expected<TestClass, TestError>::create_value(VAL, VAL);

    EXPECT_FALSE(sut1 == sut2);
    EXPECT_TRUE(sut1 != sut2);
}
} // namespace
