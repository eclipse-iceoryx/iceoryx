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

#include "iceoryx_utils/cxx/list.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::cxx;

static constexpr uint64_t TESTLISTCAPACITY{10u};
static constexpr int C_TOR_TEST_VALUE_DEFAULT_VALUE{-99};

class list_test : public Test
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

        bool operator==(const CTorTest& rhs) const
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

    list<CTorTest, TESTLISTCAPACITY> sut;
};

// list_test statics
int list_test::cTor;
int list_test::customCTor;
int list_test::copyCTor;
int list_test::moveCTor;
int list_test::moveAssignment;
int list_test::copyAssignment;
int list_test::dTor;
int list_test::classValue;

namespace
{
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
} // namespace


TEST_F(list_test, NewlyCreatedListIsEmpty)
{
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(list_test, NewlyCreatedListHasSizeZero)
{
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(list_test, ReadCapacityOnList)
{
    EXPECT_THAT(sut.capacity(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, ReadMax_sizeOnList)
{
    EXPECT_THAT(sut.max_size(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, NewListCTorWithZeroElements)
{
    constexpr uint64_t CAPACITY{42u};
    EXPECT_THAT(isSetupState(), Eq(true));
    const list<int, CAPACITY> cut;
    EXPECT_THAT(cut.empty(), Eq(true));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, CbeginCendAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.cbegin() == sut.cend(), Eq(true));
}
TEST_F(list_test, BeginEndAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.begin() == sut.end(), Eq(true));
}
TEST_F(list_test, CbeginEndAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.cbegin() == sut.end(), Eq(true));
}
TEST_F(list_test, BeginCendAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.begin() == sut.cend(), Eq(true));
}

TEST_F(list_test, CbeginCendAreDifferentWhenFilled)
{
    EXPECT_THAT(sut.emplace_front().m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT(sut.cbegin() != sut.cend(), Eq(true));
}
TEST_F(list_test, BeginEndAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.end(), Eq(true));
}
TEST_F(list_test, CbeginEndAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.cbegin() != sut.end(), Eq(true));
}
TEST_F(list_test, BeginCendAreDifferentWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.begin() != sut.cend(), Eq(true));
}

TEST_F(list_test, NotEmptyWhenFilled)
{
    sut.emplace_front();
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(list_test, NotFullWhenEmpty)
{
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(list_test, NotFullWhenPartialFilled)
{
    sut.emplace_front();
    EXPECT_THAT(TESTLISTCAPACITY, Gt(1));
    EXPECT_THAT(sut.full(), Eq(false));
}
TEST_F(list_test, FullWhenFilledWithCapacityElements)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        EXPECT_THAT(sut.emplace_front().m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    }
    EXPECT_THAT(sut.full(), Eq(true));
}
TEST_F(list_test, FullWhenFilledWithMoreThanCapacityElements)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        sut.emplace_front();
    }

    EXPECT_THAT(sut.full(), Eq(true));
    EXPECT_DEATH(sut.emplace_front(), "");
}
TEST_F(list_test, NotFullWhenFilledWithCapacityAndEraseOneElements)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        sut.emplace_front();
    }
    sut.erase(sut.cbegin());

    EXPECT_THAT(sut.size(), Eq(sut.capacity() - 1));
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(list_test, NotFullWhenFilledWithCapacityAndEraseOneAndReinsertElements)
{
    uint64_t i = 0;
    for (; i < sut.capacity(); ++i)
    {
        sut.emplace_back(i);
    }
    sut.erase(sut.cbegin());
    sut.erase(sut.cbegin());
    sut.emplace_back(i);
    sut.emplace_back(++i);

    i = 1;
    for (auto& element : sut)
    {
        EXPECT_THAT(element, Eq(++i));
    }

    EXPECT_THAT(sut.size(), Eq(sut.capacity()));
    EXPECT_THAT(sut.full(), Eq(true));
}

