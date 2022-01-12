// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/concurrent/taco.hpp"
#include "test.hpp"

#include <iostream>

namespace
{
using namespace ::testing;

struct TestData
{
    TestData() = default;
    constexpr TestData(uint32_t f_index, uint32_t f_counter, uint64_t f_timestamp)
        : index(f_index)
        , counter(f_counter)
        , timestamp(f_timestamp)
    {
    }
    uint32_t index;
    uint32_t counter;
    uint64_t timestamp;

    bool operator==(const TestData& rhs) const
    {
        return index == rhs.index && counter == rhs.counter && timestamp == rhs.timestamp;
    }
};

enum class Context : uint32_t
{
    Huey,
    Dewey,
    Louie,
    END_OF_LIST
};

class TACO_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    using MyTACO = iox::concurrent::TACO<TestData, Context>;
};

TEST_F(TACO_test, Initialized_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8b7484d-2a98-4e2c-a994-4a4722f0c95f");
    // TACO must be empty when initialized
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    auto retVal = taco.take(Context::Huey);
    EXPECT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, Initialized_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "e006901e-72d4-44ff-a0d9-f9f181e12d37");
    // TACO must be empty when initialized
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    auto retVal = taco.take(Context::Huey);
    EXPECT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, StoreAndTake_FromSameContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "52a4be5a-ff7b-4a5e-8e1d-b8cd53a0a164");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.take(Context::Huey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, StoreAndTake_FromDifferentContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "040f3ce7-8b5c-40c9-a3af-47c640af2e49");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.take(Context::Dewey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, StoreAndTake_FromSameContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "83da385c-2887-4c50-80c9-c7f888094d4a");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.take(Context::Huey);

    EXPECT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, StoreAndTake_FromDifferentContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "5081f704-6f73-4185-8b8f-10ed9a1182ec");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.take(Context::Dewey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, MultipleStoresSingleTake_FromSameContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "599b3558-f9dc-4b50-b3d5-25f6cf2bf4b9");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData_1{1, 42, 73};
    constexpr TestData ExpectedData_2{13, 111, 666};

    taco.store(ExpectedData_1, Context::Huey);
    taco.store(ExpectedData_2, Context::Huey);
    auto retVal = taco.take(Context::Huey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData_2));
}

TEST_F(TACO_test, MultipleStoresSingleTake_FromDifferentContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5e99d96-4071-4b6f-93ec-c52b5d1c5670");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData_1{1, 42, 73};
    constexpr TestData ExpectedData_2{13, 111, 666};

    taco.store(ExpectedData_1, Context::Huey);
    taco.store(ExpectedData_2, Context::Dewey);
    auto retVal = taco.take(Context::Louie);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData_2));
}

TEST_F(TACO_test, MultipleStoresSingleTake_FromSameContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "71fe8d30-f4c9-4f4e-a6e3-70b8bffb48da");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData_1{1, 42, 73};
    constexpr TestData ExpectedData_2{13, 111, 666};

    taco.store(ExpectedData_1, Context::Huey);
    taco.store(ExpectedData_2, Context::Huey);
    auto retVal = taco.take(Context::Huey);

    ASSERT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, MultipleStoresSingleTake_FromDifferentContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "145b09c0-3019-4dbb-9efb-814df294748a");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData_1{1, 42, 73};
    constexpr TestData ExpectedData_2{13, 111, 666};

    taco.store(ExpectedData_1, Context::Huey);
    taco.store(ExpectedData_2, Context::Dewey);
    auto retVal = taco.take(Context::Louie);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData_2));
}

TEST_F(TACO_test, DoubleTake)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5d3329e-ddf6-40f0-a9d7-af62b5826950");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};

    taco.store(ExpectedData, Context::Huey);
    taco.take(Context::Dewey);
    auto retVal = taco.take(Context::Louie);

    EXPECT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, Exchange_FromSameContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcb9a21e-f5e1-4d82-8056-91c69ddedde1");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};
    constexpr TestData DummyData{37, 4242, 123456};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.exchange(DummyData, Context::Huey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, Exchange_FromDifferentContext_SameContextAllowed)
{
    ::testing::Test::RecordProperty("TEST_ID", "3448ed3b-ff43-4e02-be73-335350b8df47");
    MyTACO taco(iox::concurrent::TACOMode::AccecptDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};
    constexpr TestData DummyData{37, 4242, 123456};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.exchange(DummyData, Context::Dewey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, Exchange_FromSameContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "e930f8ef-12cc-4469-8394-b38d09e4a492");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};
    constexpr TestData DummyData{37, 4242, 123456};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.exchange(DummyData, Context::Huey);

    ASSERT_THAT(retVal.has_value(), Eq(false));
}

TEST_F(TACO_test, Exchange_FromDifferentContext_SameContextDenied)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f584a4e-a197-460b-b0ba-415c772339d6");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData{1, 42, 73};
    constexpr TestData DummyData{37, 4242, 123456};

    taco.store(ExpectedData, Context::Huey);
    auto retVal = taco.exchange(DummyData, Context::Dewey);

    ASSERT_THAT(retVal.has_value(), Eq(true));
    EXPECT_THAT(*retVal, Eq(ExpectedData));
}

TEST_F(TACO_test, DoubleExchange)
{
    ::testing::Test::RecordProperty("TEST_ID", "134a9f18-9a3c-47dc-9882-a18664501db5");
    MyTACO taco(iox::concurrent::TACOMode::DenyDataFromSameContext);
    constexpr TestData ExpectedData_1{1, 42, 73};
    constexpr TestData ExpectedData_2{13, 111, 666};
    constexpr TestData DummyData{37, 4242, 123456};

    taco.store(ExpectedData_1, Context::Huey);
    auto retVal_1 = taco.exchange(ExpectedData_2, Context::Dewey);
    auto retVal_2 = taco.exchange(DummyData, Context::Louie);

    ASSERT_THAT(retVal_1.has_value(), Eq(true));
    EXPECT_THAT(*retVal_1, Eq(ExpectedData_1));

    ASSERT_THAT(retVal_2.has_value(), Eq(true));
    EXPECT_THAT(*retVal_2, Eq(ExpectedData_2));
}
} // namespace
