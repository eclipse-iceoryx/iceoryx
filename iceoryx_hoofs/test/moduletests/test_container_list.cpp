// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/lifetime_and_assignment_tracker.hpp"
#include "iox/attributes.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"
#include "iox/list.hpp"
#include "iox/logging.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"


namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

constexpr uint64_t TESTLISTCAPACITY{10U};
constexpr int64_t TEST_LIST_ELEMENT_DEFAULT_VALUE{-99L};

class list_test : public Test
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

    list<TestListElement, TESTLISTCAPACITY> sut;
};

template <typename IterType>
int64_t iteratorTraitReturnDoubleValue(IterType iter)
{
    typedef typename std::iterator_traits<IterType>::value_type IterValueType;
    IterValueType value = *iter;
    return (2 * value); // will only work for integer-convertible value types
}
} // namespace


TEST_F(list_test, NewlyCreatedListIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a6abb94-f5b9-4d84-b223-e77d6dc1e8a1");
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(list_test, NewlyCreatedListHasSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "233010fc-12d1-4931-b636-7ebe674015d4");
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(list_test, ReadCapacityOnList)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4c7b115-ce43-4998-a443-67541d057924");
    EXPECT_THAT(sut.capacity(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, ReadMax_sizeOnList)
{
    ::testing::Test::RecordProperty("TEST_ID", "8cbce269-aabb-400b-886d-20b135a225da");
    EXPECT_THAT(sut.max_size(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, NewListCTorWithZeroElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c47429b2-cccc-45a1-9ae0-dc005b873132");
    constexpr uint64_t CAPACITY{42U};
    EXPECT_THAT(isSetupState(), Eq(true));
    const list<int, CAPACITY> sut1;
    EXPECT_THAT(sut1.empty(), Eq(true));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, CbeginCendAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "978f6276-c970-4d52-abfe-d025d89405d4");
    EXPECT_THAT(sut.cbegin() == sut.cend(), Eq(true));
}
TEST_F(list_test, BeginEndAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "d25f80c4-7e4b-4269-b3a9-07a8cf78bc54");
    EXPECT_THAT(sut.begin() == sut.end(), Eq(true));
}
TEST_F(list_test, CbeginEndAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "35c8bd2f-7e6f-4d64-83b9-516a4ee23464");
    EXPECT_THAT(sut.cbegin() == sut.end(), Eq(true));
}
TEST_F(list_test, BeginCendAreTheSameWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "df6e8b55-d522-468b-8911-70d492c8f79c");
    EXPECT_THAT(sut.begin() == sut.cend(), Eq(true));
}

TEST_F(list_test, CbeginCendAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "b355ea2c-9662-40c1-992b-a6c1c01d94ec");
    EXPECT_THAT(sut.emplace_front().value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT(sut.cbegin() != sut.cend(), Eq(true));
}
TEST_F(list_test, BeginEndAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "05af85c8-475a-452c-b74b-4f4c3275b0d5");
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.end(), Eq(true));
}
TEST_F(list_test, CbeginEndAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "065c908e-2d8e-4355-9a3f-398a337e3771");
    sut.emplace_front();
    EXPECT_THAT(sut.cbegin() != sut.end(), Eq(true));
}
TEST_F(list_test, BeginCendAreDifferentWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "6164d6d9-8d88-430e-937e-e1ab36783ab6");
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.cend(), Eq(true));
}

TEST_F(list_test, NotEmptyWhenFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fa8377b-fc90-415b-93ae-d1ef1d409fcb");
    sut.emplace_front();
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(list_test, NotFullWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "51e99611-6509-424c-aeb3-fc2f846f3d95");
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(list_test, NotFullWhenPartialFilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "cebea572-808f-44d0-bcdb-46357dce90cf");
    sut.emplace_front();
    EXPECT_THAT(TESTLISTCAPACITY, Gt(1U));
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(list_test, FullWhenFilledWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "52b993fb-ff52-4ef9-8626-61cb4049c2e3");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        EXPECT_THAT(sut.emplace_front().value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    }
    EXPECT_THAT(sut.full(), Eq(true));
}
TEST_F(list_test, FullWhenFilledWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "585bb3d9-112c-4db8-af5e-e4c646723515");

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            for (uint64_t i = 0U; i < sut.capacity(); ++i)
            {
                sut.emplace_front();
            }

            EXPECT_THAT(sut.full(), Eq(true));
            sut.emplace_front();
        },
        iox::er::ENFORCE_VIOLATION);
}
TEST_F(list_test, NotFullWhenFilledWithCapacityAndEraseOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "b12784fd-0585-43cd-91ab-e4e82ece7113");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_front();
    }
    sut.erase(sut.cbegin());

    EXPECT_THAT(sut.size(), Eq(sut.capacity() - 1U));
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(list_test, NotFullWhenFilledWithCapacityAndEraseOneAndReinsertElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7e70ac7-e476-43aa-976c-1ac78513c869");
    int64_t counter = 0;
    for (; static_cast<uint64_t>(counter) < sut.capacity(); ++counter)
    {
        sut.emplace_back(counter);
    }
    sut.erase(sut.cbegin());
    sut.erase(sut.cbegin());
    sut.emplace_back(counter);
    sut.emplace_back(++counter);

    counter = 1;
    for (auto& element : sut)
    {
        EXPECT_THAT(element, Eq(++counter));
    }

    EXPECT_THAT(sut.size(), Eq(sut.capacity()));
    EXPECT_THAT(sut.full(), Eq(true));
}