TEST_F(list_test, CTorWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    list<CTorTest, CAPACITY> cut;

    EXPECT_THAT(cTor, Eq(0));
    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CustomCTorWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    constexpr uint64_t DEFAULT_VALUE{3};
    list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front(DEFAULT_VALUE);
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
    EXPECT_THAT(classValue, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, CTorWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CTorWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace_front();
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, CTorWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<CTorTest, CAPACITY> cut;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.push_front({});
    }
    cut.emplace(cut.cbegin(), 2);

    EXPECT_THAT(cut.size(), Eq(CAPACITY));
    EXPECT_THAT(cTor, Eq(CAPACITY));
    EXPECT_THAT(customCTor, Eq(0));
}


TEST_F(list_test, EmplaceWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.begin();
    decltype(CTorTest::m_value) cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& fl : cut)
    {
        --cnt;
        EXPECT_THAT(fl.m_value, Eq(cnt));
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& fl : cut)
    {
        --cnt;
        EXPECT_THAT(fl.m_value, Eq(cnt));
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace(iter, cnt);
        ++cnt;
    }

    for (auto& fl : cut)
    {
        --cnt;
        EXPECT_THAT(fl.m_value, Eq(cnt));
    }

    EXPECT_THAT(cut.size(), Eq(ELEMENT_COUNT));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT));
}

TEST_F(list_test, EmplaceWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        iter = cut.emplace(iter, cnt);
        ++cnt;
    }

    cnt = CAPACITY;
    for (auto& fl : cut)
    {
        --cnt;
        EXPECT_THAT(fl.m_value, Eq(cnt));
    }

    EXPECT_THAT(cut.size(), Eq(CAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(CAPACITY));
}


TEST_F(list_test, EmplaceReverseWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{3};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace(iter, cnt);
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

TEST_F(list_test, EmplaceReverseWithWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut.emplace(iter, cnt);
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


TEST_F(list_test, EmplaceBackWithOneElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{1};
    list<CTorTest, CAPACITY> cut;
    // CTorTest compareElement{};
    decltype(CTorTest::m_value) cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(cut.emplace_back(cnt), Eq(CTorTest{cnt}));
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
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT * 2));
}

TEST_F(list_test, EmplaceBackWithSomeElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{37};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    decltype(CTorTest::m_value) cnt = 0;

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(cut.emplace_back(cnt), Eq(CTorTest{cnt}));
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
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT * 2));
}

TEST_F(list_test, EmplaceBackWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    decltype(CTorTest::m_value) cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        EXPECT_THAT(cut.emplace_back(cnt), Eq(CTorTest{cnt}));
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
    EXPECT_THAT(customCTor, Eq(ELEMENT_COUNT * 2));
}

TEST_F(list_test, EmplaceBackWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{CAPACITY + 1};
    list<CTorTest, CAPACITY> cut;
    auto iter = cut.cbegin();
    decltype(CTorTest::m_value) cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        if (i < CAPACITY)
        {
            EXPECT_THAT(cut.emplace_back(cnt), Eq(CTorTest{cnt}));
        }
        else
        {
            EXPECT_DEATH(cut.emplace_back(cnt), "");
        }
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
    EXPECT_THAT(customCTor, Eq(CAPACITY * 2));
}

TEST_F(list_test, EmplaceWithWrongListIterator)
{
    constexpr uint64_t CAPACITY{42u};
    constexpr uint64_t ELEMENT_COUNT{13};
    list<CTorTest, CAPACITY> cut1, cut2;
    auto iterOfCut1 = cut1.begin();
    auto iterOfCut2 = cut2.begin();
    uint64_t cnt = 0;

    for (uint64_t i = 0; i < ELEMENT_COUNT; ++i)
    {
        cut1.emplace(iterOfCut1, cnt);
        ++cnt;
    }

    EXPECT_DEATH(cut1.emplace(iterOfCut2, cnt), "");
}

