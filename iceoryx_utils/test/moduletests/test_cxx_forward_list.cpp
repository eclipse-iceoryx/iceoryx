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
#include <iterator>

#include "iceoryx_utils/cxx/forward_list.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::cxx;

static constexpr uint64_t TESTLISTCAPACITY{10u};
static constexpr int C_TOR_TEST_VALUE_DEFAULT_VALUE{-99};

class forward_list_test : public Test
{
  public:
    static int cTor;
    static int customCTor;
    static int copyCTor;
    static int moveCTor;
    static int moveAssignment;
    static int copyAssignment;
    static int dTor;
    static int classValue;

    class CTorTest
    {
      public:
        CTorTest()
        {
            cTor++;
            classValue = m_value;
        }

        CTorTest(const int value)
            : m_value(value)
        {
            customCTor++;
            classValue = m_value;
        }

        CTorTest(const CTorTest& rhs)
        {
            copyCTor++;
            m_value = rhs.m_value;
            classValue = m_value;
        }

        CTorTest(CTorTest&& rhs)
        {
            moveCTor++;
            m_value = rhs.m_value;
            classValue = m_value;
        }

        CTorTest& operator=(const CTorTest& rhs)
        {
            copyAssignment++;
            m_value = rhs.m_value;
            classValue = m_value;
            return *this;
        }

        CTorTest& operator=(CTorTest&& rhs)
        {
            moveAssignment++;
            m_value = rhs.m_value;
            classValue = m_value;
            return *this;
        }

        bool operator==(const CTorTest& rhs)
        {
            return rhs.m_value == m_value;
        }

        ~CTorTest()
        {
            dTor++;
            classValue = m_value;
        }

        int m_value = C_TOR_TEST_VALUE_DEFAULT_VALUE;
    };


    void SetUp()
    {
        cTor = 0;
        customCTor = 0;
        copyCTor = 0;
        moveCTor = 0;
        moveAssignment = 0;
        copyAssignment = 0;
        dTor = 0;
        classValue = 0;
    }