TEST_F(list_test, CTorWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5a0593a-0db3-4e29-8a08-2c396eeecf38");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    list<TestListElement, CAPACITY> sut1;

    EXPECT_THAT(stats.cTor, Eq(0U));
    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CustomCTorWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "5550f1f9-7d9b-4a94-a11a-d32bdef98115");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    constexpr int64_t DEFAULT_VALUE{3};
    list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front(DEFAULT_VALUE);
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.classValue, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, CTorWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "932960f3-3b77-49d6-b2ed-d18e99d55150");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CTorWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "338e3aa7-b956-4cfd-bd6d-88e95504b52f");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace_front();
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CTorWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "45afddd5-043e-4622-8ce3-8d50652dc543");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<TestListElement, CAPACITY> sut1;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.push_front({});
    }
    sut1.emplace(sut1.cbegin(), 2U);

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(CAPACITY));
    EXPECT_THAT(stats.customCTor, Eq(0U));
}


TEST_F(list_test, EmplaceWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "a813857d-e403-4cde-b661-d87edd42f284");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.begin();
    decltype(TestListElement::value) cnt = 0U;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& listElement : sut1)
    {
        --cnt;
        EXPECT_THAT(listElement.value, Eq(cnt));
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "eaca1993-3043-46b2-a660-7cd901ba061a");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& listElement : sut1)
    {
        --cnt;
        EXPECT_THAT(listElement.value, Eq(cnt));
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "329155a9-f546-425a-9934-bda90f9efafc");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& listElement : sut1)
    {
        --cnt;
        EXPECT_THAT(listElement.value, Eq(cnt));
    }

    EXPECT_THAT(sut1.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6a9d813-effe-4e47-9dab-b62730fb2e41");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        iter = sut1.emplace(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY;
    for (auto& listElement : sut1)
    {
        --cnt;
        EXPECT_THAT(listElement.value, Eq(cnt));
    }

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(CAPACITY));
}


TEST_F(list_test, EmplaceReverseWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "656e2235-87ef-4a53-bee2-a385549bda1d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "98d09459-a59b-4458-b591-51f32d586d2a");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{3U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c69df3f7-b22d-4720-a3ee-ed0ad830668d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "77d6e3df-3aae-42e1-be16-6f7732696fee");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1U};
    list<TestListElement, CAPACITY> sut1;
    auto iter = sut1.cbegin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut1.emplace(iter, cnt);
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


TEST_F(list_test, EmplaceBackWithOneElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "8442de1e-61b2-457c-b99d-05fca558032d");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{1U};
    list<TestListElement, CAPACITY> sut1;
    // TestListElement compareElement{};
    decltype(TestListElement::value) cnt = 0U;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(sut1.emplace_back(cnt).value, Eq(cnt));
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

TEST_F(list_test, EmplaceBackWithSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c82db2f-bf74-44d2-b3f7-f7c5c5525bf1");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{37U};
    list<TestListElement, CAPACITY> sut1;
    decltype(TestListElement::value) cnt = 0U;

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(sut1.emplace_back(cnt).value, Eq(cnt));
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

TEST_F(list_test, EmplaceBackWithCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "96e1dd03-6a37-4914-a87e-70f6d3c87353");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<TestListElement, CAPACITY> sut1;
    decltype(TestListElement::value) cnt = 0U;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(sut1.emplace_back(cnt).value, Eq(cnt));
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

TEST_F(list_test, EmplaceBackWithMoreThanCapacityElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "66ea27ff-0b3a-41c0-86cd-eafe61442944");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1U};
    list<TestListElement, CAPACITY> sut1;
    decltype(TestListElement::value) cnt = 0U;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        if (i < CAPACITY)
        {
            EXPECT_THAT(sut1.emplace_back(cnt).value, Eq(cnt));
        }
        else
        {
            IOX_EXPECT_FATAL_FAILURE([&] { sut1.emplace_back(cnt); }, iox::er::ENFORCE_VIOLATION);
        }
        ++cnt;
    }

    cnt = 0U;
    for (auto& listElement : sut1)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(sut1.size(), Eq(CAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(CAPACITY));
}

