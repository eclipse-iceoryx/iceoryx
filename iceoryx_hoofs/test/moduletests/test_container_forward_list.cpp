// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/attributes.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"
#include "iox/forward_list.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/lifetime_and_assignment_tracker.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

constexpr uint64_t TESTLISTCAPACITY{10U};
constexpr int64_t TEST_LIST_ELEMENT_DEFAULT_VALUE{-99L};

class forward_list_test : public Test
{
  public:
    using TestListElement = LifetimeAndAssignmentTracker<int64_t, TEST_LIST_ELEMENT_DEFAULT_VALUE>;

    void SetUp() override
    {
        stats.reset();
    }

    TestListElement::Statistics& stats = TestListElement::stats;

    static bool isSetupState()
    {
        TestListElement::Statistics& stats = TestListElement::stats;
        return (stats.cTor == 0U && stats.customCTor == 0U && stats.copyCTor == 0U && stats.moveCTor == 0U
                && stats.moveAssignment == 0U && stats.copyAssignment == 0U && stats.dTor == 0U
                && stats.classValue == 0);
    }

    forward_list<TestListElement, TESTLISTCAPACITY> sut;
};

template <typename IterType>
int64_t iteratorTraitReturnDoubleValue(IterType iter)
{
    typedef typename std::iterator_traits<IterType>::value_type IterValueType;
    IterValueType value = *iter;
    return (2 * value); // will only work for integer-convertible value types
}
} // namespace


TEST_F(forward_list_test, NewlyCreatedListIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ec4ce8b-5d4a-4c33-97d2-04798b2fac26");
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(forward_list_test, NewlyCreatedListHasSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8079160-cf12-4f60-a6cc-6b0e1dab090a");
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(forward_list_test, ReadCapacityOnList)
{
    ::testing::Test::RecordProperty("TEST_ID", "49e66cb5-1e2a-4385-b788-1d9c3919f399");
    EXPECT_THAT(sut.capacity(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, ReadMax_sizeOnList)
{
    ::testing::Test::RecordProperty("TEST_ID", "0465ab1b-e976-4efb-82e1-0bf271248f24");
    EXPECT_THAT(sut.max_size(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, NewListCTorWithZeroElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "480ba421-8b15-4f17-b148-50101bb4b6b5");
    constexpr uint64_t CAPACITY{42U};
    EXPECT_THAT(isSetupState(), Eq(true));
    const forward_list<int, CAPACITY> sut1;
    EXPECT_THAT(sut1.empty(), Eq(true));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(forward_list_test, CbeginCendAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "567fee5d-3982-4d34-892b-0be2b24b5f50");
    EXPECT_THAT(sut.cbegin() == sut.cend(), Eq(true));
}
TEST_F(forward_list_test, BeginEndAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "915f88e6-560c-465d-a9b6-0ddfdf2abef6");
    EXPECT_THAT(sut.begin() == sut.end(), Eq(true));
}
TEST_F(forward_list_test, CbeginEndAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "f341d5db-f922-474c-9c0e-5a280c707d99");
    EXPECT_THAT(sut.cbegin() == sut.end(), Eq(true));
}
TEST_F(forward_list_test, BeginCendAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2aa9e76-566b-407c-b51c-90a4ca8536a3");
    EXPECT_THAT(sut.begin() == sut.cend(), Eq(true));
}

TEST_F(forward_list_test, CbeforebeginAndCbeginAreDifferentWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "caa528dc-5dfc-45de-a434-d072da0cb7cf");
    EXPECT_THAT(sut.cbefore_begin() != sut.cbegin(), Eq(true));
}
TEST_F(forward_list_test, beforebeginAndBeginAreDifferentWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "daa2cd58-f159-4517-81a4-87cf290d6f21");
    EXPECT_THAT(sut.before_begin() != sut.begin(), Eq(true));
}
TEST_F(forward_list_test, CbeforeBeginAndBeginAreDifferentWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ab6073e-bd7c-47bf-838e-46936a2201a4");
    EXPECT_THAT(sut.cbefore_begin() != sut.begin(), Eq(true));
}
TEST_F(forward_list_test, beforeBeginAndCbeginAreDifferentWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "dcfed4d8-57dc-4154-b7e3-d7a9233d59ae");
    EXPECT_THAT(sut.before_begin() != sut.cbegin(), Eq(true));
}

TEST_F(forward_list_test, CbeginCendAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "a8e9c012-6ee8-42fb-9851-cab9757bd987");
    EXPECT_THAT(sut.emplace_front().value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT(sut.cbegin() != sut.cend(), Eq(true));
}
TEST_F(forward_list_test, BeginEndAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "1443c9c0-309a-4ee4-8076-a9e86a599b78");
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.end(), Eq(true));
}
TEST_F(forward_list_test, CbeginEndAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "0dbc72ba-2e5e-4f19-8963-cdb5d0bd4337");
    sut.emplace_front();
    EXPECT_THAT(sut.cbegin() != sut.end(), Eq(true));
}
TEST_F(forward_list_test, BeginCendAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "a340bad6-e367-4e93-ad4c-d9abc5efe77e");
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.cend(), Eq(true));
}

