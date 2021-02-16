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

#include "iceoryx_utils/cxx/expected.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

class MockCallables
{
  public:
    MockCallables() = default;
    MOCK_METHOD0(onSuccess, void());
    MOCK_METHOD0(onEmpty, void());
    MOCK_METHOD0(onError, void());
};

class expected_test : public Test
{
  public:
    struct Test
    {
        Test(int a, int b)
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


        int m_a;
        int m_b;
    };
};

TEST_F(expected_test, CreateWithPODTypeIsSuccessful)
{
    auto sut = expected<int, float>::create_value(123);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(123));
}

TEST_F(expected_test, CreateWithErrorResultsInError)
{
    auto sut = expected<int, float>::create_error(123.12f);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(123.12f));
}

TEST_F(expected_test, ErrorTypeOnlyConstCreateWithErrorResultsInError)
{
    const auto sut = expected<float>::create_error(5.8f);
    ASSERT_THAT(std::move(sut.has_error()), Eq(true));
    ASSERT_THAT(std::move(sut.get_error()), Eq(5.8f));
}

TEST_F(expected_test, ErrorTypeOnlyCreateWithErrorResultsInError)
{
    auto sut = expected<float>::create_error(8.5f);
    ASSERT_THAT(sut.has_error(), Eq(true));
    ASSERT_THAT(sut.get_error(), Eq(8.5f));
}

TEST_F(expected_test, ErrorTypeOnlyCreateFromConstErrorResultsInError)
{
    auto constError = error<float>(8.55f);
    auto sut = expected<float>(constError);
    ASSERT_THAT(sut.has_error(), Eq(true));
    ASSERT_THAT(sut.get_error(), Eq(8.55f));
}

TEST_F(expected_test, CreateWithComplexTypeIsSuccessful)
{
    auto sut = expected<Test, int>::create_value(12, 222);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value().m_a, Eq(12));
}

TEST_F(expected_test, ConstCreateRValueAndGetValueResultsInCorrectValue)
{
    const auto&& sut = expected<int, Test>::create_value(1111);

    EXPECT_THAT(sut.value(), Eq(1111));
}

TEST_F(expected_test, ConstCreateRValueAndMoveLeadsToTermination)
{
    // const auto&& sut = expected<int, Test>::create_value(2222);
    // auto movedSut = std::move(sut);

    // EXPECT_FALSE(movedSut.has_error());
    // EXPECT_DEATH(sut.value(), ".*");
}

TEST_F(expected_test, CreateLValueAndMoveLeadsToTermination)
{
    // const auto& sut = expected<int, Test>::create_value(3333);
    // auto movedSut = std::move(sut);

    // EXPECT_DEATH(sut.value(), ".*");
}

TEST_F(expected_test, CreateWithComplexErrorResultsInError)
{
    auto sut = expected<int, Test>::create_error(313, 212);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error().m_b, Eq(212));
}

TEST_F(expected_test, CreateRValueAndGetErrorResultsInCorrectError)
{
    auto error = expected<int, Test>::create_error(131, 121).get_error();

    EXPECT_THAT(error.m_b, Eq(121));
}

TEST_F(expected_test, ConstCreateRValueAndGetErrorResultsInCorrectError)
{
    const auto&& sut = expected<int, Test>::create_error(131, 121);

    EXPECT_THAT(sut.get_error().m_b, Eq(121));
}

TEST_F(expected_test, ConstCreateLValueAndGetErrorResultsInCorrectError)
{
    const auto& sut = expected<int, Test>::create_error(343, 232);

    EXPECT_THAT(sut.get_error().m_b, Eq(232));
}

TEST_F(expected_test, CreateWithValueAndMoveCtorLeadsToInvalidation)
{
    auto sut = expected<int, Test>::create_value(66);
    auto movedValue{std::move(sut)};
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.value(), Eq(66));
}