    bool isSetupState()
    {
        if (cTor == 0 && customCTor == 0 && copyCTor == 0 && moveCTor == 0 && moveAssignment == 0 && copyAssignment == 0
            && dTor == 0 && classValue == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    forward_list<CTorTest, TESTLISTCAPACITY> sut;
};

// forward_list_test statics
int forward_list_test::cTor;
int forward_list_test::customCTor;
int forward_list_test::copyCTor;
int forward_list_test::moveCTor;
int forward_list_test::moveAssignment;
int forward_list_test::copyAssignment;
int forward_list_test::dTor;
int forward_list_test::classValue;


// test function for iterator_traits testing
template <typename IterType>
int iteratorTraitReturnDoubleValue(IterType iter)
{
    typedef typename std::iterator_traits<IterType>::value_type IterValueType;
    IterValueType m_value = *iter;
    return (2 * m_value); // will only work for integer-convertible m_value types
}

// in context of EXPECT_DEATH tests, dummyFunc() shall help suppressing following warning :
// -Wunused-comparison
// reason: the warning is already addressed with the internal handling, which shall be tested here
bool dummyFunc(bool whatever)
{
    std::cerr << "Never get here - ever " << whatever << std::endl;
    return whatever;
}


TEST_F(forward_list_test, NewlyCreatedListIsEmpty)
{
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(forward_list_test, NewlyCreatedListHasSizeZero)
{
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(forward_list_test, ReadCapacityOnList)
{
    EXPECT_THAT(sut.capacity(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, ReadMax_sizeOnList)
{
    EXPECT_THAT(sut.max_size(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, NewListCTorWithZeroElements)
{
    constexpr uint64_t CAPACITY{42u};
    EXPECT_THAT(isSetupState(), Eq(true));
    const forward_list<int, CAPACITY> cut;
    EXPECT_THAT(cut.empty(), Eq(true));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(forward_list_test, CbeginCendAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.cbegin() == sut.cend(), Eq(true));
}
TEST_F(forward_list_test, BeginEndAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.begin() == sut.end(), Eq(true));
}
TEST_F(forward_list_test, CbeginEndAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.cbegin() == sut.end(), Eq(true));
}
TEST_F(forward_list_test, BeginCendAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.begin() == sut.cend(), Eq(true));
}

TEST_F(forward_list_test, CbeforebeginAndCbeginAreDifferentWhenEmpty)
{
    EXPECT_THAT(sut.cbefore_begin() != sut.cbegin(), Eq(true));
}
TEST_F(forward_list_test, beforebeginAndBeginAreDifferentWhenEmpty)
{
    EXPECT_THAT(sut.before_begin() != sut.begin(), Eq(true));
}
TEST_F(forward_list_test, CbeforeBeginAndBeginAreDifferentWhenEmpty)
{
    EXPECT_THAT(sut.cbefore_begin() != sut.begin(), Eq(true));
}
TEST_F(forward_list_test, beforeBeginAndCbeginAreDifferentWhenEmpty)
{
    EXPECT_THAT(sut.before_begin() != sut.cbegin(), Eq(true));
}

TEST_F(forward_list_test, CbeginCendAreDifferentWhenFilled)
{
    EXPECT_TRUE(sut.emplace_front());
    EXPECT_THAT(sut.cbegin() != sut.cend(), Eq(true));
}
TEST_F(forward_list_test, BeginEndAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.end(), Eq(true));
}
TEST_F(forward_list_test, CbeginEndAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.cbegin() != sut.end(), Eq(true));
}
TEST_F(forward_list_test, BeginCendAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.cend(), Eq(true));
}

TEST_F(forward_list_test, NotEmptyWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(forward_list_test, NotFullWhenEmpty)
{
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(forward_list_test, NotFullWhenPartialFilled)
{
    sut.emplace_front();
    EXPECT_THAT(TESTLISTCAPACITY, Gt(1));
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(forward_list_test, FullWhenFilledWithCapacityElements)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        EXPECT_TRUE(sut.emplace_front());
    }
    EXPECT_THAT(sut.full(), Eq(true));
}
TEST_F(forward_list_test, FullWhenFilledWithMoreThanCapacityElements)
{
    for (uint64_t i = 0; i < sut.capacity() + 1; ++i)
    {
        sut.emplace_front();
    }
    EXPECT_FALSE(sut.emplace_front());

    EXPECT_THAT(sut.full(), Eq(true));
}
TEST_F(forward_list_test, NotFullWhenFilledWithMoreThanCapacityAnEraseOneElements)
{
    for (uint64_t i = 0; i < sut.capacity() + 1; ++i)
    {
        sut.emplace_front();
    }
    sut.erase_after(sut.cbefore_begin());

    EXPECT_THAT(sut.size(), Eq(sut.capacity() - 1));
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(forward_list_test, CTorWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    forward_list<CTorTest, CAPACITY> cut;

    EXPECT_THAT(cTor, Eq(0));
    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CustomCTorWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    constexpr uint64_t DEFAULT_VALUE{3};
    forward_list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front(DEFAULT_VALUE);
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
    EXPECT_THAT(classValue, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, CTorWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    forward_list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CTorWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, CTorWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    forward_list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(CAPACITY));
    EXPECT_THAT(cTor, Eq(CAPACITY));
}


TEST_F(forward_list_test, EmplaceAfterWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    decltype(CTorTest::m_value) cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = 0;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
    }

    EXPECT_THAT(cut.size(), Eq(CAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(CAPACITY));
}


TEST_F(forward_list_test, EmplaceAfterReverseWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_after(iter, cnt);
        ++cnt;
    }

    --cnt;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_after(iter, cnt);
        ++cnt;
    }

    --cnt;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY - 1;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(forward_list_test, EmplaceAfterReverseWithWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    forward_list<CTorTest, CAPACITY> cut;
    auto iter = cut.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_after(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY - 1;
    for (auto& fl : cut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        --cnt;
    }

    EXPECT_THAT(cut.size(), Eq(CAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(CAPACITY));
}

TEST_F(forward_list_test, EmplaceAfterWithWrongListIterator)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{13};
    forward_list<CTorTest, CAPACITY> cut1, cut2;
    auto iterOfCut1 = cut1.before_begin();
    auto iterOfCut2 = cut2.before_begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut1.emplace_after(iterOfCut1, cnt);
        ++cnt;
    }

    EXPECT_DEATH(cut1.emplace_after(iterOfCut2, cnt), "");
}

TEST_F(forward_list_test, PushFrontConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontConstSuccessfullWhenSpaceAvailableLValue)
{
    const CTorTest a{};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(1));
    EXPECT_THAT(customCTor, Eq(0u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontFailsWhenSpaceNotAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    EXPECT_THAT(sut.size(), Eq(0));
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_front(a));
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }
    EXPECT_FALSE(sut.push_front(a));
    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, PushFrontSuccessfullWhenSpaceAvailableRValue)
{
    constexpr int DEFAULT_VALUE{13};

    EXPECT_THAT(sut.size(), Eq(0));

    sut.push_front(DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(1));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, PushFrontFailsWhenSpaceNotAvailableRValue)
{
    constexpr int DEFAULT_VALUE{13};

    EXPECT_THAT(sut.size(), Eq(0));
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_front(DEFAULT_VALUE));
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    EXPECT_FALSE(sut.push_front(DEFAULT_VALUE));

    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(DEFAULT_VALUE));
    }
}