TEST_F(forward_list_test, NotEmptyWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "f667de49-7a0c-4dab-9bf5-1bfa9d8e0e93");
    sut.emplace_front();
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(forward_list_test, NotFullWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cac1c83-6a47-43f2-9c27-27db4adbf104");
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(forward_list_test, NotFullWhenPartialFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb28b9a2-55a2-4f05-9dcc-ec98dba613c3");
    sut.emplace_front();
    EXPECT_THAT(TESTLISTCAPACITY, Gt(1U));
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(forward_list_test, FullWhenFilledWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "03487ca4-71f8-4e8c-abdc-ed2cd0867b06");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        EXPECT_THAT(sut.emplace_front().value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    }
    EXPECT_THAT(sut.full(), Eq(true));
}
TEST_F(forward_list_test, FullWhenFilledWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "d9d0f7f8-9310-416a-b3d7-1e41fd41fe3e");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_front();
    }

    EXPECT_THAT(sut.full(), Eq(true));
    IOX_EXPECT_FATAL_FAILURE([&] { sut.emplace_front(); }, iox::er::ENFORCE_VIOLATION);
}
TEST_F(forward_list_test, NotFullWhenFilledWithCapacityAndEraseOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4d0e138-fe26-4796-8f7b-5e2ab736535a");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_front();
    }
    sut.erase_after(sut.cbefore_begin());

    EXPECT_THAT(sut.size(), Eq(sut.capacity() - 1U));
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(forward_list_test, NotFullWhenFilledWithCapacityAndEraseOneAndReinsertElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "7df1e41e-f3c2-4ea2-9ed1-ed68a5342ce2");
    int64_t counter = 0;
    for (; static_cast<uint64_t>(counter) < sut.capacity(); ++counter)
    {
        sut.emplace_front(counter);
    }
    sut.pop_front();
    sut.pop_front();
    sut.emplace_front(counter);
    sut.emplace_front(++counter);

    for (auto& element : sut)
    {
        EXPECT_THAT(element, Eq(counter));
        if (static_cast<uint64_t>(counter) == sut.capacity())
        {
            counter -= 3;
        }
        else
        {
            --counter;
        }
    }

    EXPECT_THAT(sut.size(), Eq(sut.capacity()));
    EXPECT_THAT(sut.full(), Eq(true));
}

TEST_F(forward_list_test, CTorWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff5e1018-2b8d-443d-bc32-8ab673eca43d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    forward_list<TestListElement, CAPACITY> sut1;

    EXPECT_THAT(stats.cTor, Eq(0U));
    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CustomCTorWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "56251d67-94f6-406d-9448-3224a58df4bb");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    constexpr int64_t DEFAULT_VALUE{3};
    forward_list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front(DEFAULT_VALUE);
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.classValue, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, CTorWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "d389fee1-a86d-4a29-a307-b3770b64e17d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    forward_list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CTorWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "2cdb47bc-2704-4a9f-8425-9783504a4c12");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CTorWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "1882dc91-9d0a-42e5-bcfe-1259772d795e");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.push_front({});
    }
    sut1.emplace_after(sut1.cbefore_begin(), 2U);

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(CAPACITY));
    EXPECT_THAT(stats.customCTor, Eq(0U));
}


TEST_F(forward_list_test, EmplaceAfterWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "89baa6c4-7c31-471f-86b8-5ca607ad8937");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    decltype(TestListElement::value) cnt = 0U;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0U;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4dbfaed-6077-40e9-b141-4f50f71d3000");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1b8898b-7b15-447c-8d17-4a81ab7a3e9e");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0U;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8fb7dc7-c91b-44a1-a596-06a8ecefd44d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(CAPACITY));
}


TEST_F(forward_list_test, EmplaceAfterReverseWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6de806a-7911-4535-997c-018e34cebb2b");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    --cnt;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "32fccdae-93cc-4de1-a7b3-638758744070");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    --cnt;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "2480c086-673a-4003-9814-f3fdeb86c5fe");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY - 1;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "a17d34f7-a454-402b-a42e-c2b87283c58f");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1U};
    forward_list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbefore_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY - 1;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(CAPACITY));
}

