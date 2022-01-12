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

#include "iceoryx_hoofs/cxx/attributes.hpp"
#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "test.hpp"


namespace
{
using namespace ::testing;
using namespace iox::cxx;

constexpr int freeFuncTestValue = 42 + 42;
constexpr int functorTestValue = 11;
constexpr int memberFuncTestValue = 4273;
constexpr int sameSignatureIntTestValue = 12345;
constexpr int sameSignatureVoidTestValue = 12346;
constexpr int sameSignatureIntIntTestValue = 12347;

int freeFunction()
{
    return freeFuncTestValue;
}

void freeVoidFunction(int& arg)
{
    arg = freeFuncTestValue;
}

class Functor
{
  public:
    int operator()()
    {
        return functorTestValue;
    }
};

struct ComplexType
{
    char a;
    int b;
    float c;

    bool operator==(const ComplexType& rhs) const
    {
        if (a == rhs.a && b == rhs.b && c == rhs.c)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

ComplexType returnComplexType(ComplexType foo)
{
    return foo;
}

int SameSignature(function_ref<int(int)> callback)
{
    return callback(sameSignatureIntTestValue);
}

int SameSignature(function_ref<int(void)> callback)
{
    return callback();
}

int SameSignature(function_ref<int(int, int)> callback)
{
    return callback(sameSignatureIntIntTestValue, sameSignatureIntIntTestValue);
}

class function_refTest : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    int foobar()
    {
        return memberFuncTestValue;
    }

    uint8_t m_iter{0};
};

using function_refDeathTest = function_refTest;

TEST_F(function_refTest, CreateEmptyIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "32c286a3-4abd-4b2d-a964-556e38726f87");
    function_ref<void()> sut;
    EXPECT_FALSE(sut);
}

TEST_F(function_refDeathTest, CallEmptyLeadsToTermination)
{
    ::testing::Test::RecordProperty("TEST_ID", "631d8765-ba25-49b4-978c-9de3b7059cb5");
    function_ref<void()> sut;
    EXPECT_DEATH(sut(), ".*");
}

TEST_F(function_refTest, CreateValidByAssignIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca09e15d-e05b-443a-b178-2aedba886cdf");
    auto lambda = [] {};
    function_ref<void()> sut;
    sut = lambda;
    EXPECT_TRUE(sut);
}

TEST_F(function_refTest, CallValidByAssignResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "99c66fbd-2df5-48d9-bc89-8394e99c76ba");
    auto lambda = []() -> int { return 7253; };
    function_ref<int()> sut;
    sut = lambda;
    EXPECT_THAT(sut(), Eq(7253));
}

TEST_F(function_refTest, CallValidByCopyConstructResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb30b36d-1c3d-4848-a497-d6d3e72edbd5");
    auto lambda = []() -> int { return 3527; };
    function_ref<int()> sut1{lambda};
    function_ref<int()> sut2(sut1);
    ASSERT_TRUE(sut2);
    EXPECT_THAT(sut2(), Eq(3527));
}

TEST_F(function_refTest, CreateValidByCopyAssignResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb8b568c-06fe-4af2-8d2a-c2527f799ad9");
    auto lambda = []() -> int { return 43; };
    function_ref<int()> sut2;
    {
        function_ref<int()> sut1{lambda};
        EXPECT_THAT(sut1(), Eq(43));
        EXPECT_FALSE(sut2);
        sut2 = sut1;
    }
    EXPECT_THAT(sut2(), Eq(43));
}

TEST_F(function_refTest, CreateInvalidByCopyAssignIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "8500375f-dcbc-439c-8497-d810747dc7a3");
    auto lambda = []() -> int { return 44; };
    function_ref<int()> sut2{lambda};
    EXPECT_THAT(sut2(), Eq(44));
    {
        function_ref<int()> sut1;
        EXPECT_FALSE(sut1);
        sut2 = sut1;
    }
    EXPECT_FALSE(sut2);
}