TEST_F(list_test, PushFrontConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontConstSuccessfullWhenSpaceAvailableLValue)
{
    const CTorTest a{};
    EXPECT_TRUE(sut.push_front(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(1));
    EXPECT_THAT(customCTor, Eq(0u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontFailsWhenSpaceNotAvailableLValue)
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

TEST_F(list_test, PushFrontSuccessfullWhenSpaceAvailableRValue)
{
    constexpr int DEFAULT_VALUE{13};

    EXPECT_THAT(sut.size(), Eq(0));

    sut.push_front(DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(1));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushFrontFailsWhenSpaceNotAvailableRValue)
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


TEST_F(list_test, PushBackConstCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_back(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushBackCustomSuccessfullWhenSpaceAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    CTorTest a{DEFAULT_VALUE};
    EXPECT_TRUE(sut.push_back(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushBackConstSuccessfullWhenSpaceAvailableLValue)
{
    const CTorTest a{};
    EXPECT_TRUE(sut.push_back(a));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(cTor, Eq(1));
    EXPECT_THAT(customCTor, Eq(0u));
    EXPECT_THAT((*sut.begin()).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
}

TEST_F(list_test, PushBackFailsWhenSpaceNotAvailableLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    EXPECT_THAT(sut.size(), Eq(0));
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_back(a));
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }
    EXPECT_FALSE(sut.push_back(a));
    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, PushBackSuccessfullWhenSpaceAvailableRValue)
{
    constexpr int DEFAULT_VALUE{13};

    EXPECT_THAT(sut.size(), Eq(0));

    sut.push_back(DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(1));
    EXPECT_THAT((*sut.begin()).m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PushBackFailsWhenSpaceNotAvailableRValue)
{
    constexpr int DEFAULT_VALUE{13};
    uint64_t i = 0;

    EXPECT_THAT(sut.size(), Eq(0));
    for (; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_back(DEFAULT_VALUE));
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    EXPECT_FALSE(sut.push_back(DEFAULT_VALUE));

    EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(DEFAULT_VALUE));
    }
}

TEST_F(list_test, PushBackCheckInsertPosition)
{
    constexpr int DEFAULT_VALUE{13};
    uint64_t i = 0;

    for (; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_TRUE(sut.push_back(i));
    }

    i = 0;
    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(i));
        ++i;
    }
}

TEST_F(list_test, AccessFrontElement)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    CTorTest& b{sut.front()};
    const CTorTest& c{sut.front()};
    EXPECT_THAT(b.m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessFrontElementFromConstList)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front({});
    sut.push_front(a);

    const list<CTorTest, TESTLISTCAPACITY> cut{sut};
    const CTorTest& c = cut.front();

    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessBackElement)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front(a);
    sut.push_front({});

    CTorTest& b{sut.back()};
    const CTorTest& c{sut.back()};
    EXPECT_THAT(b.m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, AccessBackElementFromConstList)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.push_front(a);
    sut.push_front({});

    const list<CTorTest, TESTLISTCAPACITY> cut{sut};
    const CTorTest& c = cut.back();

    EXPECT_THAT(c.m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, PopFrontOnEmptyList)
{
    EXPECT_FALSE(sut.pop_front());
    ASSERT_THAT(sut.size(), Eq(0u));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, PopFrontNonEmptyList)
{
    sut.emplace_front(101);
    ASSERT_THAT(sut.size(), Eq(1u));

    EXPECT_TRUE(sut.pop_front());

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(0u));
    EXPECT_THAT(customCTor, Eq(1u));
    ASSERT_THAT(dTor, Eq(1u));
}

TEST_F(list_test, PopFrontFullToEmptyList)
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