TEST_F(forward_list_test, EmplaceAfterWithWrongListIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ea12c2d-c942-45f7-a642-364b21267413");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{13U};
    forward_list<TestListElement, CAPACITY> sut11;
    forward_list<TestListElement, CAPACITY> sut12;
    auto iterOfSut11 = sut11.before_begin();
    auto iterOfSut12 = sut12.before_begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut11.emplace_after(iterOfSut11, cnt);
        ++cnt;
    }

    IOX_EXPECT_FATAL_FAILURE([&] { sut11.emplace_after(iterOfSut12, cnt); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, PushFrontConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "9caef076-4533-4179-b2e5-b75ce5090f5b");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontConstSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "121c8781-9697-44cf-828b-cec9f0816e8f");
    const TestListElement a{};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(1U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT((*sut.begin()).value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontFailsWhenSpaceNotAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "4d9dfebf-0f25-4472-afb1-a38ecf8322b7");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    EXPECT_THAT(sut.size(), Eq(0U));
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_front(a));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }
    EXPECT_FALSE(sut.push_front(a));
    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, PushFrontSuccessfullWhenSpaceAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "65dcef65-f68c-4268-aec1-b32439137f95");
    constexpr int64_t DEFAULT_VALUE{13L};

    EXPECT_THAT(sut.size(), Eq(0U));

    sut.push_front(TestListElement{DEFAULT_VALUE});
    EXPECT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontFailsWhenSpaceNotAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0aa7e3ae-13ba-4ef9-8a2e-afca53bd1aaa");
    constexpr int64_t DEFAULT_VALUE{13L};

    EXPECT_THAT(sut.size(), Eq(0U));
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_front(DEFAULT_VALUE));
    }

    EXPECT_FALSE(sut.push_front(DEFAULT_VALUE));

    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(DEFAULT_VALUE));
    }
}


TEST_F(forward_list_test, AccessFrontElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf08eb83-fcc0-4418-b290-2b86b3dfc70a");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    TestListElement& b{sut.front()};
    const TestListElement& c{sut.front()};
    EXPECT_THAT(b.value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, AccessFrontElementFromConstList)
{
    ::testing::Test::RecordProperty("TEST_ID", "bfa365c4-bcea-46e7-88b7-d2818304bad4");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    const forward_list<TestListElement, TESTLISTCAPACITY> sut1{sut};
    const TestListElement& c = sut1.front();

    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(sut1.front().value, Eq(DEFAULT_VALUE));
}


TEST_F(forward_list_test, PopFrontOnEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "95940c1d-bbe5-4a22-9464-34c7141f2c54");
    EXPECT_FALSE(sut.pop_front());
    ASSERT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(forward_list_test, PopFrontNonEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "33cb4d4f-b04e-4664-a5bf-8c2e5866197b");
    constexpr uint32_t ELEMENT = 101U;
    sut.emplace_front(ELEMENT);
    ASSERT_THAT(sut.size(), Eq(1U));

    EXPECT_TRUE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.dTor, Eq(1U));
}

TEST_F(forward_list_test, PopFrontFullToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f4ca087-49b2-4a15-b626-37ae0dc912f8");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_front();
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_front());
    }

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, PopFrontFullPlusOneToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "50723dcc-4d9a-4ff6-af9a-899eff3a61c9");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_front();
        EXPECT_THAT(sut.size(), Eq((i + 1U) > TESTLISTCAPACITY ? TESTLISTCAPACITY : (i + 1U)));
    }

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_front());
    }

    EXPECT_FALSE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.customCTor, Eq(0U));
    ASSERT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
}


TEST_F(forward_list_test, InsertAfterEmptyListAsLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad042f38-c3b6-489a-a79d-7be730595fde");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(sut.size(), Eq(1U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.copyCTor, Eq(1U));
    ASSERT_THAT(stats.moveCTor, Eq(0U));
    ASSERT_THAT(stats.copyAssignment, Eq(0U));
    ASSERT_THAT(stats.moveAssignment, Eq(0U));
}

TEST_F(forward_list_test, InsertAfterLValueCheckReturn)
{
    ::testing::Test::RecordProperty("TEST_ID", "17d6a817-66de-425d-925d-fef62c528dbd");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    auto iter = sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(iter == sut.begin(), Eq(true));
}

TEST_F(forward_list_test, InsertAfterEmptyListAsRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef2e7386-315e-42b3-95e0-e08fcf387d1c");
    constexpr uint32_t DATA = 10U;
    sut.insert_after(sut.before_begin(), {DATA});

    ASSERT_THAT(sut.size(), Eq(1U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.copyCTor, Eq(0U));
    ASSERT_THAT(stats.moveCTor, Eq(1U));
    ASSERT_THAT(stats.copyAssignment, Eq(0U));
    ASSERT_THAT(stats.moveAssignment, Eq(0U));
}

TEST_F(forward_list_test, InsertAfterRValueCheckReturn)
{
    ::testing::Test::RecordProperty("TEST_ID", "37db60b9-1222-4339-aabb-e8d2db11403c");
    constexpr uint32_t DATA = 10U;
    auto iter = sut.insert_after(sut.before_begin(), {DATA});

    ASSERT_THAT(iter == sut.begin(), Eq(true));
    ASSERT_THAT((*iter).value, Eq(DATA));
}

TEST_F(forward_list_test, InsertAfterBeginListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ff7fdbd-193c-488b-9b9c-5f9b10f4bd45");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert_after(sut.begin(), a);

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
}


TEST_F(forward_list_test, InsertAfterBeforeBeginListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "9cfdc926-ee3f-43c2-bf59-6dce94cd052d");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
}

