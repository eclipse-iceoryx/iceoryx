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

#include "iceoryx_utils/cxx/vector.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::cxx;

class vector_test : public Test
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
            classValue = value;
        }

        CTorTest(const int value)
            : value(value)
        {
            customCTor++;
            classValue = value;
        }

        CTorTest(const CTorTest& rhs)
        {
            copyCTor++;
            value = rhs.value;
            classValue = value;
        }

        CTorTest(CTorTest&& rhs)
        {
            moveCTor++;
            value = rhs.value;
            classValue = value;
        }

        CTorTest& operator=(const CTorTest& rhs)
        {
            copyAssignment++;
            value = rhs.value;
            classValue = value;
            return *this;
        }

        CTorTest& operator=(CTorTest&& rhs)
        {
            moveAssignment++;
            value = rhs.value;
            classValue = value;
            return *this;
        }

        ~CTorTest()
        {
            dTor++;
            classValue = value;
        }

        int value = 0;
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

    vector<int, 10> sut;
};

int vector_test::cTor;
int vector_test::customCTor;
int vector_test::copyCTor;
int vector_test::moveCTor;
int vector_test::moveAssignment;
int vector_test::copyAssignment;
int vector_test::dTor;
int vector_test::classValue;


TEST_F(vector_test, NewlyCreatedVectorIsEmpty)
{
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(vector_test, NewlyCreatedVectorHasSizeZero)
{
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(vector_test, Capacity)
{
    EXPECT_THAT(sut.capacity(), Eq(10));
}

TEST_F(vector_test, NewVectorWithElementsCTorWithZeroElements)
{
    constexpr uint64_t CAPACITY{42};
    constexpr int DEFAULT_VALUE{13};
    vector<int, CAPACITY> sut(0, DEFAULT_VALUE);
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(vector_test, NewVectorWithElementsCTorWithSomeElements)
{
    constexpr uint64_t CAPACITY{42};
    constexpr uint64_t ELEMENT_COUNT{37};
    constexpr int DEFAULT_VALUE{13};
    vector<int, CAPACITY> sut(ELEMENT_COUNT, DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(ELEMENT_COUNT));
    for (const auto& item : sut)
    {
        EXPECT_THAT(item, Eq(DEFAULT_VALUE));
    }
}

TEST_F(vector_test, NewVectorWithElementsCTorWithCapacityElements)
{
    constexpr uint64_t CAPACITY{42};
    constexpr int DEFAULT_VALUE{13};
    vector<int, CAPACITY> sut(CAPACITY, DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(CAPACITY));
    for (const auto& item : sut)
    {
        EXPECT_THAT(item, Eq(DEFAULT_VALUE));
    }
}

TEST_F(vector_test, NewVectorWithElementsCTorWithMoreThanCapacityElements)
{
    constexpr uint64_t CAPACITY{42};
    constexpr uint64_t ELEMENT_COUNT{73};
    constexpr int DEFAULT_VALUE{13};
    vector<int, CAPACITY> sut(ELEMENT_COUNT, DEFAULT_VALUE);
    EXPECT_THAT(sut.size(), Eq(CAPACITY));
    for (const auto& item : sut)
    {
        EXPECT_THAT(item, Eq(DEFAULT_VALUE));
    }
}

TEST_F(vector_test, EmplaceBackSuccessfullWhenSpaceAvailable)
{
    EXPECT_THAT(sut.emplace_back(5), Eq(true));
}

TEST_F(vector_test, EmplaceBackFailsWhenSpaceNotAvailable)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        EXPECT_THAT(sut.emplace_back(5), Eq(true));
    }
    EXPECT_THAT(sut.emplace_back(5), Eq(false));
}

TEST_F(vector_test, PushBackSuccessfullWhenSpaceAvailableLValue)
{
    const int a{5};
    EXPECT_THAT(sut.push_back(a), Eq(true));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(sut.at(0), Eq(a));
}

TEST_F(vector_test, PushBackFailsWhenSpaceNotAvailableLValue)
{
    const int a{5};
    for (uint64_t i = 0; i < 10; ++i)
    {
        EXPECT_THAT(sut.push_back(a), Eq(true));
    }
    EXPECT_THAT(sut.push_back(a), Eq(false));
}

TEST_F(vector_test, PushBackSuccessfullWhenSpaceAvailableRValue)
{
    EXPECT_THAT(sut.push_back(5), Eq(true));
    ASSERT_THAT(sut.size(), Eq(1u));
    EXPECT_THAT(sut.at(0), Eq(5));
}

TEST_F(vector_test, PushBackFailsWhenSpaceNotAvailableRValue)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        EXPECT_THAT(sut.push_back(5), Eq(true));
    }
    EXPECT_THAT(sut.push_back(5), Eq(false));
}