TEST_F(list_test, EmplaceWithWrongListIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "54b1d551-30d8-4738-a46b-b4c886e9ea50");
    constexpr uint64_t CAPACITY{42U};
    constexpr uint64_t ELEMENT_COUNT{13U};
    list<TestListElement, CAPACITY> sut11;
    list<TestListElement, CAPACITY> sut12;
    auto iterOfSut1 = sut11.begin();
    auto iterOfSut2 = sut12.begin();
    int64_t cnt = 0;

    for (uint64_t i = 0U; i < ELEMENT_COUNT; ++i)
    {
        sut11.emplace(iterOfSut1, cnt);
        ++cnt;
    }

    IOX_EXPECT_FATAL_FAILURE([&] { sut11.emplace(iterOfSut2, cnt); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, PushFrontConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dbc598c-cf33-486f-a81f-0e9a0d4be132");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontConstSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "79e03f2f-dfc2-4379-b3d1-ef3e7bd3e7be");
    const TestListElement a{};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(1U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT((*sut.begin()).value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontFailsWhenSpaceNotAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "09482164-4711-485c-971f-deca2b4fb38c");
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

TEST_F(list_test, PushFrontSuccessfullWhenSpaceAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fde99ad0-180c-4c80-8e1a-2f760ffe16af");
    constexpr int64_t DEFAULT_VALUE{13L};

    EXPECT_THAT(sut.size(), Eq(0U));

    sut.push_front(DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontFailsWhenSpaceNotAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8742e83-431d-4640-a3ca-b3655abc8420");
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


TEST_F(list_test, PushBackConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "005f2924-8373-4123-8092-9d7e5c2c6567");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_back(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushBackConstSuccessfullWhenSpaceAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fdd5ec6-6378-462f-9a17-6db541c4747e");
    const TestListElement a{};
    EXPECT_TRUE(sut.push_back(a));
    ASSERT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT(stats.cTor, Eq(1U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT((*sut.begin()).value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
}

TEST_F(list_test, PushBackFailsWhenSpaceNotAvailableLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "19b85898-44d7-4348-afdb-c40ad486c100");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    EXPECT_THAT(sut.size(), Eq(0U));
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_back(a));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }
    EXPECT_FALSE(sut.push_back(a));
    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, PushBackSuccessfullWhenSpaceAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e29b7254-47d7-488e-9c06-1eb7f8db3c37");
    constexpr int64_t DEFAULT_VALUE{13L};

    EXPECT_THAT(sut.size(), Eq(0U));

    sut.push_back(DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(1U));
    EXPECT_THAT((*sut.begin()).value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushBackFailsWhenSpaceNotAvailableRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d9ffbef-0470-4ae1-ba4c-f36779452677");
    constexpr int64_t DEFAULT_VALUE{13L};
    uint64_t i = 0U;

    EXPECT_THAT(sut.size(), Eq(0U));
    for (; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_back(DEFAULT_VALUE));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    EXPECT_FALSE(sut.push_back(DEFAULT_VALUE));

    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(DEFAULT_VALUE));
    }
}

TEST_F(list_test, PushBackCheckInsertPosition)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d07986f-87ba-4dde-bfb9-7da56eb4bb54");
    int64_t counter = 0;

    for (; static_cast<uint64_t>(counter) < TESTLISTCAPACITY; ++counter)
    {
        EXPECT_TRUE(sut.push_back(counter));
    }

    counter = 0;
    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(counter));
        ++counter;
    }
}

TEST_F(list_test, AccessFrontElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "30b4c438-0371-41be-b2b4-4953bcf25cc0");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    TestListElement& b{sut.front()};
    const TestListElement& c{sut.front()};
    EXPECT_THAT(b.value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessFrontElementFromConstList)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf5837e0-e678-4f5f-a079-633829968556");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    const list<TestListElement, TESTLISTCAPACITY> sut1{sut};
    const TestListElement& c = sut1.front();

    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessBackElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "3668ef89-281c-469b-8a8c-209eee8bfe35");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front(a);
    sut.push_front({});

    TestListElement& b{sut.back()};
    const TestListElement& c{sut.back()};
    EXPECT_THAT(b.value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessBackElementFromConstList)
{
    ::testing::Test::RecordProperty("TEST_ID", "68e074fe-6c96-462d-a3a4-bac9f5f125c8");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.push_front(a);
    sut.push_front({});

    const list<TestListElement, TESTLISTCAPACITY> sut1{sut};
    const TestListElement& c = sut1.back();

    EXPECT_THAT(c.value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PopFrontOnEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "40107c7b-8ced-4f0a-822a-9955d16f5208");
    EXPECT_FALSE(sut.pop_front());
    ASSERT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, PopFrontNonEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "55659b76-479e-476a-8f23-6704cdd5e782");
    sut.emplace_front(101U);
    ASSERT_THAT(sut.size(), Eq(1U));

    EXPECT_TRUE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.dTor, Eq(1U));
}

TEST_F(list_test, PopFrontFullToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "47e7f064-d0df-4988-b380-b5578c12f3f5");
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

TEST_F(list_test, PopFrontFullPlusOneToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4fd1e58-cc68-4654-9a36-b3d81fc5b330");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace(sut.cbegin());
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


TEST_F(list_test, PopBackOnEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "75caaa69-ed87-4b53-b7b2-8f693154c32b");
    EXPECT_FALSE(sut.pop_back());
    ASSERT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, PopBackNonEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b6f470f-7dff-4146-9a75-629ab42be4e5");
    sut.emplace_front(101U);
    ASSERT_THAT(sut.size(), Eq(1U));

    EXPECT_TRUE(sut.pop_back());

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.dTor, Eq(1U));
}