TEST_F(forward_list_test, InsertAfterBeforeBeginListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "76b17794-2ac6-4725-9192-88023e8650bc");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.emplace_front(a);
    sut.insert_after(sut.before_begin(), {});

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, InsertAfterSomeElementsListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "66930a3c-338d-438e-9077-6b89df04a533");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    uint64_t loopCounter = 0U;

    // fill half
    for (int64_t i = 0; i < static_cast<int64_t>(TESTLISTCAPACITY) / 2; ++i)
    {
        sut.emplace_front(i);
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    auto iter = sut.begin();
    // 2 increments
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, a);

    for (auto& x [[maybe_unused]] : sut)
    {
        ++loopCounter;
    }

    ASSERT_THAT(sut.size(), Eq(6U));
    ASSERT_THAT(loopCounter, Eq(6U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(6U));

    iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(4U));
    EXPECT_THAT((++iter)->value, Eq(3U));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}

TEST_F(forward_list_test, InsertAfterSomeElementsListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e30b2576-0625-44f0-95c9-f6084ff2147e");
    constexpr int64_t DEFAULT_VALUE{13L};

    // fill half
    for (uint64_t i = 0U; i < TESTLISTCAPACITY / 2U; ++i)
    {
        sut.emplace_front(static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    auto iter = sut.begin();
    // 2 increments
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(6U));

    iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(4U));
    EXPECT_THAT((++iter)->value, Eq(3U));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}


TEST_F(forward_list_test, InsertAfterFullElementsListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "97a3c126-e5fb-40db-a65f-725a0143e167");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    int64_t cnt = 0U;

    auto iter = sut.before_begin();

    // fill full-1
    for (uint64_t i = 0U; i < TESTLISTCAPACITY - 1; ++i)
    {
        iter = sut.emplace_after(iter, static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    sut.insert_after(iter, a);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1L == cnt)
        {
            // for the last element (insert_after) check for different value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(forward_list_test, InsertAfterFullElementsListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "46a0d274-4929-42c5-a427-b76e6941ad86");
    constexpr int64_t DEFAULT_VALUE{13L};
    int64_t cnt = 0U;

    auto iter = sut.before_begin();

    // fill full-1
    for (uint64_t i = 0U; i < TESTLISTCAPACITY - 1; ++i)
    {
        iter = sut.emplace_after(iter, static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1L == cnt)
        {
            // for the last element (insert_after) check for different value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(forward_list_test, IteratorArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed2c903e-ab2e-4c44-abb1-bf1db2839863");
    constexpr int64_t DEFAULT_VALUE{13L};

    // fill half
    for (uint64_t i = 0U; i < TESTLISTCAPACITY / 2U; ++i)
    {
        sut.emplace_front(static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    auto iter = sut.cbegin();
    // 2 increments
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(6U));

    iter = sut.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(4U));
    EXPECT_THAT((++iter)->value, Eq(3U));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}

TEST_F(forward_list_test, IteratorIncrementOperatorBeyondEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "4cc22ba8-a201-48c1-9d6e-1c9100b89874");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}

TEST_F(forward_list_test, ConstIteratorIncrementOperatorBeyondEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "da3e61d7-e19c-40c0-984c-9cc5044db019");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.cbegin();
    EXPECT_THAT(iter->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}


TEST_F(forward_list_test, IteratorComparisonOfDifferentLists)
{
    ::testing::Test::RecordProperty("TEST_ID", "85e3a092-b256-4ef2-ae33-087935d96483");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842U);
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    sut12.emplace_front(1313U);
    sut12.emplace_front(13131U);


    auto iterSut1 = sut11.begin();
    auto iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.before_begin();
    iterSut2 = sut12.before_begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.end();
    iterSut2 = sut12.end();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.begin();
    iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.before_begin();
    iterSut2 = sut12.before_begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.end();
    iterSut2 = sut12.end();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);
}


TEST_F(forward_list_test, ComparingConstIteratorAndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e7922f5-1832-42f6-a062-f3cd20fe2750");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842U);
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    sut12.emplace_front(1313U);
    sut12.emplace_front(13131U);


    forward_list<TestListElement, TESTLISTCAPACITY>::const_iterator iterSut1 = sut11.cbefore_begin();
    forward_list<TestListElement, TESTLISTCAPACITY>::const_iterator iterSut2 = sut11.cbefore_begin();
    forward_list<TestListElement, TESTLISTCAPACITY>::iterator iterSut3 = sut11.begin();
    forward_list<TestListElement, TESTLISTCAPACITY>::iterator iterSut4 = sut11.end();

    ASSERT_THAT(iterSut1 == iterSut3, Eq(false));
    ASSERT_THAT(iterSut3 == iterSut1, Eq(false));

    ASSERT_THAT(iterSut1 == iterSut2, Eq(true));
    ASSERT_THAT(iterSut4 == iterSut3, Eq(false));
}