TEST_F(vector_test, PopBackOnEmptyVector)
{
    sut.pop_back();
    ASSERT_THAT(sut.size(), Eq(0u));
}

TEST_F(vector_test, PopBackNonEmptyVector)
{
    vector<CTorTest, 10> sut;
    sut.emplace_back(101);
    ASSERT_THAT(sut.size(), Eq(1u));
    dTor = 0;
    sut.pop_back();
    ASSERT_THAT(sut.size(), Eq(0u));
    ASSERT_THAT(dTor, Eq(1));
}

TEST_F(vector_test, SizeIncreasesWhenElementIsAdded)
{
    sut.emplace_back(5);
    EXPECT_THAT(sut.size(), Eq(1));
}

TEST_F(vector_test, SizeEqualsCapacityWheFull)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut.emplace_back(5);
    }
    EXPECT_THAT(sut.size(), Eq(sut.capacity()));
}

TEST_F(vector_test, SizeUnchangedWhenEmplaceFails)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut.emplace_back(5);
    }
    EXPECT_THAT(sut.emplace_back(5), Eq(false));
    EXPECT_THAT(sut.size(), Eq(sut.capacity()));
}

TEST_F(vector_test, NotEmptyWhenElementWasAdded)
{
    sut.emplace_back(5);
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(vector_test, EmptyAfterClear)
{
    sut.emplace_back(5);
    sut.clear();
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(vector_test, SizeZeroAfterClear)
{
    sut.emplace_back(5);
    sut.clear();
    EXPECT_THAT(sut.size(), Eq(0));
}

TEST_F(vector_test, CopyConstructor)
{
    vector<CTorTest, 10> sut1;
    sut1.emplace_back(101);
    sut1.emplace_back(102);

    vector<CTorTest, 10> sut2(sut1);
    EXPECT_THAT(copyCTor, Eq(2));
    EXPECT_THAT(sut2.at(0).value, Eq(101));
    EXPECT_THAT(sut2.at(1).value, Eq(102));
    EXPECT_THAT(sut2.empty(), Eq(false));
    EXPECT_THAT(sut2.size(), Eq(2));
}

TEST_F(vector_test, CopyConstructorWithEmptyVector)
{
    vector<CTorTest, 10> sut1;
    vector<CTorTest, 10> sut2(sut1);
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(sut2.size(), Eq(0));
    EXPECT_THAT(sut2.empty(), Eq(true));
}

TEST_F(vector_test, CopyConstructorWithFullVector)
{
    vector<CTorTest, 10> sut1;
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut1.emplace_back(i);
    }

    vector<CTorTest, 10> sut2(sut1);
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut2.at(i) = i;
    }
    EXPECT_THAT(copyCTor, Eq(10));
    EXPECT_THAT(sut2.size(), Eq(10));
    EXPECT_THAT(sut2.empty(), Eq(false));
}

TEST_F(vector_test, MoveConstructor)
{
    vector<CTorTest, 10> sut1;
    sut1.emplace_back(8101);
    sut1.emplace_back(8102);

    vector<CTorTest, 10> sut2(std::move(sut1));
    EXPECT_THAT(moveCTor, Eq(2));
    EXPECT_THAT(sut2.at(0).value, Eq(8101));
    EXPECT_THAT(sut2.at(1).value, Eq(8102));
    EXPECT_THAT(sut2.empty(), Eq(false));
    EXPECT_THAT(sut2.size(), Eq(2));
}

TEST_F(vector_test, MoveConstructorWithEmptyVector)
{
    vector<CTorTest, 10> sut1;
    vector<CTorTest, 10> sut2(sut1);
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(sut2.size(), Eq(0));
    EXPECT_THAT(sut2.empty(), Eq(true));
}

TEST_F(vector_test, MoveConstructorWithFullVector)
{
    vector<CTorTest, 10> sut1;
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut1.emplace_back(i);
    }

    vector<CTorTest, 10> sut2(std::move(sut1));
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut2.at(i) = i;
    }
    EXPECT_THAT(moveCTor, Eq(10));
    EXPECT_THAT(sut2.size(), Eq(10));
    EXPECT_THAT(sut2.empty(), Eq(false));
}

TEST_F(vector_test, DestructorWithEmptyVector)
{
    {
        vector<CTorTest, 10> sut1;
    }
    EXPECT_THAT(dTor, Eq(0));
}