TEST_F(list_test, PopBackFullToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "ead08d89-27d6-49bd-897a-5d2303a7bc85");
    // fill even more than size
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_front();
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_back());
    }

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, PopBackFullPlusOneToEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e757d59-f497-4a55-8af9-90ab2fa51f60");
    // fill even more than size
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace(sut.cbegin());
        EXPECT_THAT(sut.size(), Eq((i + 1U) > TESTLISTCAPACITY ? TESTLISTCAPACITY : (i + 1U)));
    }

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_back());
    }

    EXPECT_FALSE(sut.pop_back());

    ASSERT_THAT(sut.size(), Eq(0U));
    ASSERT_THAT(stats.cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.customCTor, Eq(0U));
    ASSERT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
}


TEST_F(list_test, InsertEmptyListAsLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "838a25db-5903-4ba9-a965-561eec4f6625");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.insert(sut.cbegin(), a);

    ASSERT_THAT(sut.size(), Eq(1U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.copyCTor, Eq(1U));
    ASSERT_THAT(stats.moveCTor, Eq(0U));
    ASSERT_THAT(stats.copyAssignment, Eq(0U));
    ASSERT_THAT(stats.moveAssignment, Eq(0U));
}

TEST_F(list_test, InsertLValueCheckReturn)
{
    ::testing::Test::RecordProperty("TEST_ID", "866c003d-8632-49c7-b4cf-7c4475c4aa4e");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    auto iter = sut.insert(sut.begin(), a);

    ASSERT_THAT(iter == sut.begin(), Eq(true));
}

TEST_F(list_test, InsertEmptyListAsRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "be68359a-c37a-4fd4-9268-f7461417a6d6");
    sut.insert(sut.begin(), {10U});

    ASSERT_THAT(sut.size(), Eq(1U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    ASSERT_THAT(stats.copyCTor, Eq(0U));
    ASSERT_THAT(stats.moveCTor, Eq(1U));
    ASSERT_THAT(stats.copyAssignment, Eq(0U));
    ASSERT_THAT(stats.moveAssignment, Eq(0U));
}

TEST_F(list_test, InsertRValueCheckReturn)
{
    ::testing::Test::RecordProperty("TEST_ID", "cff90ab6-7e09-45a1-b2ce-a74876f71cf8");
    auto iter = sut.insert(sut.begin(), {10U});

    ASSERT_THAT(iter == sut.begin(), Eq(true));
    ASSERT_THAT((*iter).value, Eq(10U));
}

TEST_F(list_test, InsertBeginListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5a6a002-95e7-4188-8826-15f62c7abaec");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.insert(sut.begin(), a);
    sut.emplace_front();

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
}


TEST_F(list_test, InsertBeforeBeginListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ae7dc88-c9be-49ad-9f82-1a2ea5aaf1b7");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert(sut.begin(), a);

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
}

TEST_F(list_test, InsertBeforeBeginListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "906c22db-18b9-454f-8ac1-8e7044c07e31");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};

    sut.emplace_front(a);
    sut.insert(sut.begin(), {});

    ASSERT_THAT(sut.size(), Eq(2U));
    ASSERT_THAT(stats.cTor, Eq(1U));
    ASSERT_THAT(stats.customCTor, Eq(1U));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, InsertSomeElementsListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "86ae5bac-dec6-489b-b5a8-822276ae209d");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    uint64_t loopCounter = 0U;

    // this tests the test case setup (the following code needs a minimum testlist capacity)
    ASSERT_THAT(TESTLISTCAPACITY, Ge(10U));

    // fill half
    for (uint64_t i = 0U; i < 5U; ++i)
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
    sut.insert(iter, a);

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
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}

TEST_F(list_test, InsertSomeElementsListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5aac735-c551-4767-ae45-512e3989bc5f");
    constexpr int64_t DEFAULT_VALUE{13L};

    // test requires a minimum capacity
    ASSERT_THAT(TESTLISTCAPACITY, Ge(10U));

    // fill half
    for (uint64_t i = 0U; i < 5U; ++i)
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
    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(6U));

    iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(4U));
    EXPECT_THAT((++iter)->value, Eq(3U));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}