TEST_F(expected_test, CreateWithErrorAndMoveCtorLeadsToInvalidation)
{
    auto sut = expected<int, Test>::create_error(22, 33);
    auto movedValue{std::move(sut)};
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.get_error().m_b, Eq(33));
}

TEST_F(expected_test, CreateWithValueAndMoveAssignmentLeadsToInvalidation)
{
    auto sut = expected<int, Test>::create_value(73);
    auto movedValue = std::move(sut);
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.value(), Eq(73));
}

TEST_F(expected_test, CreateWithErrorAndMoveAssignmentLeadsToInvalidation)
{
    auto sut = expected<int, Test>::create_error(44, 55);
    auto movedValue = std::move(sut);
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.get_error().m_b, Eq(55));
}

TEST_F(expected_test, CreateInvalidExpectedAndCallGetErrorLeadsToTermination)
{
    auto sut = expected<int, float>::create_value(11);
    auto movedValue = std::move(sut);

    EXPECT_DEATH(sut.get_error(), ".*");
}

TEST_F(expected_test, ErrorTypeOnlyCreateInvalidExpectedAndCallGetErrorLeadsToTermination)
{
    auto sut = expected<float>::create_error(19.0f);
    auto movedValue = std::move(sut);

    EXPECT_DEATH(sut.get_error(), ".*");
}

TEST_F(expected_test, BoolOperatorReturnsError)
{
    auto sut = expected<int, Test>::create_error(123, 321);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    EXPECT_THAT(sut.get_error().m_b, Eq(321));
}

TEST_F(expected_test, BoolOperatorReturnsNoError)
{
    auto sut = expected<Test, int>::create_value(123, 321);

    ASSERT_THAT(sut.operator bool(), Eq(true));
    EXPECT_THAT(sut.value().m_a, Eq(123));
}

TEST_F(expected_test, ErrorTypeOnlyBoolOperatorReturnsError)
{
    auto sut = expected<float>::create_error(5.8f);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    ASSERT_THAT(sut.get_error(), Eq(5.8f));
}

TEST_F(expected_test, ErrorTypeOnlyBoolOperatorReturnsNoError)
{
    auto sut = expected<float>::create_value();
    ASSERT_THAT(sut.operator bool(), Eq(true));
}

TEST_F(expected_test, ValueOrWithErrorReturnsGivenValue)
{
    auto sut = expected<int, float>::create_error(16523.12f);
    EXPECT_THAT(sut.value_or(90), Eq(90));
}

TEST_F(expected_test, ValueOrWithErrorReturnsStoredValue)
{
    auto sut = expected<int, float>::create_value(165);
    EXPECT_THAT(sut.value_or(90), Eq(165));
}

TEST_F(expected_test, GetValueOrWithError)
{
    auto sut = expected<int, float>::create_error(1234.56f);
    EXPECT_THAT(sut.value_or(15), Eq(15));
}

TEST_F(expected_test, ConstGetValueOrWithError)
{
    const auto sut = expected<int, float>::create_error(1652.12f);
    EXPECT_THAT(sut.value_or(51), Eq(51));
}

TEST_F(expected_test, GetValueOrWithSuccess)
{
    auto sut = expected<int, float>::create_value(999);
    EXPECT_THAT(sut.value_or(15), Eq(999));
}

TEST_F(expected_test, ConstGetValueOrWithSuccess)
{
    const auto sut = expected<int, float>::create_value(652);
    EXPECT_THAT(sut.value_or(15), Eq(652));
}

TEST_F(expected_test, ArrowOperatorWorks)
{
    auto sut = expected<Test, float>::create_value(55, 81);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->gimme(), Eq(136));
}

TEST_F(expected_test, ConstArrowOperatorWorks)
{
    const expected<Test, float> sut(success<Test>(Test(55, 81)));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->constGimme(), Eq(136));
}