TEST_F(forward_list_test, AccessFrontElement)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front(a);

    CTorTest& b{sut.front()};
    const CTorTest& c{sut.front()};
    EXPECT_THAT(b.m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, AccessFrontElementFromConstList)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front(a);

    const forward_list<CTorTest, TESTLISTCAPACITY> cut{sut};
    const CTorTest& c = cut.front();

    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}


TEST_F(forward_list_test, PopFrontOnEmptyList)
{
    EXPECT_FALSE(sut.pop_front());
    ASSERT_THAT(sut.size(), Eq(0u));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(forward_list_test, PopFrontNonEmptyList)
{
    sut.emplace_front(101);
    ASSERT_THAT(sut.size(), Eq(1u));

    EXPECT_TRUE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(0u));
    EXPECT_THAT(customCTor, Eq(1u));
    ASSERT_THAT(dTor, Eq(1u));
}

TEST_F(forward_list_test, PopFrontFullToEmptyList)
{
    // fill even more than size
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_front();
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_front());
    }

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(dTor, Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, PopFrontFullPlusOneToEmptyList)
{
    // fill even more than size
    for (uint64_t i = 0; i < TESTLISTCAPACITY + 1; ++i)
    {
        sut.emplace_front();
        EXPECT_THAT(sut.size(), Eq((i + 1) > TESTLISTCAPACITY ? TESTLISTCAPACITY : (i + 1)));
    }

    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_front());
    }

    EXPECT_FALSE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(customCTor, Eq(0u));
    ASSERT_THAT(dTor, Eq(TESTLISTCAPACITY));
}


TEST_F(forward_list_test, InsertAfterEmptyListAsLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(sut.size(), Eq(1u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(1u));
}

TEST_F(forward_list_test, InsertAfterLValueCheckReturn)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    auto iter = sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(iter == sut.begin(), Eq(true));
}

