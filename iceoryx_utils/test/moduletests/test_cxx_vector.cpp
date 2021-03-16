// Copyright (c) 2019,2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/vector.hpp"
#include "test.hpp"

#include <vector>


using namespace ::testing;
using namespace iox::cxx;

/// Note: Where applicable, vectors of capacity 0 are tested separately,
///       since the vector implementation is specialized for capacity 0.
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
    static int dTorClassValue;

    static std::vector<int> dtorOrder;

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
            dTorClassValue = value;
            dtorOrder.emplace_back(value);
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
        dTorClassValue = 0;
        dtorOrder.clear();
    }

    vector<int, 5> sut5;
    vector<int, 0> sut0;
};

int vector_test::cTor;
int vector_test::customCTor;
int vector_test::copyCTor;
int vector_test::moveCTor;
int vector_test::moveAssignment;
int vector_test::copyAssignment;
int vector_test::dTor;
int vector_test::classValue;
int vector_test::dTorClassValue;
std::vector<int> vector_test::dtorOrder;


TEST_F(vector_test, NewlyCreatedVectorIsEmpty)
{
    EXPECT_THAT(sut5.empty(), Eq(true));
    EXPECT_THAT(sut0.empty(), Eq(true));
}

TEST_F(vector_test, NewlyCreatedVectorHasSizeZero)
{
    EXPECT_THAT(sut5.size(), Eq(0));
    EXPECT_THAT(sut0.size(), Eq(0));
}

TEST_F(vector_test, Capacity)
{
    EXPECT_THAT(sut5.capacity(), Eq(5));
    EXPECT_THAT(sut0.capacity(), Eq(0));
}

TEST_F(vector_test, NewVectorWithElementsCTorWithZeroElements)
{
    constexpr uint64_t CAPACITY{42};
    constexpr int DEFAULT_VALUE{13};
    vector<int, CAPACITY> sutC(0, DEFAULT_VALUE);
    EXPECT_THAT(sutC.empty(), Eq(true));
    vector<int, 0> sut0(0, DEFAULT_VALUE);
    EXPECT_THAT(sut0.empty(), Eq(true));
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
    vector<int, CAPACITY> sutC(ELEMENT_COUNT, DEFAULT_VALUE);
    EXPECT_THAT(sutC.size(), Eq(CAPACITY));
    for (const auto& item : sutC)
    {
        EXPECT_THAT(item, Eq(DEFAULT_VALUE));
    }
    vector<int, 0> sut0(ELEMENT_COUNT, DEFAULT_VALUE);
    EXPECT_THAT(sut0.size(), Eq(0));
}


TEST_F(vector_test, EmplaceBackSuccessfullWhenSpaceAvailable)
{
    EXPECT_THAT(sut5.emplace_back(5), Eq(true));
}

TEST_F(vector_test, EmplaceBackFailsWhenSpaceNotAvailable)
{
    for (uint64_t i = 0; i < 5; ++i)
    {
        EXPECT_THAT(sut5.emplace_back(5), Eq(true));
    }
    EXPECT_THAT(sut5.emplace_back(5), Eq(false));
}

TEST_F(vector_test, EmplaceBackFailsForCapacityZeroVector)
{
    EXPECT_THAT(sut0.emplace_back(5), Eq(false));
}

TEST_F(vector_test, PushBackSuccessfullWhenSpaceAvailableLValue)
{
    const int a{5};
    EXPECT_THAT(sut5.push_back(a), Eq(true));
    ASSERT_THAT(sut5.size(), Eq(1u));
    EXPECT_THAT(sut5.at(0), Eq(a));
}

TEST_F(vector_test, PushBackFailsWhenSpaceNotAvailableLValue)
{
    const int a{5};
    for (uint64_t i = 0; i < 5; ++i)
    {
        EXPECT_THAT(sut5.push_back(a), Eq(true));
    }
    EXPECT_THAT(sut5.push_back(a), Eq(false));
}

TEST_F(vector_test, PushBackAlwaysFailsForVectorCapacityZero)
{
    const int a{5};
    EXPECT_THAT(sut0.push_back(a), Eq(false));
}

