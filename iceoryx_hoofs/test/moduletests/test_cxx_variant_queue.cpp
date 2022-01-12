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

#include "iceoryx_hoofs/cxx/variant_queue.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::cxx;


class VariantQueue_test : public Test
{
  public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }

    void PerformTestForQueueTypes(const std::function<void(uint64_t typeID)>& testCase) noexcept
    {
        for (uint64_t i = 0; i < numberOfQueueTypes; ++i)
            testCase(i);
    }

    // if a new fifo type is added this variable has to be adjusted
    uint64_t numberOfQueueTypes = 4U;
};

TEST_F(VariantQueue_test, isEmptyWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1055246-9852-4d02-b252-f0251ede278c");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        EXPECT_THAT(sut.empty(), Eq(true));
    });
}

TEST_F(VariantQueue_test, isNotEmptyWhenOneElementIsInside)
{
    ::testing::Test::RecordProperty("TEST_ID", "428a8624-9e5a-4dac-b0be-d49a85d7cdb4");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        sut.push(123);
        EXPECT_THAT(sut.empty(), Eq(false));
    });
}

TEST_F(VariantQueue_test, popsSingleElementWhichWasPushed)
{
    ::testing::Test::RecordProperty("TEST_ID", "9cc943e7-fff2-403a-8a8a-9c821e090ef4");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        sut.push(4123);
        auto element = sut.pop();
        ASSERT_THAT(element.has_value(), Eq(true));
        EXPECT_THAT(element.value(), Eq(4123));
    });
}

TEST_F(VariantQueue_test, popsMultiElementsWhichWerePushed)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2966583-1d8c-4b24-b9b6-cfdc75dc3afb");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        sut.push(14123);
        sut.push(24123);
        sut.push(34123);

        auto element = sut.pop();
        ASSERT_THAT(element.has_value(), Eq(true));
        EXPECT_THAT(element.value(), Eq(14123));

        element = sut.pop();
        ASSERT_THAT(element.has_value(), Eq(true));
        EXPECT_THAT(element.value(), Eq(24123));

        element = sut.pop();
        ASSERT_THAT(element.has_value(), Eq(true));
        EXPECT_THAT(element.value(), Eq(34123));
    });
}

TEST_F(VariantQueue_test, pushTwoElementsAfterSecondPopIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "22cc44ac-bebe-4516-b2fe-290fbefb60b7");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        sut.push(14123);
        sut.push(24123);

        sut.pop();
        sut.pop();

        EXPECT_THAT(sut.pop().has_value(), Eq(false));
    });
}

TEST_F(VariantQueue_test, handlesOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "030f69ae-315e-43b5-83c4-a36c70371397");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 2> sut(static_cast<VariantQueueTypes>(typeID));
        // current SOFI can hold capacity +1 values, so push some more to ensure overflow
        sut.push(14123);
        sut.push(24123);
        sut.push(22222);
        sut.push(33333);
        auto maybeOverflowValue = sut.push(667);
        EXPECT_THAT(maybeOverflowValue.has_value(), Eq(true));
    });
}

TEST_F(VariantQueue_test, noPopWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3ce3ea6-f8e4-47c4-912c-5779b57d64f6");
    PerformTestForQueueTypes([](uint64_t typeID) {
        VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(typeID));
        EXPECT_THAT(sut.pop().has_value(), Eq(false));
    });
}

TEST_F(VariantQueue_test, underlyingTypeIsEmptyWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b8618f8-b0cf-4ef8-bc6d-9bdc330ca09f");
    VariantQueue<int, 5> sut(static_cast<VariantQueueTypes>(0));
    EXPECT_THAT(sut.getUnderlyingFiFo().template get_at_index<0>()->empty(), Eq(true));
}
} // namespace