TEST_F(expected_test, MoveAndCallArrowOperatorLeadsToTermination)
{
    auto sut = expected<Test, float>::create_value(55, 81);
    auto movedSut = std::move(sut);
    EXPECT_DEATH(sut->gimme(), ".*");
}

TEST_F(expected_test, DereferencingOperatorWorks)
{
    auto sut = expected<int, float>::create_value(1652);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(1652));
}

TEST_F(expected_test, ConstDereferencingOperatorWorks)
{
    const expected<int, float> sut(success<int>(981));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(981));
}

TEST_F(expected_test, MoveAndCallDereferencingOperatorLeadsToTermination)
{
    auto sut = expected<Test, float>::create_value(81, 55);
    auto movedSut = std::move(sut);
    EXPECT_DEATH(*sut, ".*");
}

TEST_F(expected_test, ErrorTypeOnlyCreateValueWithoutValueLeadsToValidSut)
{
    auto sut = expected<float>::create_value();
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, ErrorTypeOnlyCreateErrorLeadsToError)
{
    auto sut = expected<float>::create_error(12.2f);
    ASSERT_THAT(sut.has_error(), Eq(true));
    ASSERT_THAT(sut.get_error(), Eq(12.2f));
}

TEST_F(expected_test, ErrorTypeOnlyMoveCtorLeadsToInvalidation)
{
    auto sut = expected<float>::create_error(43.0f);
    auto movedValue{std::move(sut)};
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.get_error(), Eq(43.0f));
}

TEST_F(expected_test, ErrorTypeOnlyMoveAssignmentLeadsToInvalidation)
{
    auto sut = expected<float>::create_error(42.0f);
    auto movedValue = std::move(sut);
    EXPECT_TRUE(sut.has_undefined_state());
    ASSERT_FALSE(movedValue.has_undefined_state());
    EXPECT_THAT(movedValue.get_error(), Eq(42.0f));
}

TEST_F(expected_test, CreateFromEmptySuccessTypeLeadsToValidSut)
{
    expected<float> sut{success<>()};
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, CreateFromSuccessTypeLeadsToValidSut)
{
    expected<int, float> sut{success<int>(55)};
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(55));
}