TEST_F(list_test, InsertFullElementsListLValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "51f6af85-ca8c-4002-be94-76beda6f3dae");
    constexpr int64_t DEFAULT_VALUE{13L};
    const TestListElement a{DEFAULT_VALUE};
    int64_t cnt = 0U;

    auto iter = sut.begin();

    // fill full-1
    for (uint64_t i = 0U; i < TESTLISTCAPACITY - 1; ++i)
    {
        sut.emplace(iter, static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    sut.insert(iter, a);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1L == cnt)
        {
            // for the last element (insert) check for different value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(list_test, InsertFullElementsListRValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbbda6b6-3a44-49b8-b5ca-da16e48e1e83");
    constexpr int64_t DEFAULT_VALUE{13L};
    int64_t cnt = 0U;

    auto iter = sut.begin();

    // fill full-1
    for (uint64_t i = 0U; i < TESTLISTCAPACITY - 1; ++i)
    {
        sut.emplace(iter, static_cast<int64_t>(i));
        EXPECT_THAT(sut.size(), Eq(i + 1U));
    }

    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));

    for (auto& listElement : sut)
    {
        EXPECT_THAT(listElement.value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1L == cnt)
        {
            // for the last element (insert) check for different value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(list_test, IteratorArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "20e8ea50-b3f4-48ce-8b67-b140bc516353");
    constexpr int64_t DEFAULT_VALUE{13L};

    ASSERT_THAT(TESTLISTCAPACITY, Ge(10U)); // for the rest of magic numbers to fit

    // fill half
    for (uint64_t i = 0U; i < 5U; ++i)
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
    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6U));
    ASSERT_THAT(stats.cTor, Eq(0U));
    ASSERT_THAT(stats.customCTor, Eq(6U));

    iter = sut.cbegin();
    EXPECT_THAT(iter->value, Eq(4U));
    EXPECT_THAT((++iter)->value, Eq(3U));
    EXPECT_THAT((++iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(2U));
    EXPECT_THAT((++iter)->value, Eq(1U));
    EXPECT_THAT((++iter)->value, Eq(0U));
}

TEST_F(list_test, IteratorIncrementOperatorBeyondEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "74533e95-6f71-488e-a283-280e759a2e5e");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.begin();
    EXPECT_THAT(iter->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}

TEST_F(list_test, ConstIteratorIncrementOperatorBeyondEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e06d279-ba18-4c4e-a188-1bbe47709ed6");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.cbegin();
    EXPECT_THAT(iter->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}

TEST_F(list_test, IteratorDecrementOperatorBeyondBegin)
{
    ::testing::Test::RecordProperty("TEST_ID", "cc569b79-a575-47f3-86ff-276e2b482d43");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.end();
    EXPECT_THAT((--iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((--iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}

TEST_F(list_test, ConstIteratorDecrementOperatorBeyondBegin)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf6ab466-5724-40de-b684-7e2d9cd801b6");
    constexpr int64_t DEFAULT_VALUE{13L};

    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.cend();
    EXPECT_THAT((--iter)->value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((--iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}

TEST_F(list_test, IteratorDecrementOperatorBeyondBeginWithFullList)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e437ebc-9664-4cd8-ab60-843d9089efc4");
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_front(static_cast<int64_t>(i));
    }

    auto iter = sut.end();
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        EXPECT_THAT((--iter)->value, Eq(i));
    }
    EXPECT_TRUE((iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}


TEST_F(list_test, IteratorComparisonOfDifferentLists)
{
    ::testing::Test::RecordProperty("TEST_ID", "0be4cfdd-1abb-4a34-93b1-490b0cf07b9c");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842);
    sut11.emplace_front(1584122);
    sut11.emplace_front(158432);
    sut11.emplace_front(158432);

    sut12.emplace_front(1313);
    sut12.emplace_front(13131);


    auto iterSut1 = sut11.begin();
    auto iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.begin();
    iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.end();
    iterSut2 = sut12.end();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 == iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.begin();
    iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.begin();
    iterSut2 = sut12.begin();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);

    iterSut1 = sut11.end();
    iterSut2 = sut12.end();

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iterSut1 != iterSut2); }, iox::er::ENFORCE_VIOLATION);
}


TEST_F(list_test, ComparingConstIteratorAndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ab56e3f-c22b-4544-806e-04dfda4ce449");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(15842U);
    sut11.emplace_front(1584122U);
    sut11.emplace_front(158432U);
    sut11.emplace_front(158432U);

    sut12.emplace_front(1313U);
    sut12.emplace_front(13131U);


    list<TestListElement, TESTLISTCAPACITY>::const_iterator iterSut1 = sut11.cbegin();
    list<TestListElement, TESTLISTCAPACITY>::const_iterator iterSut2 = sut11.cend();
    list<TestListElement, TESTLISTCAPACITY>::iterator iterSut3 = sut11.begin();
    list<TestListElement, TESTLISTCAPACITY>::iterator iterSut4 = sut11.end();

    ASSERT_THAT(iterSut1 == iterSut3, Eq(true));
    ASSERT_THAT(iterSut3 == iterSut1, Eq(true));

    ASSERT_THAT(iterSut1 == iterSut2, Eq(false));
    ASSERT_THAT(iterSut4 == iterSut3, Eq(false));
}


TEST_F(list_test, IteratorTraitsGetValueType)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e0f5e29-ab58-4436-9f5f-50e73ba52cbf");
    list<int, 10U> sut1;

    sut1.emplace_front(5);
    auto iter{sut1.begin()};

    // using a function call here is closer to the actual use case (-> intentionally did not inline all code here)
    auto ret = iteratorTraitReturnDoubleValue(iter);

    EXPECT_THAT(ret, Eq(10U));
}