TEST_F(vector_test, PushBackSuccessfullWhenSpaceAvailableRValue)
{
    EXPECT_THAT(sut5.push_back(5), Eq(true));
    ASSERT_THAT(sut5.size(), Eq(1u));
    EXPECT_THAT(sut5.at(0), Eq(5));
}

TEST_F(vector_test, PushBackFailsWhenSpaceNotAvailableRValue)
{
    for (uint64_t i = 0; i < 5; ++i)
    {
        EXPECT_THAT(sut5.push_back(5), Eq(true));
    }
    EXPECT_THAT(sut5.push_back(5), Eq(false));
}

TEST_F(vector_test, PushBackAlwaysFailsForVectorCapacityZeroRValue)
{
    EXPECT_THAT(sut0.push_back(5), Eq(false));
}

TEST_F(vector_test, PopBackOnEmptyVector)
{
    sut5.pop_back();
    ASSERT_THAT(sut5.size(), Eq(0u));
    sut0.pop_back();
    ASSERT_THAT(sut0.size(), Eq(0u));
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
    sut5.emplace_back(5);
    EXPECT_THAT(sut5.size(), Eq(1));
}

TEST_F(vector_test, SizeEqualsCapacityIfFull)
{
    for (uint64_t i = 0; i < 5; ++i)
    {
        sut5.emplace_back(5);
    }
    EXPECT_THAT(sut5.size(), Eq(sut5.capacity()));
}

TEST_F(vector_test, FullVectorIsFull)
{
    for (uint64_t i = 0; i < 5; ++i)
    {
        sut5.emplace_back(5);
    }
    EXPECT_THAT(sut5.full(), Eq(true));
    EXPECT_THAT(sut0.full(), Eq(true));
}


TEST_F(vector_test, SizeEqualsCapacityForCapacityZeroVector)
{
    EXPECT_THAT(sut0.size(), Eq(sut0.capacity()));
}

TEST_F(vector_test, SizeUnchangedWhenEmplaceFails)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        sut5.emplace_back(5);
    }
    EXPECT_THAT(sut5.emplace_back(5), Eq(false));
    EXPECT_THAT(sut5.size(), Eq(sut5.capacity()));
}

TEST_F(vector_test, NotEmptyAndNotFullWhenElementWasAdded)
{
    sut5.emplace_back(5);
    EXPECT_THAT(sut5.empty(), Eq(false));
    EXPECT_THAT(sut5.full(), Eq(false));
}

TEST_F(vector_test, EmptyAfterClear)
{
    sut5.emplace_back(5);
    sut5.clear();
    EXPECT_THAT(sut5.empty(), Eq(true));
    sut0.clear();
    EXPECT_THAT(sut0.empty(), Eq(true));
}

TEST_F(vector_test, SizeZeroAfterClear)
{
    sut5.emplace_back(5);
    sut5.clear();
    EXPECT_THAT(sut5.size(), Eq(0));
    sut0.clear();
    EXPECT_THAT(sut0.size(), Eq(0));
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

    vector<CTorTest, 0> sut0_1;
    vector<CTorTest, 0> sut0_2(sut0_1);
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(sut0_2.size(), Eq(0));
    EXPECT_THAT(sut0_2.empty(), Eq(true));
}