TEST_F(function_refTest, CreateValidByMoveResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7b5ac66-a703-429b-9e38-44ebcd9a7519");
    auto lambda = []() -> int { return 123; };
    function_ref<int()> sut1{lambda};
    function_ref<int()> sut2{std::move(sut1)};
    ASSERT_TRUE(sut2);
    EXPECT_FALSE(sut1);
    EXPECT_THAT(sut2(), Eq(123));
}

TEST_F(function_refTest, CreateInvalidByMoveIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "bff8bc30-05eb-4714-998a-be4e0a17327f");
    function_ref<void()> sut1;
    function_ref<void()> sut2{std::move(sut1)};
    EXPECT_FALSE(sut2);
}

TEST_F(function_refTest, CreateValidByMoveAssignResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "e641e34f-0e8a-4092-8224-b5f52b964a16");
    auto lambda1 = []() -> int { return 118; };
    auto lambda2 = []() -> int { return 999; };
    function_ref<int()> sut1{lambda1};
    {
        function_ref<int()> sut2{lambda2};
        sut1 = std::move(sut2);
    }
    EXPECT_TRUE(sut1);
    EXPECT_THAT(sut1(), Eq(999));
}

TEST_F(function_refTest, CreateInvalidByMoveAssignIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "7dabac7a-dc3d-415f-8250-363ee196ebb8");
    auto lambda1 = [] {};
    function_ref<void()> sut1{lambda1};
    {
        function_ref<void()> sut2;
        sut1 = std::move(sut2);
    }
    EXPECT_FALSE(sut1);
}

TEST_F(function_refTest, CreateValidAndSwapResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ef9f5f0-c914-4b9d-9841-0c2e153a8451");
    auto lambda1 = []() -> int { return 42; };
    auto lambda2 = []() -> int { return 73; };
    function_ref<int()> sut1(lambda1);
    function_ref<int()> sut2(lambda2);
    EXPECT_THAT(sut1(), Eq(42));
    EXPECT_THAT(sut2(), Eq(73));
    sut1.swap(sut2);
    EXPECT_THAT(sut1(), Eq(73));
    EXPECT_THAT(sut2(), Eq(42));
}

TEST_F(function_refTest, CreateInvalidAndSwapWithValidResultNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdee2c00-ccbd-414d-b404-247a5566b8e2");
    auto lambda2 = []() -> int { return 7331; };
    function_ref<int()> sut1;
    function_ref<int()> sut2(lambda2);
    EXPECT_FALSE(sut1);
    EXPECT_THAT(sut2(), Eq(7331));
    sut1.swap(sut2);
    EXPECT_THAT(sut1(), Eq(7331));
    EXPECT_FALSE(sut2);
}

TEST_F(function_refTest, CreateValidWithCapturingLambdaVoidVoidIncremented)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4d55fdf-2cce-4a8a-bcd7-053f2075304f");
    auto lambda = [&] { m_iter++; };
    function_ref<void(void)> sut(lambda);
    sut();
    EXPECT_THAT(m_iter, Eq(1));
}

TEST_F(function_refTest, CreateValidWithLambdaIntVoidResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "81630738-c0b0-4730-9a2e-b47f5f3b6d22");
    auto lambda = [](void) -> int { return 1337; };
    function_ref<int(void)> sut(lambda);
    EXPECT_THAT(sut(), Eq(1337));
}

TEST_F(function_refTest, CreateValidWithLambdaIntIntIncremented)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ef4025b-b67b-49d8-b607-97dfb9bfa26f");
    auto lambda = [](int var) -> int { return ++var; };
    function_ref<int(int)> sut(lambda);
    EXPECT_THAT(sut(0), Eq(1));
}

TEST_F(function_refTest, CreateValidWithFreeFunctionResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "aaf49b6b-054a-4f8f-b176-6d92bb2918da");
    function_ref<int()> sut(freeFunction);
    EXPECT_THAT(sut(), Eq(freeFuncTestValue));
}

TEST_F(function_refTest, CreateValidWithComplexTypeResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c6a4bf0-989f-4d15-a905-03fddf6d80bc");
    ComplexType fuubar{1, 2, 1.3f};
    function_ref<ComplexType(ComplexType)> sut(returnComplexType);
    EXPECT_THAT(sut(fuubar), Eq(fuubar));
}