TEST_F(list_test, IteratorTraitsCheckIteratorCategoryOnConstIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "295e6406-93bc-4a12-9b03-33e5d494b2c2");
    auto iter [[maybe_unused]] = sut.cbegin();
    ASSERT_NE(typeid(std::iterator_traits<decltype(iter)>::iterator_category), typeid(std::random_access_iterator_tag));
    EXPECT_EQ(typeid(std::iterator_traits<decltype(iter)>::iterator_category), typeid(std::bidirectional_iterator_tag));
}

TEST_F(list_test, EmptyAfterClear)
{
    ::testing::Test::RecordProperty("TEST_ID", "5dce6c01-426a-42a9-bfeb-41916db6d744");
    sut.emplace_front(5U);
    sut.clear();
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(list_test, SizeZeroAfterClear)
{
    ::testing::Test::RecordProperty("TEST_ID", "518c4bab-3e3d-4665-9bc2-bac68568f6fa");
    sut.emplace_front(5U);
    sut.clear();
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(list_test, CopyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e65e42c-b99c-413e-94cd-a5297ed7b6ae");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(101U);
    sut11.emplace_front(102);
    EXPECT_THAT(stats.customCTor, Eq(2U));

    list<TestListElement, TESTLISTCAPACITY> sut12(sut11);

    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(2U));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    auto iter = sut12.begin();
    EXPECT_THAT(iter->value, Eq(102));
    EXPECT_THAT((++iter)->value, Eq(101U));
    EXPECT_THAT(sut12.empty(), Eq(false));
    EXPECT_THAT(sut12.size(), Eq(2U));
}

TEST_F(list_test, CopyConstructorWithEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "65409af0-bd13-4011-bdd7-e1b6aa0d4703");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    // NOLINTJUSTIFICATION the test should explicitly test the copy constructor
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    list<TestListElement, TESTLISTCAPACITY> sut12(sut11);
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(sut12.size(), Eq(0U));
    EXPECT_THAT(sut12.empty(), Eq(true));
}

TEST_F(list_test, CopyConstructorWithFullList)
{
    ::testing::Test::RecordProperty("TEST_ID", "674ac8f0-3bb5-47e4-afd9-30f0bcaf6b55");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    decltype(TestListElement::value) i = 0U;

    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut11.emplace_front(static_cast<int64_t>(i));
    }

    list<TestListElement, TESTLISTCAPACITY> sut12(sut11);
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

TEST_F(list_test, MoveConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "2fb0328e-c971-41b9-91c6-b552ea941949");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    sut11.emplace_front(8101U);
    sut11.emplace_front(8102);

    list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.moveCTor, Eq(2U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    EXPECT_THAT(stats.moveAssignment, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    auto iter = sut12.begin();
    EXPECT_THAT(iter->value, Eq(8102));
    EXPECT_THAT((++iter)->value, Eq(8101U));
    EXPECT_THAT(sut12.empty(), Eq(false));
    EXPECT_THAT(sut12.size(), Eq(2U));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved object
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_THAT(sut11.empty(), Eq(true));
}

TEST_F(list_test, MoveConstructorWithEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "42991766-c8dd-4261-814f-837ae99f6647");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));
    EXPECT_THAT(stats.moveCTor, Eq(0U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(sut12.size(), Eq(0U));
    EXPECT_THAT(sut12.empty(), Eq(true));
}

TEST_F(list_test, MoveConstructorWithFullList)
{
    ::testing::Test::RecordProperty("TEST_ID", "af338f29-14d2-4d7a-8f2b-75ce43259bcb");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        sut11.emplace_front(static_cast<int64_t>(i));
    }

    list<TestListElement, TESTLISTCAPACITY> sut12(std::move(sut11));

    EXPECT_THAT(stats.moveCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(stats.copyCTor, Eq(0U));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(sut12.empty(), Eq(false));
}

TEST_F(list_test, DestructorWithEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "6103decd-e7c8-41bb-87c9-0e3d98782df2");
    {
        list<TestListElement, TESTLISTCAPACITY> sut11;
    }
    EXPECT_THAT(stats.dTor, Eq(0U));
}

TEST_F(list_test, DestructorSomeElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "52694b52-1a4c-41fe-8624-52d419134dfd");
    {
        list<TestListElement, TESTLISTCAPACITY> sut11;
        sut11.emplace_front(891U);
        sut11.emplace_front(9191U);
        sut11.emplace_front(1U);
    }
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(3U));
    EXPECT_THAT(stats.dTor, Eq(3U));
}

TEST_F(list_test, DestructorWithFullList)
{
    ::testing::Test::RecordProperty("TEST_ID", "fcd37446-e682-4248-9d94-ca9dfaba7d0e");
    {
        list<TestListElement, TESTLISTCAPACITY> sut11;
        for (uint64_t i = 0U; i < sut11.capacity(); ++i)
        {
            sut11.emplace_front(1231U);
        }
    }

    EXPECT_THAT(stats.dTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, CopyAssignmentWithEmptySource)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4f0c079-d7bb-4c46-8221-dd0a616537bb");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
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

TEST_F(list_test, CopyAssignmentWithEmptyDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "9202d629-f0db-482e-9363-456a83410d54");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
    sut11.emplace_front(5812U);
    sut11.emplace_front(581122U);
    sut11.emplace_front(58132U);

    sut12 = sut11;
    EXPECT_THAT(stats.dTor, Eq(0U));
    EXPECT_THAT(stats.copyAssignment, Eq(0U));
    EXPECT_THAT(stats.copyCTor, Eq(3U));
    EXPECT_THAT(sut12.size(), Eq(3U));
    EXPECT_THAT(sut12.empty(), Eq(false));

    auto iter = sut12.cbegin();
    EXPECT_THAT(iter->value, Eq(58132U));
    EXPECT_THAT((++iter)->value, Eq(581122U));
    EXPECT_THAT((++iter)->value, Eq(5812U));
}