TEST_F(vector_test, CopyConstructorWithCapacityZeroVector)
{
    vector<CTorTest, 0> sut0_1;
    vector<CTorTest, 0> sut0_2(sut0_1);
    EXPECT_THAT(copyCTor, Eq(0));
    EXPECT_THAT(sut0_2.size(), Eq(0));
    EXPECT_THAT(sut0_2.empty(), Eq(true));
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

TEST_F(vector_test, MoveConstructorWithCapacityZeroVector)
{
    vector<CTorTest, 0> sut0_1;
    vector<CTorTest, 0> sut0_2(sut0_1);
    EXPECT_THAT(moveCTor, Eq(0));
    EXPECT_THAT(sut0_2.size(), Eq(0));
    EXPECT_THAT(sut0_2.empty(), Eq(true));
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

TEST_F(vector_test, DestructorWithEmptyVectorCapacity0)
{
    {
        vector<CTorTest, 0> sut1;
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
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(2));
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
    EXPECT_THAT(copyAssignment, Eq(0));
    EXPECT_THAT(copyCTor, Eq(4));
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
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(2));
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
    EXPECT_THAT(moveAssignment, Eq(0));
    EXPECT_THAT(moveCTor, Eq(4));
    EXPECT_THAT(sut2.size(), Eq(4));
    EXPECT_THAT(sut2.empty(), Eq(false));

    EXPECT_THAT(sut2.at(0).value, Eq(15842));
    EXPECT_THAT(sut2.at(1).value, Eq(1584122));
    EXPECT_THAT(sut2.at(2).value, Eq(158432));
    EXPECT_THAT(sut2.at(3).value, Eq(158432));
}

TEST_F(vector_test, BeginEndIteratorAreTheSameWhenEmpty)
{
    EXPECT_THAT(sut5.begin() == sut5.end(), Eq(true));
    EXPECT_THAT(sut0.begin() == sut0.end(), Eq(true));
}

TEST_F(vector_test, BeginEndConstIteratorAreTheSameWhenEmpty)
{
    EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->begin() == const_cast<const decltype(sut5)*>(&sut5)->end(),
                Eq(true));
    EXPECT_THAT(const_cast<const decltype(sut0)*>(&sut0)->begin() == const_cast<const decltype(sut0)*>(&sut0)->end(),
                Eq(true));
}

TEST_F(vector_test, BeginIteratorComesBeforeEndIteratorWhenNotEmpty)
{
    sut5.emplace_back(1);
    EXPECT_THAT(sut5.begin() < sut5.end(), Eq(true));
}

TEST_F(vector_test, BeginConstIteratorComesBeforeEndConstIteratorWhenNotEmpty)
{
    sut5.emplace_back(1);
    EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->begin() < const_cast<const decltype(sut5)*>(&sut5)->end(),
                Eq(true));
}

TEST_F(vector_test, BeginIteratorComesBeforeEndIteratorWhenFull)
{
    for (uint64_t i = 0; i < sut5.capacity(); ++i)
        sut5.emplace_back(i);
    EXPECT_THAT(sut5.begin() < sut5.end(), Eq(true));
}

TEST_F(vector_test, BeginConstIteratorComesBeforeEndConstIteratorWhenFull)
{
    for (uint64_t i = 0; i < sut5.capacity(); ++i)
        sut5.emplace_back(i);
    EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->begin() < const_cast<const decltype(sut5)*>(&sut5)->end(),
                Eq(true));
}

