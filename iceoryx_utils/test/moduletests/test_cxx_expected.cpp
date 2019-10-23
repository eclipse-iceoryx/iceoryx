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

#include "test.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

using namespace ::testing;
using namespace iox::cxx;

class expected_test : public Test
{
  public:
    struct Test
    {
        Test(int a, int b)
            : a(a)
            , b(b)
        {
        }

        int Gimme()
        {
            return a + b;
        }

        int ConstGimme() const
        {
            return a + b;
        }


        int a;
        int b;
    };
};

TEST_F(expected_test, CreateWithValue)
{
    auto sut = expected<int, float>::create_value(123);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut.get_value(), Eq(123));
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
    EXPECT_THAT(sut.get_value().a, Eq(12));
}

TEST_F(expected_test, CreateError)
{
    auto sut = expected<int, Test>::create_error(313, 212);
    ASSERT_THAT(sut.has_error(), Eq(true));
    EXPECT_THAT(sut.get_error().b, Eq(212));
}

TEST_F(expected_test, GetValueOrWithError)
{
    auto sut = expected<int, float>::create_error(16523.12f);
    EXPECT_THAT(sut.get_value_or(90), Eq(90));
}

TEST_F(expected_test, GetValueOrWithSuccess)
{
    auto sut = expected<int, float>::create_value(165);
    EXPECT_THAT(sut.get_value_or(90), Eq(165));
}

TEST_F(expected_test, ConstGetValueOrWithError)
{
    auto sut = expected<int, float>::create_error(1652.12f);
    EXPECT_THAT(sut.get_value_or(15), Eq(15));
}

TEST_F(expected_test, ConstGetValueOrWithSuccess)
{
    auto sut = expected<int, float>::create_value(652);
    EXPECT_THAT(sut.get_value_or(15), Eq(652));
}

TEST_F(expected_test, ArrowOperator)
{
    auto sut = expected<Test, float>::create_value(55, 81);
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->Gimme(), Eq(136));
}