TEST_F(expected_test, CreateFromErrorConstLeadsToCorrectError)
{
    const float f = 12.1f;
    expected<float> sut{error<float>(f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(12.1f));
}

TEST_F(expected_test, ErrorTypeOnlyCreateFromErrorLeadsToCorrectError)
{
    expected<float> sut{error<float>(12.1f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(12.1f));
}

TEST_F(expected_test, CreateFromErrorLeadsToCorrectError)
{
    expected<int, float> sut{error<float>(112.1f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(112.1f));
}

TEST_F(expected_test, WhenHavingAnErrorCallsOrElse)
{
    expected<int, float> sut{error<float>(112.1f)};
    float a = 0.2f;
    sut.and_then([&](int&) { a = 2.0f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(112.1f));
}

TEST_F(expected_test, ConstWhenHavingAnErrorCallsOrElse)
{
    const expected<int, float> sut{error<float>(12.1f)};
    float a = 7.1f;
    sut.and_then([&](int&) { a = 91.f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(12.1f));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingAnErrorCallsOrElse)
{
    expected<float> sut{error<float>(7112.1f)};
    float a = 70.2f;
    sut.and_then([&]() { a = 2.0f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(7112.1f));
}

TEST_F(expected_test, ErrorTypeOnlyConstWhenHavingAnErrorCallsOrElse)
{
    const expected<float> sut{error<float>(612.1f)};
    float a = 67.1f;
    sut.and_then([&]() { a = 91.f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(612.1f));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingSuccessCallsAndThen)
{
    expected<float> sut{success<>()};
    int a = 0;
    sut.and_then([&]() { a = 3; }).or_else([&](float& error) { a = error; });

    EXPECT_THAT(a, Eq(3));
}

TEST_F(expected_test, WhenHavingSuccessCallsAndThen)
{
    expected<int, float> sut{success<int>(112)};
    int a = 0;
    sut.and_then([&](int& r) { a = r; }).or_else([&](float&) { a = 3; });

    EXPECT_THAT(a, Eq(112));
}

TEST_F(expected_test, ConstWhenHavingSuccessCallsAndThen)
{
    const expected<int, float> sut{success<int>(1142)};
    int a = 0;
    sut.and_then([&](int& r) { a = r; }).or_else([&](float&) { a = 3; });

    EXPECT_THAT(a, Eq(1142));
}

TEST_F(expected_test, WhenHavingSuccessAndMoveAssignmentCallsNothing)
{
    expected<int, float> sut{success<int>(1143)};
    auto movedValue = std::move(sut);
    int a = 0;
    sut.and_then([&](auto& value) { a = value; }).or_else([&](auto& error) { a = error; });
    EXPECT_THAT(a, Eq(0));
}

TEST_F(expected_test, WhenHavingAnErrorAndMoveAssignmentCallsNothing)
{
    expected<int, float> sut{error<float>(33.44f)};
    auto movedValue = std::move(sut);
    int a = 0;
    sut.and_then([&](auto& value) { a = value; }).or_else([&](auto& error) { a = error; });
    EXPECT_THAT(a, Eq(0));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingSuccessAndMoveAssignmentCallsNothing)
{
    expected<float> sut{success<>()};
    auto movedValue = std::move(sut);
    int a = 0;
    sut.and_then([&]() { a = 54; }).or_else([&](auto& error) { a = error; });
    EXPECT_THAT(a, Eq(0));
}

TEST_F(expected_test, ErrorTypeOnlyWhenHavingAnErrorAndMoveAssignmentCallsNothing)
{
    expected<float> sut{error<float>(22.11f)};
    auto movedValue = std::move(sut);
    int a = 0;
    sut.and_then([&]() { a = 45; }).or_else([&](auto& error) { a = error; });
    EXPECT_THAT(a, Eq(0));
}

TEST_F(expected_test, ConvertNonEmptySuccessResultToErrorTypeOnlyResult)
{
    expected<int, float> sut{success<int>(123)};
    expected<float> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(false));
}

TEST_F(expected_test, ConvertConstNonEmptySuccessResultToErrorTypeOnlyResult)
{
    const expected<int, float> sut{success<int>(123)};
    expected<float> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(false));
}

TEST_F(expected_test, ConvertNonEmptyErrorResultToErrorTypeOnlyResult)
{
    expected<int, float> sut{error<float>(1.23f)};
    expected<float> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(true));
    EXPECT_THAT(sut2.get_error(), Eq(1.23f));
}

TEST_F(expected_test, ExpectedWithValueConvertsToOptionalWithValue)
{
    expected<int, float> sut{success<int>(4711)};
    optional<int> value = sut.to_optional();
    ASSERT_THAT(value.has_value(), Eq(true));
    EXPECT_THAT(*value, Eq(4711));
}

TEST_F(expected_test, ExpectedWithErrorConvertsToOptionalWithoutValue)
{
    expected<int, float> sut{error<float>(47.11f)};
    optional<int> value = sut.to_optional();
    ASSERT_THAT(value.has_value(), Eq(false));
}

TEST_F(expected_test, AndThenUnpacksOptionalWhenNonEmptyOptionalValue)
{
    auto sut = expected<iox::cxx::optional<int>, float>::create_value(123);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onSuccess).Times(1);

    sut.and_then([&mocks](int& val) {
        mocks.onSuccess();
        ASSERT_THAT(val, Eq(123));
    });
}

TEST_F(expected_test, AndThenNotCalledWhenEmptyOptionalValue)
{
    auto sut = expected<iox::cxx::optional<int>, float>::create_value(iox::cxx::nullopt);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onSuccess).Times(0);

    sut.and_then([&mocks](int&) { mocks.onSuccess(); });
}