TEST_F(vector_test, IteratorIteratesThroughNonEmptyVector)
{
    constexpr int INITIAL_VALUE{42};
    sut5.emplace_back(INITIAL_VALUE);
    sut5.emplace_back(INITIAL_VALUE + 1);
    sut5.emplace_back(INITIAL_VALUE + 2);
    const int EXPECTED_END_INDEX = sut5.size();

    int count = 0;
    for (auto& v : sut5)
    {
        EXPECT_THAT(v, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(EXPECTED_END_INDEX));
}

TEST_F(vector_test, ConstIteratorIteratesThroughNonEmptyVector)
{
    constexpr int INITIAL_VALUE{142};
    sut5.emplace_back(INITIAL_VALUE);
    sut5.emplace_back(INITIAL_VALUE + 1);
    sut5.emplace_back(INITIAL_VALUE + 2);
    const int EXPECTED_END_COUNT = sut5.size();

    int count = 0;
    for (auto& v : *const_cast<const decltype(sut5)*>(&sut5))
    {
        EXPECT_THAT(v, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, IteratorIteratesThroughFullVector)
{
    for (uint64_t k = 0; k < sut5.capacity(); ++k)
    {
        sut5.emplace_back(42 * k);
    }
    const int EXPECTED_END_COUNT = sut5.size();

    int i = 0;
    for (auto& v : sut5)
    {
        EXPECT_THAT(v, Eq(42 * (i++)));
    }
    EXPECT_THAT(i, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, ConstIteratorIteratesThroughFullVector)
{
    for (uint64_t k = 0; k < sut5.capacity(); ++k)
    {
        sut5.emplace_back(142 * k);
    }
    const int EXPECTED_END_COUNT = sut5.size();

    int i = 0;
    for (const auto& v : *const_cast<const decltype(sut5)*>(&sut5))
    {
        EXPECT_THAT(v, Eq(142 * (i++)));
    }
    EXPECT_THAT(i, Eq(EXPECTED_END_COUNT));
}

TEST_F(vector_test, IterateUsingData)
{
    sut5.emplace_back(127);
    sut5.emplace_back(128);
    sut5.emplace_back(129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5.data()[k], Eq(127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstData)
{
    sut5.emplace_back(3127);
    sut5.emplace_back(3128);
    sut5.emplace_back(3129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->data()[k], Eq(3127 + k));
    }
}

TEST_F(vector_test, IterateUsingAt)
{
    sut5.emplace_back(127);
    sut5.emplace_back(128);
    sut5.emplace_back(129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5.at(k), Eq(127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstAt)
{
    sut5.emplace_back(3127);
    sut5.emplace_back(3128);
    sut5.emplace_back(3129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->at(k), Eq(3127 + k));
    }
}

TEST_F(vector_test, IterateUsingSquareBracket)
{
    sut5.emplace_back(2127);
    sut5.emplace_back(2128);
    sut5.emplace_back(2129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5[k], Eq(2127 + k));
    }
}

TEST_F(vector_test, IterateUsingConstSquareBracket)
{
    sut5.emplace_back(4127);
    sut5.emplace_back(4128);
    sut5.emplace_back(4129);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT((*const_cast<const decltype(sut5)*>(&sut5))[k], Eq(4127 + k));
    }
}

TEST_F(vector_test, EraseReturnsIteratorEndWhenElementIsInvalid)
{
    auto i = sut5.begin() + 5;
    EXPECT_THAT(sut5.erase(i), Eq(sut5.end()));
    i = sut0.begin() + 1;
    EXPECT_THAT(sut0.erase(i), Eq(sut0.end()));
}

TEST_F(vector_test, EraseByIndexReturnsIteratorEndWhenElementIsInvalid)
{
    auto i = 5;
    EXPECT_THAT(sut5.erase(i), Eq(sut5.end()));

    i = 1;
    EXPECT_THAT(sut0.erase(i), Eq(sut0.end()));
}

TEST_F(vector_test, EraseOfLastElementReturnsIteratorEnd)
{
    sut5.emplace_back(3);
    auto it = sut5.erase(sut5.begin());
    EXPECT_THAT(it, Eq(sut5.end()));

    sut5.emplace_back(4);
    it = sut5.erase((uint64_t)0U);
    EXPECT_THAT(it, Eq(sut5.end()));
}

TEST_F(vector_test, TwoEmptyVectorOfZeroAreEqual)
{
    vector<int, 0> a, b;

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}


TEST_F(vector_test, EraseReturnsIteratorWhenElementIsValid)
{
    sut5.emplace_back(3);
    sut5.emplace_back(4);
    sut5.emplace_back(5);
    auto it = sut5.begin();
    EXPECT_THAT(*sut5.erase(it), Eq(4));

    uint64_t index = 0;
    EXPECT_THAT(*sut5.erase(index), Eq(5));
}

TEST_F(vector_test, ErasingElementDecreasesSize)
{
    sut5.emplace_back(3);
    sut5.emplace_back(4);
    sut5.emplace_back(5);
    sut5.erase(sut5.begin() + 2);
    sut5.erase((uint64_t)0U);
    EXPECT_THAT(sut5.size(), Eq(1));
}

TEST_F(vector_test, EraseOfLastElementCallsDTorOnly)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);
    classValue = 0; // reset, since emplace_back set it
    sut1.erase(sut1.begin() + 2);

    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(classValue, Eq(0));
}

TEST_F(vector_test, EraseOfMiddleElementCallsCorrectDTors)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);

    sut1.erase(sut1.begin() + 1);

    EXPECT_THAT(dTor, Eq(2));
    EXPECT_THAT(dTorClassValue, Eq(9));
}

TEST_F(vector_test, EraseByIndexOfLastElementCallsDTorOnly)
{
    vector<CTorTest, 5> sut1;
    sut1.emplace_back(7);
    sut1.emplace_back(8);
    sut1.emplace_back(9);

    sut1.erase((uint64_t)2);

    EXPECT_THAT(dTor, Eq(1));
    EXPECT_THAT(dTorClassValue, Eq(9));
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

    EXPECT_THAT(dTor, Eq(3));
    EXPECT_THAT(moveCTor, Eq(2));
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

    EXPECT_THAT(dTor, Eq(5));
    EXPECT_THAT(moveCTor, Eq(4));
}