TEST_F(list_test, CopyAssignmentWithLargerDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "f16524d0-1493-4f57-8590-211a96fb9849");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT(iter->value, Eq(3131U));
    EXPECT_THAT((++iter)->value, Eq(313U));
}

TEST_F(list_test, CopyAssignmentWithLargerSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "876a0d55-1047-4ec7-8b97-7a90b928fea1");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
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

    auto iter = sut12.cbegin();
    EXPECT_THAT(iter->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}


TEST_F(list_test, MoveAssignmentWithEmptySource)
{
    ::testing::Test::RecordProperty("TEST_ID", "135d1210-1434-40f8-b169-ef742622ec41");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
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

TEST_F(list_test, MoveAssignmentWithEmptyDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "457b17f5-2bea-429c-b836-d30b4006c8be");
    list<TestListElement, TESTLISTCAPACITY> sut11;
    list<TestListElement, TESTLISTCAPACITY> sut12;
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

    auto iter = sut12.cbegin();
    EXPECT_THAT((iter)->value, Eq(58132U));
    EXPECT_THAT((++iter)->value, Eq(581122U));
    EXPECT_THAT((++iter)->value, Eq(5812U));
}


TEST_F(list_test, MoveAssignmentWithLargerDestination)
{
    ::testing::Test::RecordProperty("TEST_ID", "66e0e3b9-be1d-4e00-a56a-41902255b43c");
    list<TestListElement, 10U> sut11;
    list<TestListElement, 10U> sut12;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(3131U));
    EXPECT_THAT((++iter)->value, Eq(313U));
}