TEST_F(forward_list_test, InsertAfterEmptyListAsRValue)
{
    sut.insert_after(sut.before_begin(), {10});

    ASSERT_THAT(sut.size(), Eq(1u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(1u));
}

TEST_F(forward_list_test, InsertAfterRValueCheckReturn)
{
    auto iter = sut.insert_after(sut.before_begin(), {10});

    ASSERT_THAT(iter == sut.begin(), Eq(true));
    ASSERT_THAT((*iter).m_value, Eq(10));
}

TEST_F(forward_list_test, InsertAfterBeginListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert_after(sut.begin(), a);

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
}


TEST_F(forward_list_test, InsertAfterBeforeBeginListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert_after(sut.before_begin(), a);

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
}

TEST_F(forward_list_test, InsertAfterBeforeBeginListRValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.emplace_front(a);
    sut.insert_after(sut.before_begin(), {});

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
}

TEST_F(forward_list_test, InsertAfterSomeElementsListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    // this tests the test case setup (the following code needs a minimum testlist capacity)
    ASSERT_THAT(TESTLISTCAPACITY, Ge(10u));

    // fill half
    for (uint64_t i = 0; i < 5; ++i)
    {
        sut.emplace_front(i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    auto iter = sut.begin();
    // 2 increments
    for (uint64_t i = 0; i < 2; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, a);

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}

TEST_F(forward_list_test, InsertAfterSomeElementsListRValue)
{
    constexpr int DEFAULT_VALUE{13};

    ASSERT_THAT(TESTLISTCAPACITY, Ge(10u)); // for the rest of magic numbers to fit

    // fill half
    for (uint64_t i = 0; i < 5; ++i)
    {
        sut.emplace_front(i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    auto iter = sut.begin();
    // 2 increments
    for (uint64_t i = 0; i < 2; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}


TEST_F(forward_list_test, InsertAfterFullElementsListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    int cnt = 0;

    auto iter = sut.before_begin();

    // fill full-1
    for (uint64_t i = 0; i < TESTLISTCAPACITY - 1; ++i)
    {
        iter = sut.emplace_after(iter, i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    sut.insert_after(iter, a);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1 == cnt)
        {
            // for the last element (insert_after) check for different m_value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(forward_list_test, InsertAfterFullElementsListRValue)
{
    constexpr int DEFAULT_VALUE{13};
    int cnt = 0;

    auto iter = sut.before_begin();

    // fill full-1
    for (uint64_t i = 0; i < TESTLISTCAPACITY - 1; ++i)
    {
        iter = sut.emplace_after(iter, i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1 == cnt)
        {
            // for the last element (insert_after) check for different m_value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(forward_list_test, IteratorArrowOperator)
{
    constexpr int DEFAULT_VALUE{13};

    ASSERT_THAT(TESTLISTCAPACITY, Ge(10u)); // for the rest of magic numbers to fit

    // fill half
    for (uint64_t i = 0; i < 5; ++i)
    {
        sut.emplace_front(i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    auto iter = sut.cbegin();
    // 2 increments
    for (uint64_t i = 0; i < 2; ++i)
    {
        ++iter;
    }
    sut.insert_after(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}


TEST_F(forward_list_test, IteratorComparisonOfDifferentLists)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);


    auto iter_sut1 = cut1.begin();
    auto iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.before_begin();
    iter_sut2 = cut2.before_begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.end();
    iter_sut2 = cut2.end();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.begin();
    iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");

    iter_sut1 = cut1.before_begin();
    iter_sut2 = cut2.before_begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");

    iter_sut1 = cut1.end();
    iter_sut2 = cut2.end();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");
}


TEST_F(forward_list_test, ComparingConstIteratorAndIterator)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);


    forward_list<CTorTest, TESTLISTCAPACITY>::const_iterator iter_sut1 = cut1.cbefore_begin();
    forward_list<CTorTest, TESTLISTCAPACITY>::const_iterator iter_sut2 = cut1.cbefore_begin();
    forward_list<CTorTest, TESTLISTCAPACITY>::iterator iter_sut3 = cut1.begin();
    forward_list<CTorTest, TESTLISTCAPACITY>::iterator iter_sut4 = cut1.end();

    ASSERT_THAT(iter_sut1 == iter_sut3, Eq(false));
    ASSERT_THAT(iter_sut3 == iter_sut1, Eq(false));

    ASSERT_THAT(iter_sut1 == iter_sut2, Eq(true));
    ASSERT_THAT(iter_sut4 == iter_sut3, Eq(false));
}


TEST_F(forward_list_test, IteratorTraitsGetValueType)
{
    forward_list<int, 10> cut;

    cut.emplace_front(5);
    auto iter{cut.begin()};

    // using a function call here is closer to the actual use case (-> intentionally did not inline all code here)
    auto ret = iteratorTraitReturnDoubleValue(iter);

    EXPECT_THAT(ret, Eq(10));
}

TEST_F(forward_list_test, IteratorTraitsCheckIteratorCategoryOnConstIterator)
{
    int cnt = 0;
    auto iter = sut.cbefore_begin();
    if (typeid(std::iterator_traits<decltype(iter)>::iterator_category) == typeid(std::random_access_iterator_tag))
    {
        ASSERT_TRUE(1);
    }
    else if (typeid(std::iterator_traits<decltype(iter)>::iterator_category) == typeid(std::forward_iterator_tag))
    {
        cnt = 1;
    }

    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(forward_list_test, EmptyAfterClear)
{
    sut.emplace_front(5);
    sut.clear();
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(forward_list_test, SizeZeroAfterClear)
{
    sut.emplace_front(5);
    sut.clear();
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(forward_list_test, CopyConstructor)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    cut1.emplace_front(101);
    cut1.emplace_front(102);
    EXPECT_THAT(customCTor, Eq(2));

    forward_list<CTorTest, TESTLISTCAPACITY> cut2(cut1);

    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(copyCTor, Eq(2));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(0));
    auto iter = cut2.begin();
    EXPECT_THAT(iter->m_value, Eq(102));
    ++iter;
    EXPECT_THAT(iter->m_value, Eq(101));
    EXPECT_THAT(cut2.empty(), Eq(false));
    EXPECT_THAT(cut2.size(), Eq(2));
}

TEST_F(forward_list_test, CopyConstructorWithEmptyForwardList)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    forward_list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(cut2.size(), Eq(0));
    EXPECT_THAT(cut2.empty(), Eq(true));
}

TEST_F(forward_list_test, CopyConstructorWithFullForwardList)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    decltype(CTorTest::m_value) i = 0;

    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        cut1.emplace_front(i);
    }

    forward_list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
    for (auto& fl : cut2)
    {
        fl.m_value = i;
        ++i;
    }

    EXPECT_THAT(copyCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(i, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.empty(), Eq(false));
}

TEST_F(forward_list_test, MoveConstructor)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    cut1.emplace_front(8101);
    cut1.emplace_front(8102);

    forward_list<CTorTest, TESTLISTCAPACITY> cut2(std::move(cut1));

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(moveCTor, Eq(2));
    EXPECT_THAT(dTor, Eq(2));
    auto iter = cut2.begin();
    EXPECT_THAT(iter->m_value, Eq(8102));
    EXPECT_THAT((++iter)->m_value, Eq(8101));
    EXPECT_THAT(cut2.empty(), Eq(false));
    EXPECT_THAT(cut2.size(), Eq(2));
    EXPECT_THAT(cut1.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveConstructorWithEmptyForwardList)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    forward_list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(cut2.size(), Eq(0));
    EXPECT_THAT(cut2.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveConstructorWithFullForwardList)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        cut1.emplace_front(i);
    }

    forward_list<CTorTest, TESTLISTCAPACITY> cut2(std::move(cut1));

    EXPECT_THAT(moveCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.empty(), Eq(false));
}

TEST_F(forward_list_test, DestructorWithEmptyForwardList)
{
    {
        forward_list<CTorTest, TESTLISTCAPACITY> cut1;
    }
    EXPECT_THAT(dTor, Eq(0));
}

TEST_F(forward_list_test, DestructorSomeElements)
{
    {
        forward_list<CTorTest, TESTLISTCAPACITY> cut1;
        cut1.emplace_front(891);
        cut1.emplace_front(9191);
        cut1.emplace_front(1);
    }
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(3));
    EXPECT_THAT(dTor, Eq(3));
}

TEST_F(forward_list_test, DestructorWithFullForwardList)
{
    {
        forward_list<CTorTest, TESTLISTCAPACITY> cut1;
        for (uint64_t i = 0; i < cut1.capacity(); ++i)
        {
            cut1.emplace_front(1231);
        }
    }

    EXPECT_THAT(dTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(TESTLISTCAPACITY));
}

TEST_F(forward_list_test, CopyAssignmentWithEmptySource)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(812);
    cut1.emplace_front(81122);
    cut1.emplace_front(8132);

    cut1 = cut2;
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cut1.empty(), Eq(true));
}

TEST_F(forward_list_test, CopyAssignmentWithEmptyDestination)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(5812);
    cut1.emplace_front(581122);
    cut1.emplace_front(58132);

    cut2 = cut1;
    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(3));
    EXPECT_THAT(cut2.size(), Eq(3));
    EXPECT_THAT(cut2.empty(), Eq(false));

    auto iter = cut2.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(58132));
    EXPECT_THAT((++iter)->m_value, Eq(581122));
    EXPECT_THAT((++iter)->m_value, Eq(5812));
}


TEST_F(forward_list_test, CopyAssignmentWithLargerDestination)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(5842);
    cut1.emplace_front(584122);
    cut1.emplace_front(58432);
    cut1.emplace_front(58432);

    cut2.emplace_front(313);
    cut2.emplace_front(3131);

    cut1 = cut2;

    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(copyAssignment, Eq(2));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(cut1.size(), Eq(2));
    EXPECT_THAT(cut1.empty(), Eq(false));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(3131));
    EXPECT_THAT((++iter)->m_value, Eq(313));
}