TEST_F(vector_test, DestructorSomeElements)
{
    {
        vector<CTorTest, 10> sut1;
        sut1.emplace_back(891);
        sut1.emplace_back(9191);
        sut1.emplace_back(1);
    }
    EXPECT_THAT(dTor, Eq(3));
}

TEST_F(vector_test, DestructorWithFullVector)
{
    {
        vector<CTorTest, 10> sut1;
        for (uint64_t i = 0; i < sut1.capacity(); ++i)
        {
            sut1.emplace_back(1231);
        }
    }

    EXPECT_THAT(dTor, Eq(10));
}

TEST_F(vector_test, CopyAssignmentWithEmptySource)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(812);
    sut1.emplace_back(81122);
    sut1.emplace_back(8132);

    sut1 = sut2;
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(sut1.size(), Eq(0));
    EXPECT_THAT(sut1.empty(), Eq(true));
}

TEST_F(vector_test, CopyAssignmentWithEmptyDestination)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(5812);
    sut1.emplace_back(581122);
    sut1.emplace_back(58132);

    sut2 = sut1;
    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(3));
    EXPECT_THAT(sut2.size(), Eq(3));
    EXPECT_THAT(sut2.empty(), Eq(false));

    EXPECT_THAT(sut2.at(0).value, Eq(5812));
    EXPECT_THAT(sut2.at(1).value, Eq(581122));
    EXPECT_THAT(sut2.at(2).value, Eq(58132));
}

TEST_F(vector_test, CopyAssignmentWithLargerDestination)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(5842);
    sut1.emplace_back(584122);
    sut1.emplace_back(58432);
    sut1.emplace_back(58432);

    sut2.emplace_back(313);
    sut2.emplace_back(3131);

    sut1 = sut2;
    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(copyAssignment, Eq(2));
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(sut1.size(), Eq(2));
    EXPECT_THAT(sut1.empty(), Eq(false));

    EXPECT_THAT(sut1.at(0).value, Eq(313));
    EXPECT_THAT(sut1.at(1).value, Eq(3131));
}

TEST_F(vector_test, CopyAssignmentWithLargerSource)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(15842);
    sut1.emplace_back(1584122);
    sut1.emplace_back(158432);
    sut1.emplace_back(158432);

    sut2.emplace_back(1313);
    sut2.emplace_back(13131);

    sut2 = sut1;

    EXPECT_THAT(dTor, Eq(0));
    EXPECT_THAT(copyAssignment, Eq(2));
    EXPECT_THAT(copyCTor, Eq(2));
    EXPECT_THAT(sut2.size(), Eq(4));
    EXPECT_THAT(sut2.empty(), Eq(false));

    EXPECT_THAT(sut2.at(0).value, Eq(15842));
    EXPECT_THAT(sut2.at(1).value, Eq(1584122));
    EXPECT_THAT(sut2.at(2).value, Eq(158432));
    EXPECT_THAT(sut2.at(3).value, Eq(158432));
}

TEST_F(vector_test, MoveAssignmentWithEmptySource)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(812);
    sut1.emplace_back(81122);
    sut1.emplace_back(8132);

    sut1 = std::move(sut2);
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(sut1.size(), Eq(0));
    EXPECT_THAT(sut1.empty(), Eq(true));
}

TEST_F(vector_test, MoveAssignmentWithEmptyDestination)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(5812);
    sut1.emplace_back(581122);
    sut1.emplace_back(58132);

    sut2 = std::move(sut1);
    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(3));
    EXPECT_THAT(sut2.size(), Eq(3));
    EXPECT_THAT(sut2.empty(), Eq(false));

    EXPECT_THAT(sut2.at(0).value, Eq(5812));
    EXPECT_THAT(sut2.at(1).value, Eq(581122));
    EXPECT_THAT(sut2.at(2).value, Eq(58132));
}

TEST_F(vector_test, MoveAssignmentWithLargerDestination)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(5842);
    sut1.emplace_back(584122);
    sut1.emplace_back(58432);
    sut1.emplace_back(58432);

    sut2.emplace_back(313);
    sut2.emplace_back(3131);

    sut1 = std::move(sut2);
    EXPECT_THAT(dTor, Eq(4));
    EXPECT_THAT(moveAssignment, Eq(2));
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(sut1.size(), Eq(2));
    EXPECT_THAT(sut1.empty(), Eq(false));

    EXPECT_THAT(sut1.at(0).value, Eq(313));
    EXPECT_THAT(sut1.at(1).value, Eq(3131));
}

