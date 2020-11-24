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

TEST_F(expected_test, CreateWithValue)
{
    auto sut = expected<int, float>::create_value(123);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(123));
}

TEST_F(expected_test, CreateWitherror)
{
    auto sut = expected<int, float>::create_error(123.12f);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(123.12f));
}

TEST_F(expected_test, CreateValue)
{
    auto sut = expected<Test, int>::create_value(12, 222);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value().m_a, Eq(12));
}

TEST_F(expected_test, CreateError)
{
    auto sut = expected<int, Test>::create_error(313, 212);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error().m_b, Eq(212));
}

TEST_F(expected_test, BoolOperatorError)
{
    auto sut = expected<int, Test>::create_error(123, 321);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    EXPECT_THAT(sut.get_error().m_b, Eq(321));
}

TEST_F(expected_test, BoolOperatorValue)
{
    auto sut = expected<Test, int>::create_value(123, 321);

    ASSERT_THAT(sut.operator bool(), Eq(true));
    EXPECT_THAT(sut.value().m_a, Eq(123));
}

TEST_F(expected_test, BoolOperatorExpectedErrorType)
{
    auto sut = expected<float>::create_error(5.8f);
    ASSERT_THAT(sut.operator bool(), Eq(false));
    ASSERT_THAT(sut.get_error(), Eq(5.8f));
}

TEST_F(expected_test, BoolOperatorExpectedErrorTypeValue)
{
    auto sut = expected<float>::create_value();
    ASSERT_THAT(sut.operator bool(), Eq(true));
}

TEST_F(expected_test, GetValueOrWithError)
{
    auto sut = expected<int, float>::create_error(16523.12f);
    EXPECT_THAT(sut.value_or(90), Eq(90));
}

TEST_F(expected_test, GetValueOrWithSuccess)
{
    auto sut = expected<int, float>::create_value(165);
    EXPECT_THAT(sut.value_or(90), Eq(165));
}

TEST_F(expected_test, ConstGetValueOrWithError)
{
    auto sut = expected<int, float>::create_error(1652.12f);
    EXPECT_THAT(sut.value_or(15), Eq(15));
}

TEST_F(expected_test, ConstGetValueOrWithSuccess)
{
    auto sut = expected<int, float>::create_value(652);
    EXPECT_THAT(sut.value_or(15), Eq(652));
}

TEST_F(expected_test, ArrowOperator)
{
    auto sut = expected<Test, float>::create_value(55, 81);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->gimme(), Eq(136));
}

TEST_F(expected_test, ConstArrowOperator)
{
    const expected<Test, float> sut(success<Test>(Test(55, 81)));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->constGimme(), Eq(136));
}

TEST_F(expected_test, Dereferencing)
{
    auto sut = expected<int, float>::create_value(1652);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(1652));
}

TEST_F(expected_test, ConstDereferencing)
{
    const expected<int, float> sut(success<int>(981));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(*sut, Eq(981));
}

TEST_F(expected_test, VoidCreateValue)
{
    auto sut = expected<float>::create_value();
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, VoidCreateError)
{
    auto sut = expected<float>::create_error(12.2f);
    ASSERT_THAT(sut.has_error(), Eq(true));
    ASSERT_THAT(sut.get_error(), Eq(12.2f));
}

TEST_F(expected_test, VoidCreateFromSuccessType)
{
    expected<float> sut{success<>()};
    ASSERT_THAT(sut.has_error(), Eq(false));
}

TEST_F(expected_test, CreateFromSuccessType)
{
    expected<int, float> sut{success<int>(55)};
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.value(), Eq(55));
}

TEST_F(expected_test, VoidCreateFromErrorType)
{
    expected<float> sut{error<float>(12.1f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(12.1f));
}

TEST_F(expected_test, CreateFromErrorType)
{
    expected<int, float> sut{error<float>(112.1f)};
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error(), Eq(112.1f));
}

TEST_F(expected_test, OrElseWhenHavingAnErrorWithResult)
{
    expected<int, float> sut{error<float>(112.1f)};
    float a = 0.2f;
    sut.and_then([&](int&) { a = 2.0f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(112.1f));
}

TEST_F(expected_test, ConstOrElseWhenHavingAnErrorWithResult)
{
    const expected<int, float> sut{error<float>(12.1f)};
    float a = 7.1f;
    sut.and_then([&](int&) { a = 91.f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(12.1f));
}

TEST_F(expected_test, ErrorTypeOnlyOrElseWhenHavingAnErrorWithResultErrorType)
{
    expected<float> sut{error<float>(7112.1f)};
    float a = 70.2f;
    sut.and_then([&]() { a = 2.0f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(7112.1f));
}

TEST_F(expected_test, ErrorTypeOnlyConstOrElseWhenHavingAnErrorWithResultErrorType)
{
    const expected<float> sut{error<float>(612.1f)};
    float a = 67.1f;
    sut.and_then([&]() { a = 91.f; }).or_else([&](float& r) { a = r; });

    EXPECT_THAT(a, Eq(612.1f));
}

TEST_F(expected_test, ValueTypeAndThenWhenHavingSuccessWithResult)
{
    expected<int, float> sut{success<int>(112)};
    int a = 0;
    sut.and_then([&](int& r) { a = r; }).or_else([&](float&) { a = 3; });

    EXPECT_THAT(a, Eq(112));
}

TEST_F(expected_test, ValueTypeConstOnSuccessWhenHavingSuccessWithResult)
{
    const expected<int, float> sut{success<int>(1142)};
    int a = 0;
    sut.and_then([&](int& r) { a = r; }).or_else([&](float&) { a = 3; });

    EXPECT_THAT(a, Eq(1142));
}

TEST_F(expected_test, ConvertNonVoidSuccessResultToVoidResult)
{
    expected<int, float> sut{success<int>(123)};
    expected<float> sut2 = sut;
    EXPECT_THAT(sut2.has_error(), Eq(false));
}

TEST_F(expected_test, ConvertNonVoidErrorResultToVoidResult)
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

TEST_F(expected_test, ExpectedWithErrorConvertsToNullopt)
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

TEST_F(expected_test, IfEmptyCalledWhenEmptyOptionalValue)
{
    auto sut = expected<iox::cxx::optional<int>, float>::create_value(iox::cxx::nullopt);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onEmpty).Times(1);

    sut.if_empty([&mocks]() { mocks.onEmpty(); });
}

TEST_F(expected_test, IfEmptyNotCalledWhenValueTypeIsNonEmptyOptionalValue)
{
    auto sut = expected<iox::cxx::optional<int>, float>::create_value(123);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onEmpty).Times(0);

    sut.if_empty([&mocks]() { mocks.onEmpty(); });
}

TEST_F(expected_test, IfEmptyNotCalledWhenErrorOccurs)
{
    auto sut = expected<iox::cxx::optional<int>, float>::create_error(42.42);
    MockCallables mocks{};
    EXPECT_CALL(mocks, onEmpty).Times(0);

    sut.if_empty([&mocks]() { mocks.onEmpty(); });
}