TEST_F(list_test, PopFrontFullPlusOneToEmptyList)
{
    // fill even more than size
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace(sut.cbegin());
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


TEST_F(list_test, PopBackOnEmptyList)
{
    EXPECT_FALSE(sut.pop_back());
    ASSERT_THAT(sut.size(), Eq(0u));
    EXPECT_THAT(isSetupState(), Eq(true));
}

TEST_F(list_test, PopBackNonEmptyList)
{
    sut.emplace_front(101);
    ASSERT_THAT(sut.size(), Eq(1u));

    EXPECT_TRUE(sut.pop_back());

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(0u));
    EXPECT_THAT(customCTor, Eq(1u));
    ASSERT_THAT(dTor, Eq(1u));
}

TEST_F(list_test, PopBackFullToEmptyList)
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
        EXPECT_TRUE(sut.pop_back());
    }

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(dTor, Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, PopBackFullPlusOneToEmptyList)
{
    // fill even more than size
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace(sut.cbegin());
        EXPECT_THAT(sut.size(), Eq((i + 1) > TESTLISTCAPACITY ? TESTLISTCAPACITY : (i + 1)));
    }

    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(TESTLISTCAPACITY - i));
        EXPECT_TRUE(sut.pop_back());
    }

    EXPECT_FALSE(sut.pop_back());

    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(cTor, Eq(TESTLISTCAPACITY));
    ASSERT_THAT(customCTor, Eq(0u));
    ASSERT_THAT(dTor, Eq(TESTLISTCAPACITY));
}


TEST_F(list_test, InsertEmptyListAsLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.insert(sut.cbegin(), a);

    ASSERT_THAT(sut.size(), Eq(1u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(1u));
}

TEST_F(list_test, InsertLValueCheckReturn)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    auto iter = sut.insert(sut.begin(), a);

    ASSERT_THAT(iter == sut.begin(), Eq(true));
}