TEST_F(function_refTest, CreateValidWithFunctorResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fd3609b-3254-429a-96ae-20f6dbe99b2a");
    Functor foo;
    function_ref<int()> sut(foo);
    EXPECT_THAT(sut(), Eq(functorTestValue));
}

TEST_F(function_refTest, CreateValidWithStdBindResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5f82896-44db-4d2d-96d0-c1b0fbbe5508");
    auto callable = std::bind(&function_refTest::foobar, this);
    function_ref<int()> sut(callable);
    EXPECT_THAT(sut(), Eq(memberFuncTestValue));
}

TEST_F(function_refTest, CreateValidWithStdFunctionResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ed2254b-a8fd-4b16-8f48-195868c044c0");
    std::function<int()> baz;
    baz = []() -> int { return 24; };
    function_ref<int()> sut(baz);
    EXPECT_THAT(sut(), Eq(24));
}

TEST_F(function_refTest, StoreInStdFunctionResultEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "99d9ac22-dddb-44fc-a80b-fe559f6acf63");
    auto lambda = []() -> int { return 37; };
    function_ref<int()> moep(lambda);
    // Moves cxx::function_ref into std::function, needs copy c'tor
    std::function<int()> sut(moep);
    EXPECT_THAT(sut(), Eq(37));
}

TEST_F(function_refTest, CallOverloadedFunctionResultsInCallOfInt)
{
    ::testing::Test::RecordProperty("TEST_ID", "3910ee08-305a-4764-82b3-8b8aa7e7038e");
    auto value = SameSignature([](int value) -> int { return value; });
    EXPECT_THAT(value, Eq(sameSignatureIntTestValue));
}

TEST_F(function_refTest, CallOverloadedFunctionResultsInCallOfVoid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca8e8384-0b20-4e4a-b372-698c4e6672b7");
    auto value = SameSignature([](void) -> int { return sameSignatureVoidTestValue; });
    EXPECT_THAT(value, Eq(sameSignatureVoidTestValue));
}

TEST_F(function_refTest, CallOverloadedFunctionResultsInCallOfIntInt)
{
    ::testing::Test::RecordProperty("TEST_ID", "b37158b6-8100-4f80-bd62-d2957a7d9c46");
    auto value = SameSignature([](int value1, int value2 IOX_MAYBE_UNUSED) -> int { return value1; });
    EXPECT_THAT(value, Eq(sameSignatureIntIntTestValue));
}

TEST_F(function_refTest, CreationWithFunctionPointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "082bd86b-85d8-478b-b723-3d1f0db2d01d");
    constexpr auto fp = &freeFunction;
    function_ref<int(void)> sut(fp);

    ASSERT_TRUE(sut.operator bool());
    auto result = sut();
    EXPECT_EQ(result, freeFuncTestValue);
}

TEST_F(function_refTest, CreationWithFunctionPointerWithRefArgWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d75aa14-9743-49ee-b80c-b47b1326b96b");
    constexpr auto fp = &freeVoidFunction;
    function_ref<void(int&)> sut(fp);

    ASSERT_TRUE(sut.operator bool());
    int arg{0};
    sut(arg);
    EXPECT_EQ(arg, freeFuncTestValue);
}

TEST_F(function_refTest, CreationWithFunctionPointerWithComplexTypeArgWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "769a00c8-9187-4cff-b352-790311c2c42f");
    constexpr auto fp = &returnComplexType;
    function_ref<ComplexType(ComplexType)> sut(fp);

    ASSERT_TRUE(sut.operator bool());
    ComplexType arg{1, 2, 3.3};
    auto result = sut(arg);
    EXPECT_EQ(result, arg);
}

TEST_F(function_refTest, CreationWithFunctionNullPointerIsNotCallable)
{
    ::testing::Test::RecordProperty("TEST_ID", "e10c9dd4-d8a2-4231-a6c0-0e93e65e1ae0");
    int (*fp)(int) = nullptr;
    function_ref<int(int)> sut(fp);

    EXPECT_FALSE(sut.operator bool());
}

} // namespace