TEST_F(vector_test, EraseMiddleElementDataCorrectAfterwards)
{
    sut5.emplace_back(97);
    sut5.emplace_back(101);
    sut5.emplace_back(98);
    sut5.emplace_back(99);

    sut5.erase(sut5.begin() + 1);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5[k], Eq(97 + k));
    }
}

TEST_F(vector_test, EraseFrontElementDataCorrectAfterwards)
{
    sut5.emplace_back(6101);
    sut5.emplace_back(597);
    sut5.emplace_back(598);
    sut5.emplace_back(599);

    sut5.erase(sut5.begin());

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5[k], Eq(597 + k));
    }
}

TEST_F(vector_test, EraseLastElementDataCorrectAfterwards)
{
    sut5.emplace_back(7597);
    sut5.emplace_back(7598);
    sut5.emplace_back(7599);
    sut5.emplace_back(7600);
    sut5.emplace_back(7601);
    sut5.emplace_back(76101);

    sut5.erase(sut5.begin() + 5);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5[k], Eq(7597 + k));
    }
}

TEST_F(vector_test, EraseLastElementOfFullVectorDataCorrectAfterwards)
{
    for (int i = 0; i < static_cast<int>(sut5.capacity()); ++i)
        sut5.emplace_back(i * 123);

    sut5.erase(sut5.begin() + sut5.size() - 1);

    for (uint64_t k = 0; k < sut5.size(); ++k)
    {
        EXPECT_THAT(sut5[k], Eq(k * 123));
    }
}

TEST_F(vector_test, FrontPointsToFirstElement)
{
    sut5.emplace_back(1);
    sut5.emplace_back(2);
    sut5.emplace_back(3);
    EXPECT_THAT(sut5.front(), Eq(1));
}

TEST_F(vector_test, BackPointsToLastElement)
{
    sut5.emplace_back(4);
    sut5.emplace_back(5);
    sut5.emplace_back(6);
    EXPECT_THAT(sut5.back(), Eq(6));
}

TEST_F(vector_test, ConstFrontPointsToFirstElement)
{
    sut5.emplace_back(7);
    sut5.emplace_back(8);
    sut5.emplace_back(9);
    EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->front(), Eq(7));
}

TEST_F(vector_test, ConstBackPointsToLastElement)
{
    sut5.emplace_back(10);
    sut5.emplace_back(11);
    sut5.emplace_back(12);
    EXPECT_THAT(const_cast<const decltype(sut5)*>(&sut5)->back(), Eq(12));
}

TEST_F(vector_test, ConstructorWithSizeParameterSmallerThanCapacity)
{
    vector<CTorTest, 5> sut(2);
    EXPECT_THAT(vector_test::cTor, Eq(2));
    ASSERT_THAT(sut.size(), Eq(2));
}

TEST_F(vector_test, ConstructorWithSizeParameterGreaterThanCapacity)
{
    vector<CTorTest, 5> sut(7);
    EXPECT_THAT(vector_test::cTor, Eq(5));
    ASSERT_THAT(sut.size(), Eq(5));
}