TEST_F(forward_list_test, IteratorTraitsGetValueType)
{
    ::testing::Test::RecordProperty("TEST_ID", "f3f2b8c8-50e0-4e4f-aa9a-74eff3a2014a");
    forward_list<int, TESTLISTCAPACITY> sut1;

    sut1.emplace_front(static_cast<int32_t>(TESTLISTCAPACITY) / 2);
    auto iter{sut1.begin()};

    // using a function call here is closer to the actual use case (-> intentionally did not inline all code here)
    auto ret = iteratorTraitReturnDoubleValue(iter);

    EXPECT_THAT(ret, Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, IteratorTraitsCheckIteratorCategoryOnConstIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "ffbb06eb-5267-45e0-91e4-6172a27a3489");
    auto iter [[maybe_unused]] = sut.cbefore_begin();
    ASSERT_NE(typeid(std::iterator_traits<decltype(iter)>::iterator_category), typeid(std::random_access_iterator_tag));
    EXPECT_EQ(typeid(std::iterator_traits<decltype(iter)>::iterator_category), typeid(std::forward_iterator_tag));
}

TEST_F(forward_list_test, EmptyAfterClear)
{
    ::testing::Test::RecordProperty("TEST_ID", "3970e5a4-0ac3-4acf-b5ef-40c96125039b");
    sut.emplace_front(4U);
    sut.clear();
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(forward_list_test, SizeZeroAfterClear)
{
    ::testing::Test::RecordProperty("TEST_ID", "410dc821-1b7f-4cf3-bdad-99e9d15c74db");
    sut.emplace_front(4U);
    sut.clear();
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(forward_list_test, CopyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "792edf58-e345-4fea-bd41-35144da1b514");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    constexpr uint32_t ELEMENT1 = 101U;
    constexpr uint32_t ELEMENT2 = 102U;
    sut11.emplace_front(ELEMENT1);
    sut11.emplace_front(ELEMENT2);
    EXPECT_THAT(stats.customCTor, Eq(2U));

    forward_list<TestListElement, TESTLISTCAPACITY> sut12(sut11);

    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(2U));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    auto iter = sut12.begin();
    EXPECT_THAT(iter->value, Eq(ELEMENT2));
    ++iter;
    EXPECT_THAT(iter->value, Eq(ELEMENT1));
    EXPECT_THAT(sut12.empty(), Eq(false));
    EXPECT_THAT(sut12.size(), Eq(2U));
}

TEST_F(forward_list_test, CopyConstructorWithEmptyForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ca26707-e1a6-4bf1-b795-67ee4efac72f");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization) copy constructor shall be tested
    forward_list<TestListElement, TESTLISTCAPACITY> sut12(sut11);
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(sut12.size(), Eq(0U));
    EXPECT_THAT(sut12.empty(), Eq(true));
}

TEST_F(forward_list_test, CopyConstructorWithFullForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "60f40098-0565-4276-810c-86d7e1685f2d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    decltype(TestListElement::value) i = 0U;

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut11.emplace_front(static_cast<int64_t>(i));
    }

    forward_list<TestListElement, TESTLISTCAPACITY> sut12(sut11);
    for (auto& listElement : sut12)
    {
        listElement.value = i;
        ++i;
    }

    EXPECT_THAT(stats.copyCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(i, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.empty(), Eq(false));
}

TEST_F(forward_list_test, MoveConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "7579f4ae-e00d-46b4-b3fb-9ecbf5c7fc3e");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    constexpr uint32_t ELEMENT1 = 8101U;
    constexpr uint32_t ELEMENT2 = 8102U;
    sut11.emplace_front(ELEMENT1);
    sut11.emplace_front(ELEMENT2);

    forward_list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.moveCTor, Eq(2U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    auto iter = sut12.begin();
    EXPECT_THAT(iter->value, Eq(ELEMENT2));
    EXPECT_THAT((++iter)->value, Eq(ELEMENT1));
    EXPECT_THAT(sut12.empty(), Eq(false));
    EXPECT_THAT(sut12.size(), Eq(2U));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved object
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_THAT(sut11.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveConstructorWithEmptyForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4b0fa37-fd3d-424a-9564-6c13f4eb4593");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(sut12.size(), Eq(0U));
    EXPECT_THAT(sut12.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveConstructorWithFullForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "4794ef99-0115-4fba-9615-fb75f7185455");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut11.emplace_front(static_cast<int64_t>(i));
    }

    forward_list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));

    EXPECT_THAT(stats.moveCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.empty(), Eq(false));
}