TEST_F(vector_test, MoveAssignmentWithLargerSource)
{
    vector<CTorTest, 10> sut1, sut2;
    sut1.emplace_back(15842);
    sut1.emplace_back(1584122);
    sut1.emplace_back(158432);
    sut1.emplace_back(158432);

    sut2.emplace_back(1313);
    sut2.emplace_back(13131);

    sut2 = std::move(sut1);

    EXPECT_THAT(dTor, Eq(4));
    EXPECT_THAT(moveAssignment, Eq(2));
    EXPECT_THAT(moveCTor, Eq(2));
    EXPECT_THAT(sut2.size(), Eq(4));
    EXPECT_THAT(sut2.empty(), Eq(false));

    EXPECT_THAT(sut2.at(0).value, Eq(15842));
    EXPECT_THAT(sut2.at(1).value, Eq(1584122));
    EXPECT_THAT(sut2.at(2).value, Eq(158432));
    EXPECT_THAT(sut2.at(3).value, Eq(158432));
}

TEST_F(vector_test, BeginEndIteratorAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut.begin() == sut.end(), Eq(true));
}

TEST_F(vector_test, BeginEndConstIteratorAreTheSameWhenEmpty)
{
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->begin() == const_cast<const decltype(sut)*>(&sut)->end(),
                Eq(true));
}

TEST_F(vector_test, BeginIteratorComesBeforeEndIteratorWhenNotEmpty)
{
    sut.emplace_back(1);
    EXPECT_THAT(sut.begin() < sut.end(), Eq(true));
}

TEST_F(vector_test, BeginConstIteratorComesBeforeEndConstIteratorWhenNotEmpty)
{
    sut.emplace_back(1);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->begin() < const_cast<const decltype(sut)*>(&sut)->end(),
                Eq(true));
}

TEST_F(vector_test, BeginIteratorComesBeforeEndIteratorWhenFull)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
        sut.emplace_back(i);
    EXPECT_THAT(sut.begin() < sut.end(), Eq(true));
}

TEST_F(vector_test, BeginConstIteratorComesBeforeEndConstIteratorWhenFull)
{
    for (uint64_t i = 0; i < sut.capacity(); ++i)
        sut.emplace_back(i);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->begin() < const_cast<const decltype(sut)*>(&sut)->end(),
                Eq(true));
}

TEST_F(vector_test, IteratorIteratesThroughNonEmptyVector)
{
    constexpr int INITIAL_VALUE{42};
    sut.emplace_back(INITIAL_VALUE);
    sut.emplace_back(INITIAL_VALUE + 1);
    sut.emplace_back(INITIAL_VALUE + 2);
    const int EXPECTED_END_INDEX = sut.size();

    int count = 0;
    for (auto& v : sut)
    {
        EXPECT_THAT(v, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(EXPECTED_END_INDEX));
}

TEST_F(vector_test, ConstIteratorIteratesThroughNonEmptyVector)
{
    constexpr int INITIAL_VALUE{142};
    sut.emplace_back(INITIAL_VALUE);
    sut.emplace_back(INITIAL_VALUE + 1);
    sut.emplace_back(INITIAL_VALUE + 2);
    const int EXPECTED_END_COUNT = sut.size();

    int count = 0;
    for (auto& v : *const_cast<const decltype(sut)*>(&sut))
    {
        EXPECT_THAT(v, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, IteratorIteratesThroughFullVector)
{
    for (uint64_t k = 0; k < sut.capacity(); ++k)
    {
        sut.emplace_back(42 * k);
    }
    const int EXPECTED_END_COUNT = sut.size();

    int i = 0;
    for (auto& v : sut)
    {
        EXPECT_THAT(v, Eq(42 * (i++)));
    }
    EXPECT_THAT(i, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, ConstIteratorIteratesThroughFullVector)
{
    for (uint64_t k = 0; k < sut.capacity(); ++k)
    {
        sut.emplace_back(142 * k);
    }
    const int EXPECTED_END_COUNT = sut.size();

    int i = 0;
    for (const auto& v : *const_cast<const decltype(sut)*>(&sut))
    {
        EXPECT_THAT(v, Eq(142 * (i++)));
    }
    EXPECT_THAT(i, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, IterateUsingData)
{
    sut.emplace_back(127);
    sut.emplace_back(128);
    sut.emplace_back(129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut.data()[k], Eq(127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstData)
{
    sut.emplace_back(3127);
    sut.emplace_back(3128);
    sut.emplace_back(3129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->data()[k], Eq(3127 + k));
    }
}

TEST_F(vector_test, IterateUsingAt)
{
    sut.emplace_back(127);
    sut.emplace_back(128);
    sut.emplace_back(129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut.at(k), Eq(127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstAt)
{
    sut.emplace_back(3127);
    sut.emplace_back(3128);
    sut.emplace_back(3129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->at(k), Eq(3127 + k));
    }
}

TEST_F(vector_test, IterateUsingSquareBracket)
{
    sut.emplace_back(2127);
    sut.emplace_back(2128);
    sut.emplace_back(2129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut[k], Eq(2127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstSquareBracket)
{
    sut.emplace_back(4127);
    sut.emplace_back(4128);
    sut.emplace_back(4129);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT((*const_cast<const decltype(sut)*>(&sut))[k], Eq(4127 + k));
    }
}

TEST_F(vector_test, EraseReturnsNullWhenElementIsInvalid)
{
    auto i = sut.begin() + 5;
    EXPECT_THAT(sut.erase(i), Eq(nullptr));
}

TEST_F(vector_test, ErasingElementDecreasesSize)
{
    sut.emplace_back(3);
    sut.emplace_back(4);
    sut.emplace_back(5);
    sut.erase(sut.begin() + 2);
    sut.erase(sut.begin());
    EXPECT_THAT(sut.size(), Eq(1));
}

TEST_F(vector_test, EraseOfLastElementCallsDTorOnly)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);

    sut1.erase(sut1.begin() + 2);

    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(classValue, Eq(9));
}

TEST_F(vector_test, EraseOfMiddleElementCallsDTorAndMove)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);
    sut1.emplace_back(10);
    sut1.emplace_back(11);

    sut1.erase(sut1.begin() + 2);

    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(moveAssignment, Eq(2));
}

TEST_F(vector_test, EraseOfFrontElementCallsDTorAndMove)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);
    sut1.emplace_back(10);
    sut1.emplace_back(11);

    sut1.erase(sut1.begin());

    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(moveAssignment, Eq(4));
}

TEST_F(vector_test, EraseMiddleElementDataCorrectAfterwards)
{
    sut.emplace_back(97);
    sut.emplace_back(101);
    sut.emplace_back(98);
    sut.emplace_back(99);

    sut.erase(sut.begin() + 1);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut[k], Eq(97 + k));
    }
}