TEST_F(expected_test, ConstArrowOperator)
{
    const expected<Test, float> sut(success<Test>(Test(55, 81)));
    ASSERT_THAT(sut.has_error(), Eq(false));
    EXPECT_THAT(sut->ConstGimme(), Eq(136));
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
    EXPECT_THAT(sut.get_value(), Eq(55));
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

TEST_F(expected_test, OnErrorWhenHavingAnErrorWithResult)
{
    expected<int, float> sut{error<float>(112.1f)};
    float a = 0.2f;
    sut.on_error([&](expected<int, float>& r) { a = r.get_error(); }).on_success([&](expected<int, float>&) {
        a = 2.0f;
    });

    EXPECT_THAT(a, Eq(112.1f));
}

TEST_F(expected_test, ConstOnErrorWhenHavingAnErrorWithResult)
{
    const expected<int, float> sut{error<float>(12.1f)};
    float a = 7.1f;
    sut.on_error([&](expected<int, float>& r) { a = r.get_error(); }).on_success([&](expected<int, float>&) {
        a = 91.f;
    });

    EXPECT_THAT(a, Eq(12.1f));
}

TEST_F(expected_test, VoidOnErrorWhenHavingAnErrorWithResult)
{
    expected<float> sut{error<float>(7112.1f)};
    float a = 70.2f;
    sut.on_error([&](expected<float>& r) { a = r.get_error(); }).on_success([&](expected<float>&) { a = 2.0f; });

    EXPECT_THAT(a, Eq(7112.1f));
}

TEST_F(expected_test, VoidConstOnErrorWhenHavingAnErrorWithResult)
{
    const expected<float> sut{error<float>(612.1f)};
    float a = 67.1f;
    sut.on_error([&](expected<float>& r) { a = r.get_error(); }).on_success([&](expected<float>&) { a = 91.f; });

    EXPECT_THAT(a, Eq(612.1f));
}

TEST_F(expected_test, OnErrorWhenHavingAnErrorWithoutResult)
{
    expected<int, float> sut{error<float>(112.1f)};
    int a = 0;
    sut.on_error([&]() { a = 5; }).on_success([&]() { a = 7; });

    EXPECT_THAT(a, Eq(5));
}

TEST_F(expected_test, ConstOnErrorWhenHavingAnErrorWithoutResult)
{
    const expected<int, float> sut{error<float>(1112.1f)};
    int a = 0;
    sut.on_error([&]() { a = 51; }).on_success([&]() { a = 71; });

    EXPECT_THAT(a, Eq(51));
}

TEST_F(expected_test, VoidOnErrorWhenHavingAnErrorWithoutResult)
{
    expected<float> sut{error<float>(4112.1f)};
    int a = 0;
    sut.on_error([&]() { a = 53; }).on_success([&]() { a = 37; });

    EXPECT_THAT(a, Eq(53));
}

TEST_F(expected_test, VoidConstOnErrorWhenHavingAnErrorWithoutResult)
{
    const expected<float> sut{error<float>(18112.1f)};
    int a = 0;
    sut.on_error([&]() { a = 451; }).on_success([&]() { a = 71; });

    EXPECT_THAT(a, Eq(451));
}

TEST_F(expected_test, OnSuccessWhenHavingSuccessWithResult)
{
    expected<int, float> sut{success<int>(112)};
    int a = 0;
    sut.on_error([&](expected<int, float>&) { a = 3; }).on_success([&](expected<int, float>& r) { a = r.get_value(); });

    EXPECT_THAT(a, Eq(112));
}

TEST_F(expected_test, ConstOnSuccessWhenHavingSuccessWithResult)
{
    const expected<int, float> sut{success<int>(1142)};
    int a = 0;
    sut.on_error([&](expected<int, float>&) { a = 3; }).on_success([&](expected<int, float>& r) { a = r.get_value(); });

    EXPECT_THAT(a, Eq(1142));
}

TEST_F(expected_test, VoidOnSuccessWhenHavingSuccessWithResult)
{
    expected<float> sut{success<>()};
    int a = 0;
    sut.on_error([&](expected<float>&) { a = 3; }).on_success([&](expected<float>& r) { a = (r.has_error()) ? 1 : 2; });

    EXPECT_THAT(a, Eq(2));
}

TEST_F(expected_test, VoidConstOnSuccessWhenHavingSuccessWithResult)
{
    const expected<float> sut{success<>()};
    int a = 0;
    sut.on_error([&](expected<float>&) { a = 3; }).on_success([&](expected<float>& r) {
        a = (r.has_error()) ? 13 : 23;
    });

    EXPECT_THAT(a, Eq(23));
}

TEST_F(expected_test, OnSuccessWhenHavingSuccessWithoutResult)
{
    expected<int, float> sut{success<int>(112)};
    int a = 0;
    sut.on_error([&]() { a = 3; }).on_success([&]() { a = 55; });

    EXPECT_THAT(a, Eq(55));
}

TEST_F(expected_test, ConstOnSuccessWhenHavingSuccessWithoutResult)
{
    const expected<int, float> sut{success<int>(1125)};
    int a = 0;
    sut.on_error([&]() { a = 3; }).on_success([&]() { a = 555; });

    EXPECT_THAT(a, Eq(555));
}

TEST_F(expected_test, VoidOnSuccessWhenHavingSuccessWithoutResult)
{
    expected<float> sut{success<>()};
    int a = 0;
    sut.on_error([&]() { a = 3; }).on_success([&]() { a = 855; });

    EXPECT_THAT(a, Eq(855));
}

TEST_F(expected_test, VoidConstOnSuccessWhenHavingSuccessWithoutResult)
{
    const expected<float> sut{success<>()};
    int a = 0;
    sut.on_error([&]() { a = 3; }).on_success([&]() { a = 8553; });

    EXPECT_THAT(a, Eq(8553));
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