TEST_F(forward_list_test, DestructorWithEmptyForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "f076d174-cb08-414f-bdfa-ff00411079ba");
    {
        forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    }
    EXPECT_THAT(stats.dTor, Eq(0U));
}

TEST_F(forward_list_test, DestructorSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "3740a69c-f4c9-41eb-8bcd-1c91e0c156e4");
    {
        forward_list<TestListElement, TESTLISTCAPACITY> sut11;
        sut11.emplace_front(891U);
        sut11.emplace_front(9191U);
        sut11.emplace_front(1U);
    }
    EXPECT_THAT(stats.cTor, Eq(0U));

    EXPECT_THAT(stats.customCTor, Eq(3U));
    EXPECT_THAT(stats.dTor, Eq(3U));
}

TEST_F(forward_list_test, DestructorWithFullForwardList)
{
    ::testing::Test::RecordProperty("TEST_ID", "278e77ee-8cb0-44e9-b42a-4221d6f1a257");
    {
        forward_list<TestListElement, TESTLISTCAPACITY> sut11;
        for (uint64_t i = 0U; i < sut11.capacity(); ++i)
        {
            sut11.emplace_front(1231U);
        }
    }

    EXPECT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, CopyAssignmentWithEmptySource)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b5dc0b4-7b0b-43c9-8608-50b0f0b921ae");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(812U);
    sut11.emplace_front(81122U);
    sut11.emplace_front(8132U);

    sut11 = sut12;
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(sut11.empty(), Eq(true));
}

TEST_F(forward_list_test, CopyAssignmentWithEmptyDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "72b62a1a-da62-46fe-ac04-9ec107947d0d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    constexpr uint32_t ELEMENT1 = 5812U;
    constexpr uint32_t ELEMENT2 = 581122U;
    constexpr uint32_t ELEMENT3 = 58132U;
    sut11.emplace_front(ELEMENT1);
    sut11.emplace_front(ELEMENT2);
    sut11.emplace_front(ELEMENT3);

    sut12 = sut11;
    EXPECT_THAT(stats.dTor, Eq(0U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    EXPECT_THAT(stats.copyCTor, Eq(3U));
    EXPECT_THAT(sut12.size(), Eq(3U));
    EXPECT_THAT(sut12.empty(), Eq(false));

    auto iter = sut12.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(ELEMENT3));
    EXPECT_THAT((++iter)->value, Eq(ELEMENT2));
    EXPECT_THAT((++iter)->value, Eq(ELEMENT1));
}


TEST_F(forward_list_test, CopyAssignmentWithLargerDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a5e2ddd-a9a8-4852-89db-8958aeb84a7a");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(5842U);
    sut11.emplace_front(584122U);
    sut11.emplace_front(58432U);
    sut11.emplace_front(58432U);

    sut12.emplace_front(313U);
    sut12.emplace_front(3131U);

    sut11 = sut12;

    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(stats.copyAssignment, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(sut11.size(), Eq(2U));
    EXPECT_THAT(sut11.empty(), Eq(false));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(3131U));
    EXPECT_THAT((++iter)->value, Eq(313U));
}

TEST_F(forward_list_test, CopyAssignmentWithLargerSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "be223c46-144b-42fa-9bbe-e17625754c7d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842U);
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    sut12.emplace_front(1313U);
    sut12.emplace_front(13131U);

    sut12 = sut11;

    EXPECT_THAT(stats.dTor, Eq(0U));
    EXPECT_THAT(stats.copyAssignment, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(2U));
    EXPECT_THAT(sut12.size(), Eq(4U));
    EXPECT_THAT(sut12.empty(), Eq(false));

    auto iter = sut12.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}


TEST_F(forward_list_test, MoveAssignmentWithEmptySource)
{
    ::testing::Test::RecordProperty("TEST_ID", "ccd0dd16-b472-4f49-b83e-695b86c6e4f3");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(812U);
    sut11.emplace_front(81122U);
    sut11.emplace_front(8132U);

    sut11 = std::move(sut12);

    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(sut11.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveAssignmentWithEmptyDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "650eee96-687a-49b9-90fe-d0d9fa7e4eba");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(5812U);
    sut11.emplace_front(581122U);
    sut11.emplace_front(58132U);

    sut12 = std::move(sut11);

    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.moveCTor, Eq(3U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(3U));

    EXPECT_THAT(sut12.size(), Eq(3U));
    EXPECT_THAT(sut12.empty(), Eq(false));

    auto iter = sut12.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(58132U));
    EXPECT_THAT((++iter)->value, Eq(581122U));
    EXPECT_THAT((++iter)->value, Eq(5812U));
}