TEST_F(forward_list_test, CopyAssignmentWithLargerSource)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);

    cut2 = cut1;

    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(2));
    EXPECT_THAT(copyCTor, Eq(2));
    EXPECT_THAT(cut2.size(), Eq(4));
    EXPECT_THAT(cut2.empty(), Eq(false));

    auto iter = cut2.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}


TEST_F(forward_list_test, MoveAssignmentWithEmptySource)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(812);
    cut1.emplace_front(81122);
    cut1.emplace_front(8132);

    cut1 = std::move(cut2);

    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cut1.empty(), Eq(true));
}

TEST_F(forward_list_test, MoveAssignmentWithEmptyDestination)
{
    forward_list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(5812);
    cut1.emplace_front(581122);
    cut1.emplace_front(58132);

    cut2 = std::move(cut1);

    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(3));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(3));

    EXPECT_THAT(cut2.size(), Eq(3));
    EXPECT_THAT(cut2.empty(), Eq(false));

    auto iter = cut2.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(58132));
    EXPECT_THAT((++iter)->m_value, Eq(581122));
    EXPECT_THAT((++iter)->m_value, Eq(5812));
}


TEST_F(forward_list_test, MoveAssignmentWithLargerDestination)
{
    forward_list<CTorTest, 10> cut1, cut2;
    cut1.emplace_front(5842);
    cut1.emplace_front(584122);
    cut1.emplace_front(58432);
    cut1.emplace_front(58432);

    cut2.emplace_front(313);
    cut2.emplace_front(3131);

    cut1 = std::move(cut2);

    EXPECT_THAT(dTor, Eq(4));
    EXPECT_THAT(moveAssignment, Eq(2));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(cut1.size(), Eq(2));
    EXPECT_THAT(cut1.empty(), Eq(false));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(3131));
    EXPECT_THAT((++iter)->m_value, Eq(313));
}