TEST_F(list_test, InsertEmptyListAsRValue)
{
    sut.insert(sut.begin(), {10});

    ASSERT_THAT(sut.size(), Eq(1u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(1u));
}

TEST_F(list_test, InsertRValueCheckReturn)
{
    auto iter = sut.insert(sut.begin(), {10});

    ASSERT_THAT(iter == sut.begin(), Eq(true));
    ASSERT_THAT((*iter).m_value, Eq(10));
}

TEST_F(list_test, InsertBeginListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.insert(sut.begin(), a);
    sut.emplace_front();

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT(iter->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
}


TEST_F(list_test, InsertBeforeBeginListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.emplace_front();
    sut.insert(sut.begin(), a);

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
}

TEST_F(list_test, InsertBeforeBeginListRValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};

    sut.emplace_front(a);
    sut.insert(sut.begin(), {});

    ASSERT_THAT(sut.size(), Eq(2u));
    ASSERT_THAT(cTor, Eq(1u));
    ASSERT_THAT(customCTor, Eq(1u));
    auto iter = sut.begin();
    EXPECT_THAT((*iter).m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
}

TEST_F(list_test, InsertSomeElementsListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    uint64_t loopCounter = 0;

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
    sut.insert(iter, a);

    for (auto& x : sut)
    {
        ++loopCounter;
    }

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(loopCounter, Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.begin();
    EXPECT_THAT(iter->m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}

TEST_F(list_test, InsertSomeElementsListRValue)
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
    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.begin();
    EXPECT_THAT(iter->m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}


TEST_F(list_test, InsertFullElementsListLValue)
{
    constexpr int DEFAULT_VALUE{13};
    const CTorTest a{DEFAULT_VALUE};
    int cnt = 0;

    auto iter = sut.begin();

    // fill full-1
    for (uint64_t i = 0; i < TESTLISTCAPACITY - 1; ++i)
    {
        sut.emplace(iter, i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    sut.insert(iter, a);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1 == cnt)
        {
            // for the last element (insert) check for different m_value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(list_test, InsertFullElementsListRValue)
{
    constexpr int DEFAULT_VALUE{13};
    int cnt = 0;

    auto iter = sut.begin();

    // fill full-1
    for (uint64_t i = 0; i < TESTLISTCAPACITY - 1; ++i)
    {
        sut.emplace(iter, i);
        EXPECT_THAT(sut.size(), Eq(i + 1));
    }

    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(TESTLISTCAPACITY));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(TESTLISTCAPACITY));

    for (auto& fl : sut)
    {
        EXPECT_THAT(fl.m_value, Eq(cnt));
        ++cnt;
        if (TESTLISTCAPACITY - 1 == cnt)
        {
            // for the last element (insert) check for different m_value
            cnt = DEFAULT_VALUE;
        }
    }
}

TEST_F(list_test, IteratorArrowOperator)
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
    sut.insert(iter, DEFAULT_VALUE);

    ASSERT_THAT(sut.size(), Eq(6u));
    ASSERT_THAT(cTor, Eq(0u));
    ASSERT_THAT(customCTor, Eq(6u));

    iter = sut.cbegin();
    EXPECT_THAT(iter->m_value, Eq(4));
    EXPECT_THAT((++iter)->m_value, Eq(3));
    EXPECT_THAT((++iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(2));
    EXPECT_THAT((++iter)->m_value, Eq(1));
    EXPECT_THAT((++iter)->m_value, Eq(0));
}

TEST_F(list_test, IteratorIncrementOperatorBeyondEnd)
{
    constexpr int DEFAULT_VALUE{13};

    // fill
    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.begin();
    EXPECT_THAT(iter->m_value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}

TEST_F(list_test, ConstIteratorIncrementOperatorBeyondEnd)
{
    constexpr int DEFAULT_VALUE{13};

    // fill
    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.cbegin();
    EXPECT_THAT(iter->m_value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((++iter) == sut.cend());
    EXPECT_TRUE((++iter) == sut.cend());
}

TEST_F(list_test, IteratorDecrementOperatorBeyondBegin)
{
    constexpr int DEFAULT_VALUE{13};

    // fill
    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.end();
    EXPECT_THAT((--iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((--iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}

TEST_F(list_test, ConstIteratorDecrementOperatorBeyondBegin)
{
    constexpr int DEFAULT_VALUE{13};

    // fill
    sut.push_front(DEFAULT_VALUE);

    auto iter = sut.cend();
    EXPECT_THAT((--iter)->m_value, Eq(DEFAULT_VALUE));
    EXPECT_TRUE((--iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}

TEST_F(list_test, IteratorDecrementOperatorBeyondBeginWithFullList)
{
    constexpr int DEFAULT_VALUE{13};

    // fill
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        sut.emplace_front(i);
    }

    auto iter = sut.end();
    for (uint64_t i = 0; i < sut.capacity(); ++i)
    {
        EXPECT_THAT((--iter)->m_value, Eq(i));
    }
    EXPECT_TRUE((iter) == sut.cbegin());
    EXPECT_TRUE((--iter) == sut.cbegin());
}


TEST_F(list_test, IteratorComparisonOfDifferentLists)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);


    auto iter_sut1 = cut1.begin();
    auto iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.begin();
    iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.end();
    iter_sut2 = cut2.end();
    EXPECT_DEATH(dummyFunc(iter_sut1 == iter_sut2), "");

    iter_sut1 = cut1.begin();
    iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");

    iter_sut1 = cut1.begin();
    iter_sut2 = cut2.begin();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");

    iter_sut1 = cut1.end();
    iter_sut2 = cut2.end();
    EXPECT_DEATH(dummyFunc(iter_sut1 != iter_sut2), "");
}


TEST_F(list_test, ComparingConstIteratorAndIterator)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(15842);
    cut1.emplace_front(1584122);
    cut1.emplace_front(158432);
    cut1.emplace_front(158432);

    cut2.emplace_front(1313);
    cut2.emplace_front(13131);


    list<CTorTest, TESTLISTCAPACITY>::const_iterator iter_sut1 = cut1.cbegin();
    list<CTorTest, TESTLISTCAPACITY>::const_iterator iter_sut2 = cut1.cend();
    list<CTorTest, TESTLISTCAPACITY>::iterator iter_sut3 = cut1.begin();
    list<CTorTest, TESTLISTCAPACITY>::iterator iter_sut4 = cut1.end();

    ASSERT_THAT(iter_sut1 == iter_sut3, Eq(true));
    ASSERT_THAT(iter_sut3 == iter_sut1, Eq(true));

    ASSERT_THAT(iter_sut1 == iter_sut2, Eq(false));
    ASSERT_THAT(iter_sut4 == iter_sut3, Eq(false));
}


TEST_F(list_test, IteratorTraitsGetValueType)
{
    list<int, 10> cut;

    cut.emplace_front(5);
    auto iter{cut.begin()};

    // using a function call here is closer to the actual use case (-> intentionally did not inline all code here)
    auto ret = iteratorTraitReturnDoubleValue(iter);

    EXPECT_THAT(ret, Eq(10));
}

TEST_F(list_test, IteratorTraitsCheckIteratorCategoryOnConstIterator)
{
    int cnt = 0;
    auto iter = sut.cbegin();
    if (typeid(std::iterator_traits<decltype(iter)>::iterator_category) == typeid(std::random_access_iterator_tag))
    {
        ASSERT_TRUE(1);
    }
    else if (typeid(std::iterator_traits<decltype(iter)>::iterator_category) == typeid(std::bidirectional_iterator_tag))
    {
        cnt = 1;
    }

    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(list_test, EmptyAfterClear)
{
    sut.emplace_front(5);
    sut.clear();
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(list_test, SizeZeroAfterClear)
{
    sut.emplace_front(5);
    sut.clear();
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(list_test, CopyConstructor)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    cut1.emplace_front(101);
    cut1.emplace_front(102);
    EXPECT_THAT(customCTor, Eq(2));

    list<CTorTest, TESTLISTCAPACITY> cut2(cut1);

    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(copyCTor, Eq(2));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(0));
    auto iter = cut2.begin();
    EXPECT_THAT(iter->m_value, Eq(102));
    EXPECT_THAT((++iter)->m_value, Eq(101));
    EXPECT_THAT(cut2.empty(), Eq(false));
    EXPECT_THAT(cut2.size(), Eq(2));
}

TEST_F(list_test, CopyConstructorWithEmptyList)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(cut2.size(), Eq(0));
    EXPECT_THAT(cut2.empty(), Eq(true));
}

TEST_F(list_test, CopyConstructorWithFullList)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    decltype(CTorTest::m_value) i = 0;

    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        cut1.emplace_front(i);
    }

    list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
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

TEST_F(list_test, MoveConstructor)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    cut1.emplace_front(8101);
    cut1.emplace_front(8102);

    list<CTorTest, TESTLISTCAPACITY> cut2(std::move(cut1));

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(moveCTor, Eq(2));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(dTor, Eq(2));
    auto iter = cut2.begin();
    EXPECT_THAT(iter->m_value, Eq(8102));
    EXPECT_THAT((++iter)->m_value, Eq(8101));
    EXPECT_THAT(cut2.empty(), Eq(false));
    EXPECT_THAT(cut2.size(), Eq(2));
    EXPECT_THAT(cut1.empty(), Eq(true));
}

TEST_F(list_test, MoveConstructorWithEmptyList)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    list<CTorTest, TESTLISTCAPACITY> cut2(cut1);
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(cut2.size(), Eq(0));
    EXPECT_THAT(cut2.empty(), Eq(true));
}

TEST_F(list_test, MoveConstructorWithFullList)
{
    list<CTorTest, TESTLISTCAPACITY> cut1;
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        cut1.emplace_front(i);
    }

    list<CTorTest, TESTLISTCAPACITY> cut2(std::move(cut1));

    EXPECT_THAT(moveCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.size(), Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cut2.empty(), Eq(false));
}

TEST_F(list_test, DestructorWithEmptyList)
{
    {
        list<CTorTest, TESTLISTCAPACITY> cut1;
    }
    EXPECT_THAT(dTor, Eq(0));
}

TEST_F(list_test, DestructorSomeElements)
{
    {
        list<CTorTest, TESTLISTCAPACITY> cut1;
        cut1.emplace_front(891);
        cut1.emplace_front(9191);
        cut1.emplace_front(1);
    }
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(3));
    EXPECT_THAT(dTor, Eq(3));
}

TEST_F(list_test, DestructorWithFullList)
{
    {
        list<CTorTest, TESTLISTCAPACITY> cut1;
        for (uint64_t i = 0; i < cut1.capacity(); ++i)
        {
            cut1.emplace_front(1231);
        }
    }

    EXPECT_THAT(dTor, Eq(TESTLISTCAPACITY));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(TESTLISTCAPACITY));
}

TEST_F(list_test, CopyAssignmentWithEmptySource)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
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

TEST_F(list_test, CopyAssignmentWithEmptyDestination)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(5812);
    cut1.emplace_front(581122);
    cut1.emplace_front(58132);

    cut2 = cut1;
    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(3));
    EXPECT_THAT(cut2.size(), Eq(3));
    EXPECT_THAT(cut2.empty(), Eq(false));

    auto iter = cut2.cbegin();
    EXPECT_THAT(iter->m_value, Eq(58132));
    EXPECT_THAT((++iter)->m_value, Eq(581122));
    EXPECT_THAT((++iter)->m_value, Eq(5812));
}


TEST_F(list_test, CopyAssignmentWithLargerDestination)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT(iter->m_value, Eq(3131));
    EXPECT_THAT((++iter)->m_value, Eq(313));
}