TEST_F(forward_list_test, MoveAssignmentWithLargerDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "f85ecbbf-a08f-46ee-9a54-edef0fb6291d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(5842U);
    sut11.emplace_front(584122U);
    sut11.emplace_front(58432U);
    sut11.emplace_front(58432U);

    sut12.emplace_front(313U);
    sut12.emplace_front(3131U);

    sut11 = std::move(sut12);

    EXPECT_THAT(stats.dTor, Eq(4U));
    EXPECT_THAT(stats.moveAssignment, Eq(2U));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(sut11.size(), Eq(2U));
    EXPECT_THAT(sut11.empty(), Eq(false));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(3131U));
    EXPECT_THAT((++iter)->value, Eq(313U));
}

TEST_F(forward_list_test, MoveAssignmentWithLargerSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f415c61-384c-44be-b61d-76c61236f673");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    forward_list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842U);
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    sut12.emplace_front(1313U);
    sut12.emplace_front(13131U);

    sut12 = std::move(sut11);

    EXPECT_THAT(stats.dTor, Eq(4U));
    EXPECT_THAT(stats.moveAssignment, Eq(2U));
    EXPECT_THAT(stats.moveCTor, Eq(2U));
    EXPECT_THAT(sut12.size(), Eq(4U));
    EXPECT_THAT(sut12.empty(), Eq(false));


    auto iter = sut12.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(forward_list_test, RemoveDefaultElementFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad425b66-2e2a-4ff7-b338-ae9d86b97ca9");
    auto cnt = sut.remove({});

    EXPECT_THAT(stats.cTor, Eq(1U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}
TEST_F(forward_list_test, RemoveCustomElementFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "43a46355-3fc5-42dd-ae88-db28f1e6dcba");
    auto cnt = sut.remove({10U});

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}
TEST_F(forward_list_test, RemoveOneDefaultElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6c6e21b-f2f5-469f-a655-290c9f2faa1d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt = sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(4U));
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(4U));
    EXPECT_THAT(cnt, Eq(2U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}
TEST_F(forward_list_test, RemoveOneCustomElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "41cf180d-7cb9-4221-b850-98b806c64761");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt = sut11.remove({1584122U});

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(5U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(sut11.size(), Eq(5U));
    EXPECT_THAT(cnt, Eq(1U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}
TEST_F(forward_list_test, RemoveNotExistentElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "0550f6e2-f2c1-415f-a5f3-a9afad3d7932");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt = sut11.remove({1243U});

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(5U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(stats.classValue, Eq(1243U));
    EXPECT_THAT(sut11.size(), Eq(6U));
    EXPECT_THAT(cnt, Eq(0U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(forward_list_test, RemoveOnetoEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3f3c773-7fa8-44ff-b5f7-9bbc790f85e1");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);

    auto cnt = sut11.remove({15842U});

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(1U));
}

TEST_F(forward_list_test, RemoveWithFewMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee0e523c-fd50-43bb-8340-58294c9d7592");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(1U));
    EXPECT_THAT(cnt, Eq(2U));
}

TEST_F(forward_list_test, RemoveWithAllMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "294c0407-5475-4422-b255-816a511d8b7d");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(2U));
}

TEST_F(forward_list_test, RemoveAllFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "25af8e19-2a36-4b9d-b26e-66cdaaa0cbc2");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    constexpr uint32_t ELEMENT = 15842U;
    sut11.emplace_front(ELEMENT);
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove({ELEMENT});
    cnt += sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.dTor, Eq(5U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(3U));
}


TEST_F(forward_list_test, RemoveIfFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e140a9e-ddf7-46cf-a3ce-aebaa82165fa");
    auto cnt = sut.remove_if([](const TestListElement&) { return true; });

    EXPECT_THAT(isSetupState(), Eq(true));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}


TEST_F(forward_list_test, RemoveIfOneDefaultElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "44b32eb8-4038-43b9-9060-7c464f41b2be");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt =
        sut11.remove_if([](const TestListElement& sut1) { return sut1.value == TEST_LIST_ELEMENT_DEFAULT_VALUE; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(4U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(stats.classValue, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT(sut11.size(), Eq(4U));
    EXPECT_THAT(cnt, Eq(2U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(forward_list_test, RemoveIfOneCustomElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "7eca865c-8aad-499d-a8e4-581277cfc6e0");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt = sut11.remove_if([](const TestListElement& sut1) { return sut1.value == 1584122U; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(4U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut11.size(), Eq(5U));
    EXPECT_THAT(cnt, Eq(1U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(forward_list_test, RemoveIfNotExistentElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "90ed9470-b17a-42cb-95ad-4f926bbbdcbe");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    auto cnt = sut11.remove_if([](const TestListElement& sut1) { return sut1.value == 1234U; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(4U));
    EXPECT_THAT(stats.dTor, Eq(0U));
    EXPECT_THAT(sut11.size(), Eq(6U));
    EXPECT_THAT(cnt, Eq(0U));

    auto iter = sut11.cbefore_begin();
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(forward_list_test, RemoveIfOnetoEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "32c1c9b3-64bf-42cf-b558-46ddd042903b");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);

    auto cnt = sut11.remove_if([](const TestListElement& sut1) { return sut1.value == 15842U; });

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(1U));
}

TEST_F(forward_list_test, RemoveIfWithFewMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "8431f210-d8e4-422c-82e0-4d019de35c12");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt =
        sut11.remove_if([](const TestListElement& sut1) { return sut1.value == TEST_LIST_ELEMENT_DEFAULT_VALUE; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(sut11.size(), Eq(1U));
    EXPECT_THAT(cnt, Eq(2U));
}

TEST_F(forward_list_test, RemoveIfWithAllMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "b076c5bb-e0e7-41f6-8e08-cd42c2dcee68");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt =
        sut11.remove_if([](const TestListElement& sut1) { return sut1.value == TEST_LIST_ELEMENT_DEFAULT_VALUE; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(2U));
}

TEST_F(forward_list_test, RemoveIfAllFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "06b33436-aad4-4621-880d-a8eac1150caa");
    forward_list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove_if([](const TestListElement& sut1) { return sut1.value == 15842U; });
    cnt += sut11.remove_if([](const TestListElement& sut1) { return sut1.value == TEST_LIST_ELEMENT_DEFAULT_VALUE; });

    EXPECT_THAT(stats.cTor, Eq(2U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(3U));
}

TEST_F(forward_list_test, writeContentViaDereferencedIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "9cf93531-33ac-4cac-8234-741500755701");
    constexpr uint64_t TEST_VALUE{356U};
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto sut1{sut};
    auto iter = sut1.begin();
    TestListElement element{TEST_VALUE};
    *iter = element;
    EXPECT_THAT(sut1.front().value, Eq(TEST_VALUE));
}

TEST_F(forward_list_test, invalidIteratorErase)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d8a0810-e99c-47bd-9534-cdf74a445f60");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto iter = sut.begin();
    sut.pop_front();

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase_after(iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, invalidIteratorIncrement)
{
    ::testing::Test::RecordProperty("TEST_ID", "5dcf55cb-f2a5-4c40-8af7-2a0b736f3b9b");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    sut.pop_front();

    IOX_EXPECT_FATAL_FAILURE([&] { ++iter; }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, invalidIteratorComparison)
{
    ::testing::Test::RecordProperty("TEST_ID", "11413af1-1b36-4b0b-9a5a-9d87fe8bd8c5");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    sut.pop_front();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(sut.cbegin() == iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, invalidIteratorComparisonUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "0248c2a5-48c4-48d0-a276-359878ce1106");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }
    auto iter = sut.cbegin();
    sut.pop_front();
    auto iter2 = sut.cbegin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iter2 != iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, invalidIteratorDereferencing)
{
    ::testing::Test::RecordProperty("TEST_ID", "17fc46ba-716d-43ca-ae74-232fc7ea2ed6");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    sut.pop_front();

    IOX_EXPECT_FATAL_FAILURE([&] { sut.remove(*iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, invalidIteratorAddressOfOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "445f6f24-1e14-4dd5-8744-ef8785747026");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_front(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    sut.pop_front();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iter->value == 12U); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(forward_list_test, ListIsCopyableViaMemcpy)
{
    ::testing::Test::RecordProperty("TEST_ID", "a80ec7c9-9256-4a11-a4ec-102b22db2de2");
    uint64_t i = 0U;
    using TestFwdList = forward_list<TestListElement, TESTLISTCAPACITY>;
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) needed for test
    alignas(TestFwdList) uint8_t otherSutBuffer[sizeof(TestFwdList)];
    auto* otherSutPtr = static_cast<uint8_t*>(otherSutBuffer);

    {
        TestFwdList sut1;

        for (; i < TESTLISTCAPACITY; ++i)
        {
            const uint64_t j{i};
            sut1.emplace_front(static_cast<int64_t>(j));
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) needed for memcpy
        memcpy(reinterpret_cast<void*>(otherSutPtr), reinterpret_cast<const void*>(&sut1), sizeof(sut1));

        // overwrite copied-from list before it's being destroyed
        sut1.clear();
        for (uint64_t k = 0U; k < TESTLISTCAPACITY; ++k)
        {
            const uint64_t j{k + i};
            sut1.emplace_front(static_cast<int64_t>(j));
        }
    }

    // reinterpret_cast needed since distinct pointer types need to be compared
    // safe since otherSutBuffer is aligned to TestFwdList
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    for (auto& listElement : *reinterpret_cast<TestFwdList*>(otherSutPtr))
    {
        --i;
        EXPECT_THAT(listElement.value, Eq(i));
    }
}