TEST_F(forward_list_test, MoveAssignmentWithLargerSource)
{
    forward_list<CTorTest, 10> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);

    cut2 = std::move(cut1);

    EXPECT_THAT(dTor, Eq(4));
    EXPECT_THAT(moveAssignment, Eq(2));
    EXPECT_THAT(moveCTor, Eq(2));
    EXPECT_THAT(cut2.size(), Eq(4));
    EXPECT_THAT(cut2.empty(), Eq(false));


    auto iter = cut2.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(forward_list_test, RemoveDefaultElementFromEmptyList)
{
    auto cnt = sut.remove({});

    EXPECT_THAT(cTor, Eq(1));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}
TEST_F(forward_list_test, RemoveCustomElementFromEmptyList)
{
    auto cnt = sut.remove({10});

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}
TEST_F(forward_list_test, RemoveOneDefaultElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove({});

    EXPECT_THAT(cTor, Eq(3));
    EXPECT_THAT(customCTor, Eq(4));
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(4));
    EXPECT_THAT(cnt, Eq(2));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}
TEST_F(forward_list_test, RemoveOneCustomElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove({1584122});

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(5));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(5));
    EXPECT_THAT(cnt, Eq(1));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}
TEST_F(forward_list_test, RemoveNotExistentElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove({1243});

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(5));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(cut1.size(), Eq(6));
    EXPECT_THAT(cnt, Eq(0));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(forward_list_test, RemoveOnetoEmptyList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);

    auto cnt = cut1.remove({15842});

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(forward_list_test, RemoveWithFewMatches)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove({});

    EXPECT_THAT(cTor, Eq(3));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(1));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(forward_list_test, RemoveWithAllMatches)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove({});

    EXPECT_THAT(cTor, Eq(3));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(forward_list_test, RemoveAllFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove({15842});
    cnt += cut1.remove({});

    EXPECT_THAT(cTor, Eq(3));
    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(dTor, Eq(5));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(3));
}


TEST_F(forward_list_test, RemoveIfFromEmptyList)
{
    auto cnt = sut.remove_if([](const CTorTest& cut) { return true; });

    EXPECT_THAT(isSetupState(), Eq(true));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}


TEST_F(forward_list_test, RemoveIfOneDefaultElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == C_TOR_TEST_VALUE_DEFAULT_VALUE; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(4));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(4));
    EXPECT_THAT(cnt, Eq(2));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(forward_list_test, RemoveIfOneCustomElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == 1584122; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(4));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(cut1.size(), Eq(5));
    EXPECT_THAT(cnt, Eq(1));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(forward_list_test, RemoveIfNotExistentElementFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == 1234; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(4));
    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(cut1.size(), Eq(6));
    EXPECT_THAT(cnt, Eq(0));

    auto iter = cut1.cbefore_begin();
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(forward_list_test, RemoveIfOnetoEmptyList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == 15842; });

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(forward_list_test, RemoveIfWithFewMatches)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == C_TOR_TEST_VALUE_DEFAULT_VALUE; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(1));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(forward_list_test, RemoveIfWithAllMatches)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == C_TOR_TEST_VALUE_DEFAULT_VALUE; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(forward_list_test, RemoveIfAllFromList)
{
    forward_list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == 15842; });
    cnt += cut1.remove_if([](const CTorTest& cut) { return cut.m_value == C_TOR_TEST_VALUE_DEFAULT_VALUE; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(3));
}