TEST_F(list_test, CopyAssignmentWithLargerSource)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
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

    auto iter = cut2.cbegin();
    EXPECT_THAT(iter->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}


TEST_F(list_test, MoveAssignmentWithEmptySource)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
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

TEST_F(list_test, MoveAssignmentWithEmptyDestination)
{
    list<CTorTest, TESTLISTCAPACITY> cut1, cut2;
    cut1.emplace_front(5812);
    cut1.emplace_front(581122);
    cut1.emplace_front(58132);

    cut2 = std::move(cut1);

    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(moveCTor, Eq(3));
    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(3));

    EXPECT_THAT(cut2.size(), Eq(3));
    EXPECT_THAT(cut2.empty(), Eq(false));

    auto iter = cut2.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(58132));
    EXPECT_THAT((++iter)->m_value, Eq(581122));
    EXPECT_THAT((++iter)->m_value, Eq(5812));
}


TEST_F(list_test, MoveAssignmentWithLargerDestination)
{
    list<CTorTest, 10> cut1, cut2;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(3131));
    EXPECT_THAT((++iter)->m_value, Eq(313));
}

TEST_F(list_test, MoveAssignmentWithLargerSource)
{
    list<CTorTest, 10> cut1, cut2;
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


    auto iter = cut2.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(list_test, RemoveDefaultElementFromEmptyList)
{
    auto cnt = sut.remove({});

    EXPECT_THAT(cTor, Eq(1));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}
TEST_F(list_test, RemoveCustomElementFromEmptyList)
{
    auto cnt = sut.remove({10});

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}
TEST_F(list_test, RemoveOneDefaultElementFromList)
{
    list<CTorTest, 10> cut1;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}
TEST_F(list_test, RemoveOneCustomElementFromList)
{
    list<CTorTest, 10> cut1;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}
TEST_F(list_test, RemoveNotExistentElementFromList)
{
    list<CTorTest, 10> cut1;
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
    EXPECT_THAT(classValue, Eq(1243));
    EXPECT_THAT(cut1.size(), Eq(6));
    EXPECT_THAT(cnt, Eq(0));

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(list_test, RemoveOnetoEmptyList)
{
    list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);

    auto cnt = cut1.remove({15842});

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(2));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(list_test, RemoveWithFewMatches)
{
    list<CTorTest, 10> cut1;
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

TEST_F(list_test, RemoveWithAllMatches)
{
    list<CTorTest, 10> cut1;
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove({});

    EXPECT_THAT(cTor, Eq(3));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(list_test, RemoveAllFromList)
{
    list<CTorTest, 10> cut1;
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


TEST_F(list_test, RemoveIfFromEmptyList)
{
    auto cnt = sut.remove_if([](const CTorTest& cut) { return true; });

    EXPECT_THAT(isSetupState(), Eq(true));
    EXPECT_THAT(sut.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(0));
}


TEST_F(list_test, RemoveIfOneDefaultElementFromList)
{
    list<CTorTest, 10> cut1;
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
    EXPECT_THAT(classValue, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT(cut1.size(), Eq(4));
    EXPECT_THAT(cnt, Eq(2));

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(list_test, RemoveIfOneCustomElementFromList)
{
    list<CTorTest, 10> cut1;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(list_test, RemoveIfNotExistentElementFromList)
{
    list<CTorTest, 10> cut1;
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

    auto iter = cut1.cbegin();
    EXPECT_THAT((iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(158432));
    EXPECT_THAT((++iter)->m_value, Eq(1584122));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(C_TOR_TEST_VALUE_DEFAULT_VALUE));
    EXPECT_THAT((++iter)->m_value, Eq(15842));
}

TEST_F(list_test, RemoveIfOnetoEmptyList)
{
    list<CTorTest, 10> cut1;
    cut1.emplace_front(15842);

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == 15842; });

    EXPECT_THAT(cTor, Eq(0));
    EXPECT_THAT(customCTor, Eq(1));
    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(1));
}

TEST_F(list_test, RemoveIfWithFewMatches)
{
    list<CTorTest, 10> cut1;
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

TEST_F(list_test, RemoveIfWithAllMatches)
{
    list<CTorTest, 10> cut1;
    cut1.emplace_front();
    cut1.emplace_front();

    auto cnt = cut1.remove_if([](const CTorTest& cut) { return cut.m_value == C_TOR_TEST_VALUE_DEFAULT_VALUE; });

    EXPECT_THAT(cTor, Eq(2));
    EXPECT_THAT(customCTor, Eq(0));
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(cut1.size(), Eq(0));
    EXPECT_THAT(cnt, Eq(2));
}

TEST_F(list_test, RemoveIfAllFromList)
{
    list<CTorTest, 10> cut1;
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


TEST_F(list_test, invalidIteratorErase)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);

    EXPECT_DEATH(sut.erase(iter), "");
}

TEST_F(list_test, invalidIteratorIncrement)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);

    EXPECT_DEATH(++iter, "");
}

TEST_F(list_test, invalidIteratorDecrement)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    sut.erase(iter);

    EXPECT_DEATH(--iter, "");
}

TEST_F(list_test, invalidIteratorComparison)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 = sut.erase(iter);

    EXPECT_DEATH(sut.cbegin() == iter, "");
}

TEST_F(list_test, invalidIteratorComparisonUnequal)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 = sut.erase(iter);

    EXPECT_DEATH(dummyFunc(iter2 != iter), "");
}

TEST_F(list_test, invalidIteratorDereferencing)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 = sut.erase(iter);

    EXPECT_DEATH(dummyFunc((*iter).m_value), "");
}

TEST_F(list_test, invalidIteratorAddressOfOperator)
{
    for (uint64_t i = 0; i < TESTLISTCAPACITY; ++i)
    {
        sut.emplace_back((const uint64_t)i);
    }

    auto iter = sut.cbegin();
    ++iter;
    auto iter2 = sut.erase(iter);

    EXPECT_DEATH(dummyFunc(iter->m_value == 12), "");
}