TEST_F(vector_test, EraseFrontElementDataCorrectAfterwards)
{
    sut.emplace_back(6101);
    sut.emplace_back(597);
    sut.emplace_back(598);
    sut.emplace_back(599);

    sut.erase(sut.begin());

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut[k], Eq(597 + k));
    }
}

TEST_F(vector_test, EraseLastElementDataCorrectAfterwards)
{
    sut.emplace_back(7597);
    sut.emplace_back(7598);
    sut.emplace_back(7599);
    sut.emplace_back(7600);
    sut.emplace_back(7601);
    sut.emplace_back(76101);

    sut.erase(sut.begin() + 5);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut[k], Eq(7597 + k));
    }
}

TEST_F(vector_test, EraseLastElementOfFullVectorDataCorrectAfterwards)
{
    for (int i = 0; i < static_cast<int>(sut.capacity()); ++i)
        sut.emplace_back(i * 123);

    sut.erase(sut.begin() + sut.size() - 1);

    for (uint64_t k = 0; k < sut.size(); ++k)
    {
        EXPECT_THAT(sut[k], Eq(k * 123));
    }
}

TEST_F(vector_test, FrontPointsToFirstElement)
{
    sut.emplace_back(1);
    sut.emplace_back(2);
    sut.emplace_back(3);
    EXPECT_THAT(sut.front(), Eq(1));
}

TEST_F(vector_test, BackPointsToLastElement)
{
    sut.emplace_back(4);
    sut.emplace_back(5);
    sut.emplace_back(6);
    EXPECT_THAT(sut.back(), Eq(6));
}

TEST_F(vector_test, ConstFrontPointsToFirstElement)
{
    sut.emplace_back(7);
    sut.emplace_back(8);
    sut.emplace_back(9);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->front(), Eq(7));
}

TEST_F(vector_test, ConstBackPointsToLastElement)
{
    sut.emplace_back(10);
    sut.emplace_back(11);
    sut.emplace_back(12);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->back(), Eq(12));
}

TEST_F(vector_test, ConstructorWithSizeParameterSmallerThanCapacity)
{
    vector<CTorTest, 5> sut(2);
    EXPECT_THAT(vector_test::copyCTor, Eq(2));
    ASSERT_THAT(sut.size(), Eq(2));
}

TEST_F(vector_test, ConstructorWithSizeParameterGreaterThanCapacity)
{
    vector<CTorTest, 5> sut(7);
    EXPECT_THAT(vector_test::copyCTor, Eq(5));
    ASSERT_THAT(sut.size(), Eq(5));
}