TEST_F(vector_test, TwoEmptyVectorOfSameCapacityAreEqual)
{
    vector<int, 10> a, b;

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST_F(vector_test, TwoEmptyVectorOfDifferentCapacityAreEqual)
{
    vector<int, 10> a;
    vector<int, 20> b;
    vector<int, 0> c;

    EXPECT_TRUE(a == b);
    EXPECT_TRUE(b == c);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(b != c);
}

TEST_F(vector_test, TwoEqualVectorsWithSameCapacityAreEqual)
{
    vector<int, 10> a, b;
    a.emplace_back(1);
    a.emplace_back(2);
    a.emplace_back(3);

    b.emplace_back(1);
    b.emplace_back(2);
    b.emplace_back(3);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    vector<int, 0> c, d;
    EXPECT_TRUE(c == d);
    EXPECT_FALSE(c != d);
}

TEST_F(vector_test, TwoEqualVectorsWithDifferentCapacityAreEqual)
{
    vector<int, 10> a;
    a.emplace_back(4);
    a.emplace_back(5);
    a.emplace_back(6);

    vector<int, 20> b;
    b.emplace_back(4);
    b.emplace_back(5);
    b.emplace_back(6);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST_F(vector_test, TwoVectorsWithDifferentSizeAndSameCapacityAreNotEqual)
{
    vector<int, 10> a, b;
    a.emplace_back(7);
    a.emplace_back(8);
    a.emplace_back(9);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(vector_test, TwoNonEqualVectorsWithDifferentCapacityAreNotEqual)
{
    vector<int, 10> a;
    a.emplace_back(7);
    a.emplace_back(8);
    a.emplace_back(9);

    vector<int, 20> b;
    b.emplace_back(1);
    b.emplace_back(2);
    b.emplace_back(3);

    vector<int, 0> c;

    EXPECT_FALSE(a == b);
    EXPECT_FALSE(b == c);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(b != c);
}

TEST_F(vector_test, SubsetVectorWithSameCapacityIsNotEqual)
{
    vector<int, 10> a, b;
    a.emplace_back(7);
    a.emplace_back(8);
    a.emplace_back(9);

    b.emplace_back(7);
    b.emplace_back(8);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(vector_test, SubsetVectorWithDifferentCapacityIsNotEqual)
{
    vector<int, 10> a;
    a.emplace_back(11);
    a.emplace_back(12);
    a.emplace_back(13);

    vector<int, 20> b;
    b.emplace_back(11);
    b.emplace_back(12);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(vector_test, PartiallyEqualVectorsWithSameCapacityAreNotEqual)
{
    vector<int, 10> a, b;
    a.emplace_back(14);
    a.emplace_back(15);
    a.emplace_back(16);

    b.emplace_back(14);
    b.emplace_back(15);
    b.emplace_back(666);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(vector_test, PartiallyEqualVectorsWithDifferentCapacityAreNotEqual)
{
    vector<int, 10> a;
    a.emplace_back(17);
    a.emplace_back(18);
    a.emplace_back(19);

    vector<int, 20> b;
    b.emplace_back(17);
    b.emplace_back(18);
    b.emplace_back(999);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST_F(vector_test, MemoryEfficientImplementation)
{
    vector<uint8_t, 0> sut0;
    EXPECT_THAT(sizeof(sut0), Eq(1U));
}

TEST_F(vector_test, FullVectorDestroysElementsInReverseOrder)
{
    static constexpr uint64_t VECTOR_CAPACITY = 35U;
    static constexpr uint64_t INDEX_END = VECTOR_CAPACITY - 1U;
    static constexpr uint64_t SOME_OFFSET = 9128U;

    {
        vector<CTorTest, VECTOR_CAPACITY> sut;

        for (uint64_t i = 0U; i < VECTOR_CAPACITY; ++i)
        {
            sut.emplace_back(i + SOME_OFFSET);
        }
    }

    ASSERT_THAT(dtorOrder.size(), Eq(VECTOR_CAPACITY));
    for (uint64_t i = 0U; i < VECTOR_CAPACITY; ++i)
    {
        EXPECT_THAT(dtorOrder[i], Eq(INDEX_END - i + SOME_OFFSET));
    }
}

TEST_F(vector_test, PartiallyFullVectorDestroysElementsInReverseOrder)
{
    static constexpr uint64_t VECTOR_CAPACITY = 40U;
    static constexpr uint64_t VECTOR_SIZE = 20U;
    static constexpr uint64_t INDEX_END = VECTOR_SIZE - 1U;
    static constexpr uint64_t SOME_OFFSET = 1337U;

    {
        vector<CTorTest, VECTOR_CAPACITY> sut;

        for (uint64_t i = 0U; i < VECTOR_SIZE; ++i)
        {
            sut.emplace_back(i + SOME_OFFSET);
        }
    }

    ASSERT_THAT(dtorOrder.size(), Eq(VECTOR_SIZE));
    for (uint64_t i = 0U; i < VECTOR_SIZE; ++i)
    {
        EXPECT_THAT(dtorOrder[i], Eq(INDEX_END - i + SOME_OFFSET));
    }
}

TEST_F(vector_test, PopBackReturnsFalseOnEmptyVector)
{
    EXPECT_FALSE(sut5.pop_back());
}

TEST_F(vector_test, PopBackReturnsTrueOnNonEmptyVector)
{
    sut5.emplace_back(123);
    EXPECT_TRUE(sut5.pop_back());
}

TEST_F(vector_test, PopBackReturnsTrueTillItsEmpty)
{
    static constexpr uint64_t VECTOR_SIZE = 5U;
    for (uint64_t i = 0U; i < VECTOR_SIZE; ++i)
    {
        sut5.emplace_back(i);
    }

    for (uint64_t i = 0U; i < VECTOR_SIZE; ++i)
    {
        EXPECT_TRUE(sut5.pop_back());
    }

    EXPECT_FALSE(sut5.pop_back());
}

TEST_F(vector_test, ResizeFailsWhenCountIsGreaterThanCapacity)
{
    EXPECT_FALSE(sut.resize(sut.capacity() + 1U));
}

TEST_F(vector_test, ResizeWithTemplateValueFailsWhenCountIsGreaterThanCapacity)
{
    EXPECT_FALSE(sut.resize(sut.capacity() + 1U, 12));
}

TEST_F(vector_test, SizeIncreaseWithResizeAndDefaultCTorWorks)
{
    class DefaultCTor
    {
      public:
        DefaultCTor()
            : m_a{1231}
        {
        }
        int m_a;
    };
    iox::cxx::vector<DefaultCTor, 10U> sut;

    EXPECT_TRUE(sut.resize(5U));
    ASSERT_THAT(sut.size(), Eq(5U));
    for (auto& e : sut)
    {
        EXPECT_THAT(e.m_a, Eq(1231));
    }
}

TEST_F(vector_test, SizeIncreaseWithResizeAndTemplateValueWorks)
{
    EXPECT_TRUE(sut.resize(4U, 421337));
    ASSERT_THAT(sut.size(), Eq(4U));
    for (auto& e : sut)
    {
        EXPECT_THAT(e, Eq(421337));
    }
}

TEST_F(vector_test, SizeDecreaseWithResizeAndDefaultCTorWorks)
{
    iox::cxx::vector<CTorTest, 10U> sut;
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_back(i);
    }

    EXPECT_TRUE(sut.resize(7U));
    EXPECT_THAT(dTor, Eq(3));
    ASSERT_THAT(dtorOrder.size(), Eq(3U));
    EXPECT_THAT(dtorOrder[0], Eq(9));
    EXPECT_THAT(dtorOrder[1], Eq(8));
    EXPECT_THAT(dtorOrder[2], Eq(7));
}

TEST_F(vector_test, SizeDecreaseWithResizeAndTemplateValueWorks)
{
    iox::cxx::vector<CTorTest, 10U> sut;
    for (uint64_t i = 0U; i < sut.capacity(); ++i)
    {
        sut.emplace_back(i + 10);
    }

    EXPECT_TRUE(sut.resize(7U, 66807));
    EXPECT_THAT(dTor, Eq(3));
    ASSERT_THAT(dtorOrder.size(), Eq(3U));
    EXPECT_THAT(dtorOrder[0], Eq(19));
    EXPECT_THAT(dtorOrder[1], Eq(18));
    EXPECT_THAT(dtorOrder[2], Eq(17));
}

TEST_F(vector_test, ResizeWithDefaultCTorChangesNothingIfSizeAlreadyFits)
{
    sut.emplace_back(5);
    sut.emplace_back(6);
    EXPECT_TRUE(sut.resize(2U));

    ASSERT_THAT(sut.size(), Eq(2U));
    EXPECT_THAT(sut[0], Eq(5));
    EXPECT_THAT(sut[1], Eq(6));
}

TEST_F(vector_test, ResizeWithTemplateValueChangesNothingIfSizeAlreadyFits)
{
    sut.emplace_back(7);
    sut.emplace_back(9);
    EXPECT_TRUE(sut.resize(2U, 421337));

    ASSERT_THAT(sut.size(), Eq(2U));
    EXPECT_THAT(sut[0], Eq(7));
    EXPECT_THAT(sut[1], Eq(9));
}