TEST_F(list_test, MoveAssignmentWithLargerSource)
{
    ::testing::Test::RecordProperty("TEST_ID", "c190218d-f3dc-4f44-b4b4-2e8f5b161a65");
    list<TestListElement, 10U> sut11;
    list<TestListElement, 10U> sut12;
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


    auto iter = sut12.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(list_test, RemoveDefaultElementFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "c77e3f62-1c49-4672-9772-79a1fd818b80");
    auto cnt = sut.remove({});

    EXPECT_THAT(stats.cTor, Eq(1U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}
TEST_F(list_test, RemoveCustomElementFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5c57a6d-e7d7-4219-a4a9-fa59f0b9d08f");
    auto cnt = sut.remove({10U});

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}
TEST_F(list_test, RemoveOneDefaultElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "55bd88da-8f67-4887-a44b-1019691b243f");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}
TEST_F(list_test, RemoveOneCustomElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b9886b8-34d2-4c9b-b34f-035744f4300a");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}
TEST_F(list_test, RemoveNotExistentElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cd93353-bf5a-4e52-a570-06bbf710cdd1");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(list_test, RemoveOnetoEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "5037ae8b-3acb-4176-a2b0-075090570393");
    list<TestListElement, 10U> sut11;
    sut11.emplace_front(15842U);

    auto cnt = sut11.remove({15842U});

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.dTor, Eq(2U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(1U));
}

TEST_F(list_test, RemoveWithFewMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc475185-c34a-4b2f-bd79-63c8e5eeb36b");
    list<TestListElement, 10U> sut11;
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

TEST_F(list_test, RemoveWithAllMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c04ace9-7bf9-4153-b53b-be3d6cadb513");
    list<TestListElement, 10U> sut11;
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(0U));
    EXPECT_THAT(stats.dTor, Eq(3U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(2U));
}

TEST_F(list_test, RemoveAllFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc2d5aa7-b754-461b-b9d2-7563e49caed3");
    list<TestListElement, 10U> sut11;
    sut11.emplace_front(15842U);
    sut11.emplace_front();
    sut11.emplace_front();

    auto cnt = sut11.remove({15842U});
    cnt += sut11.remove({});

    EXPECT_THAT(stats.cTor, Eq(3U));
    EXPECT_THAT(stats.customCTor, Eq(2U));
    EXPECT_THAT(stats.dTor, Eq(5U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(3U));
}


TEST_F(list_test, RemoveIfFromEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "926361f6-cab5-441c-abbb-890e383797ab");
    auto cnt = sut.remove_if([](const TestListElement&) { return true; });

    EXPECT_THAT(isSetupState(), Eq(true));
    EXPECT_THAT(sut.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(0U));
}


TEST_F(list_test, RemoveIfOneDefaultElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "76741358-0f13-4d7f-88ed-cf7e834b32f6");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(list_test, RemoveIfOneCustomElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "af431555-7e93-4518-bdf8-2a775bc0a43d");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(list_test, RemoveIfNotExistentElementFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "89313c6c-e6b7-49aa-a9ad-ac2934356ee5");
    list<TestListElement, 10U> sut11;
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

    auto iter = sut11.cbegin();
    EXPECT_THAT((iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(158432U));
    EXPECT_THAT((++iter)->value, Eq(1584122U));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(TEST_LIST_ELEMENT_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->value, Eq(15842U));
}

TEST_F(list_test, RemoveIfOnetoEmptyList)
{
    ::testing::Test::RecordProperty("TEST_ID", "d88620c3-9aa9-40a9-9666-4b37b93263af");
    list<TestListElement, 10U> sut11;
    sut11.emplace_front(15842U);

    auto cnt = sut11.remove_if([](const TestListElement& sut1) { return sut1.value == 15842U; });

    EXPECT_THAT(stats.cTor, Eq(0U));
    EXPECT_THAT(stats.customCTor, Eq(1U));
    EXPECT_THAT(stats.dTor, Eq(1U));
    EXPECT_THAT(sut11.size(), Eq(0U));
    EXPECT_THAT(cnt, Eq(1U));
}

TEST_F(list_test, RemoveIfWithFewMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ce863b0-a80e-41f7-a1a8-4f44511260d5");
    list<TestListElement, 10U> sut11;
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

TEST_F(list_test, RemoveIfWithAllMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "c27c1020-fc7d-4b4d-9331-486eb616cf84");
    list<TestListElement, 10U> sut11;
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

TEST_F(list_test, RemoveIfAllFromList)
{
    ::testing::Test::RecordProperty("TEST_ID", "070ccd82-a32f-4653-8152-9103a1f9a4e6");
    list<TestListElement, 10U> sut11;
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

TEST_F(list_test, writeContentViaDereferencedIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3f6af26-8ced-4a38-9170-dede9b5722ff");
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

TEST_F(list_test, invalidIteratorErase)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fa4664f-19b1-4178-a4ea-79afc1d3b3e9");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);


    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorIncrement)
{
    ::testing::Test::RecordProperty("TEST_ID", "c001c7b4-ccdf-4c28-88d6-b70369881001");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);

    IOX_EXPECT_FATAL_FAILURE([&] { ++iter; }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorDecrement)
{
    ::testing::Test::RecordProperty("TEST_ID", "df823946-c753-436f-b9d3-0710ac72746f");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);

    IOX_EXPECT_FATAL_FAILURE([&] { --iter; }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorComparison)
{
    ::testing::Test::RecordProperty("TEST_ID", "bfd0f516-2c81-4179-ba38-1ff01acef1cb");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 [[maybe_unused]] = sut.erase(iter);


    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(sut.cbegin() == iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorComparisonUnequal)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb159e7a-d571-4e33-84a8-6c1be7fb79c3");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 = sut.erase(iter);


    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iter2 != iter); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorDereferencing)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d0f92cb-12f2-4747-bbce-552c45a11a9a");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 [[maybe_unused]] = sut.erase(iter);

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT((*iter).value); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, invalidIteratorAddressOfOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "15cb8798-603d-41d5-a222-1c9825287bb3");
    for (uint64_t i = 0U; i < TESTLISTCAPACITY; ++i)
    {
        const uint64_t j{i};
        sut.emplace_back(static_cast<int64_t>(j));
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 [[maybe_unused]] = sut.erase(iter);

    IOX_EXPECT_FATAL_FAILURE([&] { IOX_DISCARD_RESULT(iter->value == 12U); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(list_test, ListIsCopyableViaMemcpy)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce16f50e-8da5-497c-abd5-a3af4ebd78d2");
    uint64_t i = 0U;
    using TestFwdList = list<TestListElement, TESTLISTCAPACITY>;
    // NOLINTJUSTIFICATION required temporary memory buffer for the memcpy test
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    alignas(TestFwdList) uint8_t otherSutBuffer[sizeof(TestFwdList)];
    uint8_t* otherSutPtr = &otherSutBuffer[0];

    {
        TestFwdList sut1;

        for (; i < TESTLISTCAPACITY; ++i)
        {
            const uint64_t j{i};
            sut1.emplace_front(static_cast<int64_t>(j));
        }

        // NOLINTJUSTIFICATION the list is trivially copyable and this test verifies this with memcpy
        // NOLINTNEXTLINE(bugprone-undefined-memory-manipulation)
        memcpy(otherSutPtr, &sut1, sizeof(sut1));

        // overwrite copied-from list before it's being destroyed
        sut1.clear();
        for (uint64_t k = 0U; k < TESTLISTCAPACITY; ++k)
        {
            const uint64_t j{k + i};
            sut1.emplace_front(static_cast<int64_t>(j));
        }
    }

    // NOLINTJUSTIFICATION we need to verify that list is in fact trivially copyable and stored in otherSubBuffer
    // therefore we need to cast it
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    for (auto& listElement : *reinterpret_cast<TestFwdList*>(otherSutPtr))
    {
        --i;
        EXPECT_THAT(listElement.value, Eq(i));
    }
}
