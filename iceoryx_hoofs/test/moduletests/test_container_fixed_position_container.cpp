// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
// Copyright (c) 2023 by Dennis Liu <dennis48161025@gmail.com>. All rights reserved.
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

#include "iox/detail/hoofs_error_reporting.hpp"
#include "iox/fixed_position_container.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/lifetime_and_assignment_tracker.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

// should this places in other file?
template <typename T>
class MovableButNonCopyableTestClass
{
  public:
    // moveable only class requires this
    explicit MovableButNonCopyableTestClass(const T value)
        : value(value)
    {
    }

    MovableButNonCopyableTestClass(const MovableButNonCopyableTestClass& rhs) = delete;
    MovableButNonCopyableTestClass& operator=(const MovableButNonCopyableTestClass& rhs) = delete;

    MovableButNonCopyableTestClass(MovableButNonCopyableTestClass&& rhs) noexcept
        : value(rhs.value)
    {
    }

    MovableButNonCopyableTestClass& operator=(MovableButNonCopyableTestClass&& rhs) noexcept
    {
        if (this != &rhs)
        {
            value = std::move(rhs.value);
        }
        return *this;
    }

  public:
    T value;
};

struct FixedPositionContainer_test : public Test
{
    using DataType = uint64_t;
    static constexpr DataType CAPACITY{10};

    using Sut = FixedPositionContainer<DataType, CAPACITY>;

    using ComplexType = LifetimeAndAssignmentTracker<DataType, 0>;
    using NonCopyType = MovableButNonCopyableTestClass<DataType>;

    using SutComplex = FixedPositionContainer<ComplexType, CAPACITY>;
    using SutNonCopy = FixedPositionContainer<NonCopyType, CAPACITY>;

    void SetUp() override
    {
        stats.reset();
    }

    void fillSut()
    {
        for (DataType i = 0; i < CAPACITY; ++i)
        {
            const auto it = sut.emplace(i);
            ASSERT_THAT(it.to_index(), Eq(i));
        }
    }

    void fillSutComplex()
    {
        fillComplex(sut_complex);
    }

    static void fillComplex(SutComplex& s)
    {
        for (DataType i = 0; i < CAPACITY; ++i)
        {
            const auto it = s.emplace(i);
            ASSERT_THAT(it.to_index(), Eq(i));
        }
    }

    Sut sut;
    SutComplex sut_complex;
    SutNonCopy sut_noncopy;

    ComplexType::Statistics& stats = ComplexType::stats;
};

TEST_F(FixedPositionContainer_test, Capacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "17669b2f-d53b-4ac9-8190-b1c32f8ec4ba");
    EXPECT_THAT(sut.capacity(), Eq(CAPACITY));
}

// BEGIN test copy constructor

TEST_F(FixedPositionContainer_test, UsingCopyCtorEmptyContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c528ef3-9c2d-4eb2-93a9-2d998d0db380");

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(true));
    EXPECT_THAT(copy_sut_complex.size(), Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorSingleElementContainerPreservesElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "f3aaf452-77fa-4535-bf0b-37bedefc2bf6");

    constexpr DataType EXPECTED_VALUE{42U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{1U};

    sut_complex.emplace(EXPECTED_VALUE);
    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    EXPECT_THAT(copy_sut_complex.begin()->value, Eq(EXPECTED_VALUE));

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorMultipleElementsContainerPreservesAllElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "6261f53e-8089-4b9b-9b2d-9da0016a2f1e");

    constexpr SutComplex::IndexType EXPECTED_SIZE{4U};
    std::vector<DataType> EXPECTED_VALUE = {56U, 57U, 58U, 59U};
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        sut_complex.emplace(EXPECTED_VALUE[i]);
    }

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (Sut::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(copy_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
        EXPECT_THAT(copy_sut_complex.iter_from_index(i), Ne(sut_complex.iter_from_index(i)));
    }

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorFullCapacityContainerPreservesAllElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "028704df-b2f3-4133-9c16-b9d2c6a79916");

    fillSutComplex();
    constexpr SutComplex::IndexType EXPECTED_SIZE{CAPACITY};

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(true));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(CAPACITY));
    for (Sut::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(copy_sut_complex.iter_from_index(i)->value, Eq(sut_complex.iter_from_index(i)->value));
        EXPECT_THAT(copy_sut_complex.iter_from_index(i), Ne(sut_complex.iter_from_index(i)));
    }

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorFromNonEmptyWithFirstIndexErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "acd52957-1d8a-4bd8-a960-e9c040d919c2");

    std::vector<DataType> EXPECTED_VALUE = {63U, 64U, 65U, 66U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{3U};

    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_EQ(copy_sut_complex.size(), EXPECTED_SIZE);
    EXPECT_EQ(copy_sut_complex.begin()->value, 64U);
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorFromNonEmptyWithFirstAndMiddleAndLastErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "f21ff6d9-f50a-499f-912a-923bd4273b07");

    std::vector<DataType> EXPECTED_VALUE = {1U, 2U, 3U, 5U, 6U, 7U, 8U};

    fillSutComplex();
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST);
    sut_complex.erase(SutComplex::Index::LAST / 2);

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(CAPACITY - 3U));

    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyCtorWillNotChangeSourceContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d60e04b-341f-4da1-8f3e-2736529d7843");

    std::vector<DataType> EXPECTED_VALUE = {63U, 64U, 65U, 66U};
    constexpr uint64_t EXPECTED_SIZE{4U};
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }

    SutComplex copy_sut_complex{sut_complex};

    EXPECT_THAT(sut_complex.size(), Eq(EXPECTED_SIZE));

    auto it = sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

// END test copy constructor


// BEGIN test move constructor

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromEmptyContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "af8958fb-9a09-4987-b290-ce41abdc2354");

    SutComplex move_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(true));
    EXPECT_THAT(move_sut_complex.size(), Eq(0));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromSingleElementToEmptyContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "df6c1884-43c6-4d1e-b889-6cbf4b9ee726");

    constexpr DataType EXPECTED_VALUE{42U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{1U};
    sut_complex.emplace(EXPECTED_VALUE);

    SutComplex move_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    EXPECT_THAT(move_sut_complex.begin()->value, Eq(EXPECTED_VALUE));

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    // Note: It is sufficient to do it only once for "sut_complex"
    EXPECT_THAT(sut_complex.full(), Eq(false));
    EXPECT_THAT(sut_complex.empty(), Eq(true));
    EXPECT_THAT(sut_complex.size(), Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromMultipleElementsContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9d929ae-23c8-4b5b-ba82-e5af12cdace4");

    std::vector<DataType> EXPECTED_VALUE{56U, 57U, 58U, 59U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{4U};
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        sut_complex.emplace(EXPECTED_VALUE[i]);
    }

    SutComplex move_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(move_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
    }

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromFullCapacityContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a9ca6d1-5ac3-4e31-9cb9-0476176531e1");

    fillSutComplex();
    constexpr SutComplex::IndexType EXPECTED_SIZE{CAPACITY};
    std::vector<DataType> EXPECTED_VALUE;
    for (const auto& item : sut_complex)
    {
        EXPECTED_VALUE.emplace_back(item.value);
    }

    SutComplex move_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(move_sut_complex.full(), Eq(true));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(move_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
    }

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromNonEmptyWithFirstIndexErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "de0eaa3c-bf30-4899-95ec-6c23bbd53a24");

    std::vector<DataType> EXPECTED_VALUE = {63U, 64U, 65U, 66U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{3U};

    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex move_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    EXPECT_THAT(move_sut_complex.begin()->value, Eq(64U));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorFromNonEmptyWithFirstAndMiddleAndLastErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "073e3bc6-1e33-46b8-860b-c35d1f599d11");

    std::vector<DataType> EXPECTED_VALUE = {1U, 2U, 3U, 5U, 6U, 7U, 8U};

    fillSutComplex();
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST);
    sut_complex.erase(SutComplex::Index::LAST / 2);

    SutComplex copy_sut_complex{std::move(sut_complex)};

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(CAPACITY - 3U));

    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveCtorAtNonCopyableTypeShouldCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "e1cc7c9f-c1b5-4047-811b-004302af5c00");

    constexpr uint64_t EXPECTED_SIZE{2U};
    sut_noncopy.emplace(7U);
    sut_noncopy.emplace(8U);

    SutNonCopy move_sut_noncopy{std::move(sut_noncopy)};

    EXPECT_THAT(move_sut_noncopy.size(), Eq(EXPECTED_SIZE));
}

// END test move constructor


// BEGIN test copy assignment

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromEmptyContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "013338e3-4330-49b4-8aa4-9b66517bb3bc");

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(true));
    EXPECT_THAT(copy_sut_complex.size(), Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromSingleElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6cf9e9d1-91a9-4403-a25a-52b64dd523be");

    constexpr DataType EXPECTED_VALUE{42U};
    constexpr uint64_t EXPECTED_SIZE{1U};
    sut_complex.emplace(EXPECTED_VALUE);

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(sut_complex.size()));
    EXPECT_THAT(copy_sut_complex.begin()->value, Eq(EXPECTED_VALUE));

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromMultipleElementsContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "262ad71a-0ee2-4661-b2c8-a3cca9c1cf5e");

    std::vector<DataType> EXPECTED_VALUE{56U, 57U, 58U, 59U};
    constexpr uint64_t EXPECTED_SIZE{4U};
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        sut_complex.emplace(EXPECTED_VALUE[i]);
    }

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(sut_complex.size()));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(copy_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
        EXPECT_THAT(copy_sut_complex.iter_from_index(i), Ne(sut_complex.iter_from_index(i)));
    }

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromFullCapacityContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "b46d0be7-5977-467e-adc4-2e9adc554fdd");

    fillSutComplex();
    constexpr SutComplex::IndexType EXPECTED_SIZE{CAPACITY};
    std::vector<DataType> EXPECTED_VALUE;
    for (const auto& item : sut_complex)
    {
        EXPECTED_VALUE.emplace_back(item.value);
    }

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(true));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(copy_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
        EXPECT_THAT(copy_sut_complex.iter_from_index(i), Ne(sut_complex.iter_from_index(i)));
    }

    EXPECT_THAT(stats.copyCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveCTor, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromEmptyToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d906d20-aacc-4536-86e7-bd4aafcdc2f7");

    std::vector<DataType> EXPECTED_VALUE = {12U, 13U, 14U, 15U, 16U};
    SutComplex copy_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(0));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(true));
    EXPECT_THAT(copy_sut_complex.size(), Eq(0));
    EXPECT_THAT(copy_sut_complex.begin(), Eq(copy_sut_complex.end()));
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromLargerSizeToSmallerSizeContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "cce5bca5-7bfd-4909-bd60-acfffbb1611e");

    std::vector<DataType> EXPECTED_VALUE = {21U, 22U, 23U, 24U, 25U};
    std::vector<DataType> DUMMY_VALUE = {94U, 95U, 96U};
    constexpr uint64_t EXPECTED_SIZE{5U};

    SutComplex copy_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentBetweenContainersOfEqalSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd1b1c4b-20a4-464a-b036-8ce4764f3ac5");

    std::vector<DataType> EXPECTED_VALUE = {29U, 28U, 27U, 26U};
    std::vector<DataType> DUMMY_VALUE = {37U, 38U, 39U, 40U};
    constexpr uint64_t EXPECTED_SIZE{4U};

    SutComplex copy_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromSmallerSizeToLargerSizeContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "412b4439-66dd-4e5c-93f7-511e1e965b78");

    std::vector<DataType> EXPECTED_VALUE = {1U, 2U, 3U, 4U};
    std::vector<DataType> DUMMY_VALUE = {31U, 32U, 33U, 34U, 35U, 36U, 37U};
    constexpr uint64_t EXPECTED_SIZE{4U};

    SutComplex copy_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyWithFirstIndexErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "929e7bae-f276-4ae5-b559-7bb518198e63");

    std::vector<DataType> DUMMY_VALUE = {12U, 32U, 23U, 14U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE.begin() + 1, DUMMY_VALUE.end());
    constexpr uint64_t EXPECTED_SIZE{3U};

    for (const auto& value : DUMMY_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyWithFirstIndexErasedToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd882c6e-1e46-495c-b2cf-24056c144d85");

    std::vector<DataType> DUMMY_VALUE_SRC = {65U, 66U, 23U, 7U, 12U};
    std::vector<DataType> DUMMY_VALUE_DEST = {1U, 3U, 5U, 16U, 18U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{4U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyToNonEmptyContainerWithBothFirstIndexErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad3d96da-e64a-4252-950c-a36f5333e42a");

    std::vector<DataType> DUMMY_VALUE_SRC = {98U, 99U, 100U, 101U, 102U, 103U};
    std::vector<DataType> DUMMY_VALUE_DEST = {12U, 33U, 544U, 162U, 182U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{5U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }
    copy_sut_complex.erase(SutComplex::Index::FIRST);

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test,
       UsingCopyAssignmentFromNonEmptyWithFirstErasedToNonEmptyContainerWithFirstAndSecondErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3ac8e6d-795e-4e41-bad3-aba39483d6d5");

    std::vector<DataType> DUMMY_VALUE_SRC = {56U, 54U, 55U, 33U, 12U, 34U};
    std::vector<DataType> DUMMY_VALUE_DEST = {18U, 22U, 42U, 323U, 216U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{5U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }
    copy_sut_complex.erase(SutComplex::Index::FIRST);
    copy_sut_complex.erase(SutComplex::Index::FIRST + 1);

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "0c6138ce-861e-42d8-b7f2-ecd4ac01537e");

    std::vector<DataType> DUMMY_VALUE = {17U, 26U, 32U, 357U, 30U, 21U, 18U, 100U, 67U, 79U};
    std::vector<DataType> EXPECTED_VALUE = {26U, 32U, 357U, 21U, 18U, 100U, 67U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4127ad54-f272-4f61-9737-e41b92d7cf60");

    std::vector<DataType> DUMMY_VALUE_SRC = {111U, 112U, 113U, 114U, 115U, 116U, 117U, 118U, 119U, 120U};
    std::vector<DataType> DUMMY_VALUE_DEST = {189U, 112U, 124U, 1735U, 10U, 11U, 14U, 164U, 123U, 12U};
    std::vector<DataType> EXPECTED_VALUE = {112U, 113U, 114U, 116U, 117U, 118U, 119U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test,
       UsingCopyAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainerWithFirstIndexErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "4aea0c73-98c7-45b1-81e0-713c18ea16de");

    std::vector<DataType> DUMMY_VALUE_SRC = {101U, 102U, 103U, 104U, 105U, 106U, 107U, 108U, 109U, 110U};
    std::vector<DataType> DUMMY_VALUE_DEST = {89U, 12U, 24U, 735U, 0U, 1U, 4U, 64U, 23U, 2U};
    std::vector<DataType> EXPECTED_VALUE = {102U, 103U, 104U, 106U, 107U, 108U, 109U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }
    copy_sut_complex.erase(SutComplex::Index::FIRST);

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(
    FixedPositionContainer_test,
    UsingCopyAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainerWithNeighboringFirstAndOneBeforeMiddleAndOneBeforeLastErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e086470-8b0e-4c82-8c5d-7a9c45312729");

    std::vector<DataType> DUMMY_VALUE_SRC = {121U, 122U, 123U, 124U, 125U, 126U, 127U, 128U, 129U, 130U};
    std::vector<DataType> DUMMY_VALUE_DEST = {79U, 2U, 14U, 725U, 40U, 15U, 34U, 54U, 13U, 32U};
    std::vector<DataType> EXPECTED_VALUE = {122U, 123U, 124U, 126U, 127U, 128U, 129U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }
    copy_sut_complex.erase(SutComplex::Index::FIRST + 1);
    copy_sut_complex.erase(SutComplex::Index::LAST / 2 + 1);
    copy_sut_complex.erase(SutComplex::Index::LAST - 1);

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentFromNonEmptyWithLastErasedToFullContainerWithFirstErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "82e562f9-89fe-4998-870f-c575da5a3f79");

    std::vector<DataType> DUMMY_VALUE_SRC = {131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U};
    std::vector<DataType> DUMMY_VALUE_DEST = {23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 32U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin(), DUMMY_VALUE_SRC.end() - 1);
    constexpr uint64_t EXPECTED_SIZE{9U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }
    copy_sut_complex.erase(SutComplex::Index::FIRST);

    copy_sut_complex = sut_complex;

    EXPECT_THAT(copy_sut_complex.full(), Eq(false));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentWillNotChangeSourceContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "22191ca0-2350-4901-b6f3-1786621f6a17");

    std::vector<DataType> EXPECTED_VALUE = {63U, 64U, 65U, 66U};
    constexpr uint64_t EXPECTED_SIZE{4U};
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }

    SutComplex copy_sut_complex;
    copy_sut_complex = sut_complex;

    EXPECT_THAT(sut_complex.size(), Eq(EXPECTED_SIZE));

    auto it = sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }
}

TEST_F(FixedPositionContainer_test, UsingCopyAssignmentInsertionShouldFailWhenCapacityReached)
{
    ::testing::Test::RecordProperty("TEST_ID", "fcbe01f1-b3d4-4794-b291-efeeddd4db7f");

    std::vector<DataType> DUMMY_VALUE_SRC = {131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U};
    std::vector<DataType> DUMMY_VALUE_DEST = {23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 32U};
    std::vector<DataType> EXPECTED_VALUE = {132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 77U, 88U};
    constexpr uint64_t EXPECTED_SIZE{CAPACITY};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex copy_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        copy_sut_complex.emplace(value);
    }

    copy_sut_complex = sut_complex;

    copy_sut_complex.emplace(77U);
    copy_sut_complex.emplace(88U);

    EXPECT_THAT(copy_sut_complex.full(), Eq(true));
    EXPECT_THAT(copy_sut_complex.empty(), Eq(false));
    EXPECT_THAT(copy_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = copy_sut_complex.begin();
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    // should failed
    auto failed_it = copy_sut_complex.emplace(1000000U);
    EXPECT_THAT(failed_it, Eq(copy_sut_complex.end()));
}

// END test copy assignment


// BEGIN test move assignment

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromEmptyContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "711ced12-4b93-47d1-af37-cace03fac2c1");

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(true));
    EXPECT_THAT(move_sut_complex.size(), Eq(0));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromSingleElementContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3902afc-5eba-4e10-8412-f09b7b5d17b8");

    constexpr DataType EXPECTED_VALUE{42U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{1U};
    sut_complex.emplace(EXPECTED_VALUE);

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    EXPECT_THAT(move_sut_complex.begin()->value, Eq(EXPECTED_VALUE));

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    EXPECT_THAT(stats.copyAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromMultipleElementsContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "c44da583-1ed8-4c83-b5bb-dba5d64b21d9");

    std::vector<DataType> EXPECTED_VALUE{56U, 57U, 58U, 59U};
    constexpr SutComplex::IndexType EXPECTED_SIZE{4U};
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        sut_complex.emplace(EXPECTED_VALUE[i]);
    }

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(move_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
    }

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromFullCapacityContainerClearsOriginal)
{
    ::testing::Test::RecordProperty("TEST_ID", "3196b101-f03a-4029-abb8-77106f0b45d8");

    fillSutComplex();
    constexpr SutComplex::IndexType EXPECTED_SIZE{CAPACITY};
    std::vector<DataType> EXPECTED_VALUE;
    for (const auto& item : sut_complex)
    {
        EXPECTED_VALUE.emplace_back(item.value);
    }

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(true));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    for (SutComplex::IndexType i = 0; i < EXPECTED_SIZE; ++i)
    {
        EXPECT_THAT(move_sut_complex.iter_from_index(i)->value, Eq(EXPECTED_VALUE[i]));
    }

    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(EXPECTED_SIZE));
    EXPECT_THAT(stats.moveAssignment, Eq(0));
    // make sure clear() been called
    EXPECT_THAT(stats.dTor, Eq(EXPECTED_SIZE));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromEmptyToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "998e06c0-6879-451e-a493-e3e26944feff");

    std::vector<DataType> EXPECTED_VALUE = {12U, 13U, 14U, 15U, 16U};
    SutComplex move_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(0));
    EXPECT_THAT(move_sut_complex.empty(), Eq(true));
    EXPECT_THAT(move_sut_complex.size(), Eq(0));

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromLargerSizeToSmallerSizeContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4b5b538-740f-4543-b493-5ea87e0ea8cc");

    std::vector<DataType> EXPECTED_VALUE = {21U, 22U, 23U, 24U, 25U};
    std::vector<DataType> DUMMY_VALUE = {94U, 95U, 96U};
    constexpr uint64_t EXPECTED_SIZE{5U};

    SutComplex move_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentBetweenContainersOfEqalSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "50030f15-aefc-4086-aca0-02c1d5e032a1");

    std::vector<DataType> EXPECTED_VALUE = {29U, 28U, 27U, 26U};
    std::vector<DataType> DUMMY_VALUE = {37U, 38U, 39U, 40U};
    constexpr uint64_t EXPECTED_SIZE{4U};

    SutComplex move_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromSmallerSizeToLargerSizeContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6759de7f-5555-4251-89a2-dbcc3c2f2efb");

    std::vector<DataType> EXPECTED_VALUE = {1U, 2U, 3U, 4U};
    std::vector<DataType> DUMMY_VALUE = {31U, 32U, 33U, 34U, 35U, 36U, 37U};
    constexpr uint64_t EXPECTED_SIZE{4U};

    SutComplex move_sut_complex;
    for (const auto& value : EXPECTED_VALUE)
    {
        sut_complex.emplace(value);
    }
    for (const auto& value : DUMMY_VALUE)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyWithFirstIndexErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "95c1839b-0755-458c-908b-89b59a914fb5");

    std::vector<DataType> DUMMY_VALUE = {12U, 32U, 23U, 14U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE.begin() + 1, DUMMY_VALUE.end());
    constexpr uint64_t EXPECTED_SIZE{3U};

    for (const auto& value : DUMMY_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyWithFirstIndexErasedToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "db50dd57-6e56-4343-981b-debb4780d403");

    std::vector<DataType> DUMMY_VALUE_SRC = {65U, 66U, 23U, 7U, 12U};
    std::vector<DataType> DUMMY_VALUE_DEST = {1U, 3U, 5U, 16U, 18U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{4U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyToNonEmptyContainerWithBothFirstIndexErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "2655d41e-06d9-4e85-a356-a0ba256b35ee");

    std::vector<DataType> DUMMY_VALUE_SRC = {98U, 99U, 100U, 101U, 102U, 103U};
    std::vector<DataType> DUMMY_VALUE_DEST = {12U, 33U, 544U, 162U, 182U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{5U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }
    move_sut_complex.erase(SutComplex::Index::FIRST);

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test,
       UsingMoveAssignmentFromNonEmptyWithFirstErasedToNonEmptyContainerWithFirstAndSecondErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "8aa4a221-ed52-49e9-91c3-81d45d70edc5");

    std::vector<DataType> DUMMY_VALUE_SRC = {56U, 54U, 55U, 33U, 12U, 34U};
    std::vector<DataType> DUMMY_VALUE_DEST = {18U, 22U, 42U, 323U, 216U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin() + 1, DUMMY_VALUE_SRC.end());
    constexpr uint64_t EXPECTED_SIZE{5U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }
    move_sut_complex.erase(SutComplex::Index::FIRST);
    move_sut_complex.erase(SutComplex::Index::FIRST + 1);

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "646d08e5-d26a-4efe-96e1-fa79ef1549b7");

    std::vector<DataType> DUMMY_VALUE = {17U, 26U, 32U, 357U, 30U, 21U, 18U, 100U, 67U, 79U};
    std::vector<DataType> EXPECTED_VALUE = {26U, 32U, 357U, 21U, 18U, 100U, 67U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "62367ed3-a97a-4dae-82f3-e7bacd432b9b");

    std::vector<DataType> DUMMY_VALUE_SRC = {111U, 112U, 113U, 114U, 115U, 116U, 117U, 118U, 119U, 120U};
    std::vector<DataType> DUMMY_VALUE_DEST = {189U, 112U, 124U, 1735U, 10U, 11U, 14U, 164U, 123U, 12U};
    std::vector<DataType> EXPECTED_VALUE = {112U, 113U, 114U, 116U, 117U, 118U, 119U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test,
       UsingMoveAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainerWithFirstIndexErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b57f4cc-7a79-4a0a-a1f2-0a2f1e943fdf");

    std::vector<DataType> DUMMY_VALUE_SRC = {101U, 102U, 103U, 104U, 105U, 106U, 107U, 108U, 109U, 110U};
    std::vector<DataType> DUMMY_VALUE_DEST = {89U, 12U, 24U, 735U, 0U, 1U, 4U, 64U, 23U, 2U};
    std::vector<DataType> EXPECTED_VALUE = {102U, 103U, 104U, 106U, 107U, 108U, 109U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }
    move_sut_complex.erase(SutComplex::Index::FIRST);

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(
    FixedPositionContainer_test,
    UsingMoveAssignmentFromNonEmptyWithFirstAndMiddleAndLastErasedToNonEmptyContainerWithNeighboringFirstAndOneBeforeMiddleAndOneBeforeLastErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "74cf9827-99ea-45a8-884d-e8efff9b1290");

    std::vector<DataType> DUMMY_VALUE_SRC = {121U, 122U, 123U, 124U, 125U, 126U, 127U, 128U, 129U, 130U};
    std::vector<DataType> DUMMY_VALUE_DEST = {79U, 2U, 14U, 725U, 40U, 15U, 34U, 54U, 13U, 32U};
    std::vector<DataType> EXPECTED_VALUE = {122U, 123U, 124U, 126U, 127U, 128U, 129U};
    constexpr uint64_t EXPECTED_SIZE{7U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST / 2);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }
    move_sut_complex.erase(SutComplex::Index::FIRST + 1);
    move_sut_complex.erase(SutComplex::Index::LAST / 2 + 1);
    move_sut_complex.erase(SutComplex::Index::LAST - 1);

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentFromNonEmptyWithLastErasedToFullContainerWithFirstErased)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca8b489c-c24d-478e-8720-f265687209ea");

    std::vector<DataType> DUMMY_VALUE_SRC = {131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U};
    std::vector<DataType> DUMMY_VALUE_DEST = {23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 32U};
    std::vector<DataType> EXPECTED_VALUE(DUMMY_VALUE_SRC.begin(), DUMMY_VALUE_SRC.end() - 1);
    constexpr uint64_t EXPECTED_SIZE{9U};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }
    move_sut_complex.erase(SutComplex::Index::FIRST);

    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(move_sut_complex.full(), Eq(false));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentInsertionShouldFailWhenCapacityReached)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad438f8a-2b9e-45d3-8d89-feefbccf3f03");

    std::vector<DataType> DUMMY_VALUE_SRC = {131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U};
    std::vector<DataType> DUMMY_VALUE_DEST = {23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 32U};
    std::vector<DataType> EXPECTED_VALUE = {132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 77U, 88U};
    constexpr uint64_t EXPECTED_SIZE{CAPACITY};

    for (const auto& value : DUMMY_VALUE_SRC)
    {
        sut_complex.emplace(value);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    sut_complex.erase(SutComplex::Index::LAST);

    SutComplex move_sut_complex;
    for (const auto& value : DUMMY_VALUE_DEST)
    {
        move_sut_complex.emplace(value);
    }

    move_sut_complex = std::move(sut_complex);

    move_sut_complex.emplace(77U);
    move_sut_complex.emplace(88U);

    EXPECT_THAT(move_sut_complex.full(), Eq(true));
    EXPECT_THAT(move_sut_complex.empty(), Eq(false));
    EXPECT_THAT(move_sut_complex.size(), Eq(EXPECTED_SIZE));
    auto it = move_sut_complex.iter_from_index(SutComplex::Index::FIRST);
    for (const auto& value : EXPECTED_VALUE)
    {
        EXPECT_THAT(it->value, Eq(value));
        ++it;
    }

    EXPECT_THAT(sut_complex.empty(), Eq(true));

    // should failed
    auto failed_it = move_sut_complex.emplace(1000000U);
    EXPECT_THAT(failed_it, Eq(move_sut_complex.end()));
}

TEST_F(FixedPositionContainer_test, UsingMoveAssignmentAtNonCopyableTypeShouldCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4876d02-d855-4bcc-af39-3d2dc388c40d");

    constexpr uint64_t EXPECTED_SIZE{2U};
    sut_noncopy.emplace(7U);
    sut_noncopy.emplace(8U);

    SutNonCopy move_sut_noncopy;
    move_sut_noncopy = std::move(sut_noncopy);

    EXPECT_THAT(move_sut_noncopy.size(), Eq(EXPECTED_SIZE));
}

TEST_F(FixedPositionContainer_test, IteratorsAfterMoveWorkAsExpected)
{
    ::testing::Test::RecordProperty("TEST_ID", "17b91183-9f1e-4ab4-ab27-e34f096674d8");

    std::vector<DataType> EXPECTED_VALUE = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};

    fillSutComplex();

    SutComplex move_sut_complex;
    move_sut_complex = std::move(sut_complex);

    EXPECT_THAT(sut_complex.begin(), Eq(sut_complex.end()));
    auto it = move_sut_complex.begin();
    for (SutComplex::IndexType i = 0U; it != move_sut_complex.end(); ++it, ++i)
    {
        EXPECT_THAT(it->value, Eq(EXPECTED_VALUE[i]));
    }
}

// END test move assignment


// BEGIN test empty

TEST_F(FixedPositionContainer_test, NewlyCreatedContainerIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1fb0f86-8c48-4be5-aec6-8d269cdb258c");
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, AddingOneElementResultsInNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4d7d451b-a8e2-460c-b2c2-3b3ee58acfdb");

    sut.emplace(0U);
    EXPECT_THAT(sut.empty(), Eq(false));
}

TEST_F(FixedPositionContainer_test, AddingOneElementAndErasingAgainResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "97568d5d-89c0-40a6-8cfa-e12b40ca5115");

    auto it = sut.emplace(0U);
    sut.erase(it);
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, FillingUpResultsInNonEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d1ddef4-6578-4f3d-adf3-7e739f8f062e");

    for (DataType i = 0; i < CAPACITY; ++i)
    {
        sut.emplace(i);
        EXPECT_THAT(sut.empty(), Eq(false));
    }
}

TEST_F(FixedPositionContainer_test, FillingUpAndErasingAgainResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec7a25aa-5c28-4ad1-87e0-4c7888915833");

    fillSut();

    for (Sut::IndexType i = 0; i < CAPACITY; ++i)
    {
        EXPECT_THAT(sut.empty(), Eq(false));
        sut.erase(i);
    }
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(FixedPositionContainer_test, FillingUpAndErasingAgainInReverseOrderResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2373a83-d2b9-4305-a6cc-581338163686");

    fillSut();

    for (Sut::IndexType i = 0; i < CAPACITY; ++i)
    {
        EXPECT_THAT(sut.empty(), Eq(false));
        sut.erase(static_cast<Sut::IndexType>(Sut::Index::LAST - i));
    }
    EXPECT_THAT(sut.empty(), Eq(true));
}

// END test empty


// BEGIN test full

TEST_F(FixedPositionContainer_test, NewlyCreatedContainerIsNotFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "69f3e641-3356-4c52-ae3e-fcca4811e329");
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(FixedPositionContainer_test, AddingOneElementResultsInNonFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "9752cfe6-e734-465c-8754-cdf8f6fdf13f");

    sut.emplace(0U);
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(FixedPositionContainer_test, FillingUpFinallyResultsInFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b8ab137-d69b-48a5-a34b-ba721383c186");

    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        EXPECT_THAT(sut.full(), Eq(false));
        sut.emplace(i);
    }
    EXPECT_THAT(sut.full(), Eq(true));
}

TEST_F(FixedPositionContainer_test, FillingUpAndRemovingLastResultsInNonFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "5506f2cf-7de1-4b38-a91e-114dfdd5c55d");

    fillSut();

    sut.erase(Sut::Index::LAST);
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(FixedPositionContainer_test, FillingUpAndRemovingFirstResultsInNonFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "05b52974-f536-454e-a66a-0c95f46b9361");

    fillSut();

    sut.erase(Sut::Index::FIRST);
    EXPECT_THAT(sut.full(), Eq(false));
}

TEST_F(FixedPositionContainer_test, FillingUpAndRemovingMiddleResultsInNonFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "eaa7b1e9-73de-48a9-848d-b11fa62ee3f3");

    fillSut();

    sut.erase(Sut::Index::LAST / 2U);
    EXPECT_THAT(sut.full(), Eq(false));
}

// END test full


// BEGIN test size

TEST_F(FixedPositionContainer_test, NewlyCreatedContainerHasSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "b603f39c-54b9-4312-a3a8-d64590830a7d");
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(FixedPositionContainer_test, AddingOneElementResultsInSizeOfOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "07d884ab-c831-4d31-9d26-b852f528db48");

    sut.emplace(0U);
    EXPECT_THAT(sut.size(), Eq(1));
}

TEST_F(FixedPositionContainer_test, FillingUpFinallyResultsSizeOfCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "418271b7-e96e-4a9e-bb65-30b26f9005ba");

    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        EXPECT_THAT(sut.size(), Eq(i));
        sut.emplace(i);
    }
    EXPECT_THAT(sut.size(), Eq(CAPACITY));
}

TEST_F(FixedPositionContainer_test, FillingUpAndRemovinOneElementResultsInReducedSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c655b05-dbf2-4657-bf30-ac0b07870da3");

    fillSut();

    sut.erase(Sut::Index::LAST / 2U);
    EXPECT_THAT(sut.size(), Eq(CAPACITY - 1U));
}

// END test size


// BEGIN test emplace

TEST_F(FixedPositionContainer_test, EmplaceOnEmptyContainerReturnsIteratorToTheAddedElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "70f31dcc-5395-4c3f-b75f-a44ecd8e385f");

    constexpr DataType EXPECTED_VALUE{13};
    auto it = sut.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test,
       EmplaceWithComplexTypeOnEmptyConatainerReturnsIteratorToTheAddedElementAndCallsCorrectConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "e62ad545-1f34-4edf-97ea-2a2aa5f2d15f");

    constexpr DataType EXPECTED_VALUE{3113};
    auto it = sut_complex.emplace(EXPECTED_VALUE);

    EXPECT_THAT(stats.cTor, Eq(0));
    EXPECT_THAT(stats.customCTor, Eq(1));
    EXPECT_THAT(stats.dTor, Eq(0));
    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(0));

    // this needs to be at the end to prevent this code changing 'stats'
    ASSERT_THAT(it, Ne(sut_complex.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
    constexpr SutComplex::IndexType EXPECTED_INITIAL_INDEX{SutComplex::Index::FIRST};
    EXPECT_THAT(it.to_index(), Eq(EXPECTED_INITIAL_INDEX));
}

TEST_F(FixedPositionContainer_test, EmplaceOnFullContainerReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5a41bdd-e42d-4d4c-bdef-53ff1ae4e2a4");

    constexpr DataType EXPECTED_VALUE{42};
    fillSut();

    auto it = sut.emplace(EXPECTED_VALUE);

    EXPECT_THAT(it, Eq(sut.end()));
}

TEST_F(FixedPositionContainer_test, EmplaceWithComplexTypeOnFullContainerReturnsEndIteratorAndDoesNotCallAnyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "af225467-f44b-4866-9714-a5508a226810");

    constexpr DataType EXPECTED_VALUE{42};
    fillSutComplex();
    stats.reset();

    auto it = sut_complex.emplace(EXPECTED_VALUE);

    EXPECT_THAT(stats.cTor, Eq(0));
    EXPECT_THAT(stats.customCTor, Eq(0));
    EXPECT_THAT(stats.dTor, Eq(0));
    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(0));

    EXPECT_THAT(it, Eq(sut_complex.end()));
}

TEST_F(FixedPositionContainer_test, EmplaceWithPartiallyFilledUpContainerWorksWhenFirstSlotIsFree)
{
    ::testing::Test::RecordProperty("TEST_ID", "38f1635e-e8f1-47af-8887-63511df58673");

    fillSut();

    const std::vector<Sut::IndexType> erased{1, 5};

    for (const auto& i : erased)
    {
        sut.erase(i);
    }
    constexpr Sut::IndexType INDEX_TO_ERASE_FOR_INSERTION{Sut::Index::FIRST};
    sut.erase(INDEX_TO_ERASE_FOR_INSERTION);

    constexpr DataType EXPECTED_VALUE{0};
    auto it = sut.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
    EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE_FOR_INSERTION));
}

TEST_F(FixedPositionContainer_test,
       EmplaceWithComplexTypeWithPartiallyFilledUpContainerWorksWhenFirstSlotIsFreeAndCallsCorrectConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "a515c3a4-1dbf-47c5-aa2c-ed74615922e9");

    fillSutComplex();

    const std::vector<SutComplex::IndexType> erased{1, 5};

    for (const auto& i : erased)
    {
        sut_complex.erase(i);
    }
    sut_complex.erase(SutComplex::Index::FIRST);
    stats.reset();

    constexpr DataType EXPECTED_VALUE{0};
    auto it = sut_complex.emplace(EXPECTED_VALUE);

    EXPECT_THAT(stats.cTor, Eq(0));
    EXPECT_THAT(stats.customCTor, Eq(1));
    EXPECT_THAT(stats.dTor, Eq(0));
    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(0));

    // this needs to be at the end to prevent this code changing 'stats'
    ASSERT_THAT(it, Ne(sut_complex.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));

    auto iter = sut_complex.begin();
    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        if (std::find(erased.begin(), erased.end(), i) == erased.end())
        {
            ASSERT_THAT(iter, Ne(sut_complex.end()));
            EXPECT_THAT(*iter, Eq(i));
            ++iter;
        }
    }
    EXPECT_THAT(iter, Eq(sut_complex.end()));
}

TEST_F(FixedPositionContainer_test, EmplaceWithPartiallyFilledUpContainerWorksWhenNotTheFirstSlotIsFree)
{
    ::testing::Test::RecordProperty("TEST_ID", "3caed564-010e-447d-bdb3-899fde04da88");

    fillSut();

    const std::vector<Sut::IndexType> erased{2, 5};

    for (const auto& i : erased)
    {
        sut.erase(i);
    }
    sut.erase(1);

    constexpr DataType EXPECTED_VALUE{1};
    auto it = sut.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test,
       EmplaceWithComplexTypeWithPartiallyFilledUpContainerWorksWhenNotTheFirstSlotIsFreeAndCallsCorrectConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9034d36-1197-49b9-b1f4-71e6678ad541");

    fillSutComplex();

    const std::vector<SutComplex::IndexType> erased{2, 5};

    for (const auto& i : erased)
    {
        sut_complex.erase(i);
    }
    sut_complex.erase(1);
    stats.reset();

    constexpr DataType EXPECTED_VALUE{1};
    auto it = sut_complex.emplace(EXPECTED_VALUE);

    EXPECT_THAT(stats.cTor, Eq(0));
    EXPECT_THAT(stats.customCTor, Eq(1));
    EXPECT_THAT(stats.dTor, Eq(0));
    EXPECT_THAT(stats.copyCTor, Eq(0));
    EXPECT_THAT(stats.moveCTor, Eq(0));

    // this needs to be at the end to prevent this code changing 'stats'
    ASSERT_THAT(it, Ne(sut_complex.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));

    auto iter = sut_complex.begin();
    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        if (std::find(erased.begin(), erased.end(), i) == erased.end())
        {
            ASSERT_THAT(iter, Ne(sut_complex.end()));
            EXPECT_THAT(*iter, Eq(i));
            ++iter;
        }
    }
    EXPECT_THAT(iter, Eq(sut_complex.end()));
}

// END test emplace


// BEGIN test insert

// NOTE: there is only two test for 'insert' since it simply forwards to 'emplace'; if this changes, more tests need to
// be written; a note at the implementation of 'insert' ensures that this will not be forgotten

TEST_F(FixedPositionContainer_test, InsertReturnsIteratorToTheAddedElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "275ea2ee-bba9-40e5-a961-c9d3cc73792f");

    constexpr DataType EXPECTED_VALUE{1331};
    auto it = sut.insert(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, InsertWithComplexTypeReturnsIteratorToTheAddedElementAndCallsCopyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "e537f31b-1a79-4d99-b6ba-e798a3f884eb");

    constexpr DataType EXPECTED_VALUE{1313};
    const ComplexType value{EXPECTED_VALUE};
    stats.reset();

    auto it = sut_complex.insert(value);

    EXPECT_THAT(stats.cTor, Eq(0));
    EXPECT_THAT(stats.customCTor, Eq(0));
    EXPECT_THAT(stats.dTor, Eq(0));
    EXPECT_THAT(stats.copyCTor, Eq(1));
    EXPECT_THAT(stats.moveCTor, Eq(0));

    // this needs to be at the end to prevent this code changing 'stats'
    ASSERT_THAT(it, Ne(sut_complex.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
}

// END test insert


// BEGIN test erase

TEST_F(FixedPositionContainer_test, EraseOnContainerWithOneElementReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd5229c7-b7d0-4de0-89aa-9b58135249a3");

    constexpr DataType EXPECTED_VALUE{73};
    const auto it_emplaced = sut_complex.emplace(EXPECTED_VALUE);
    stats.reset();

    auto it = sut_complex.erase(it_emplaced.to_index());

    EXPECT_THAT(it, Eq(sut_complex.end()));

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, EraseOnLastElementOnFullContainerReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "390504e4-fcae-46b2-8ad9-25d95cf3fed2");

    fillSutComplex();
    stats.reset();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{SutComplex::Index::LAST};
    auto it = sut_complex.erase(INDEX_TO_ERASE);

    EXPECT_THAT(it, Eq(sut_complex.end()));

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
}

TEST_F(FixedPositionContainer_test, EraseOnLastElementOnNonFullContainerReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eee74ce-2490-432a-9967-b24ad6f04121");

    fillSutComplex();
    sut_complex.erase(SutComplex::Index::LAST);
    stats.reset();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{SutComplex::Index::LAST - 1U};
    auto it = sut_complex.erase(INDEX_TO_ERASE);

    EXPECT_THAT(it, Eq(sut_complex.end()));

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
}

TEST_F(FixedPositionContainer_test, EraseOnFirstElementOnFullContainerReturnsIteratorToNextElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "da8379dd-8d71-4a30-a698-3df1be6bfb80");

    fillSutComplex();
    stats.reset();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{SutComplex::Index::FIRST};
    auto it = sut_complex.erase(INDEX_TO_ERASE);

    EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE + 1));

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
}

TEST_F(FixedPositionContainer_test, EraseOnArbitraryNonFirstOrLastElementReturnsIteratorToNextElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "22ab7321-94f3-4060-836d-f4400a63dabd");

    fillSutComplex();
    stats.reset();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{SutComplex::Index::LAST / 2U};
    auto it = sut_complex.erase(INDEX_TO_ERASE);

    EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE + 1U));

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
}

TEST_F(FixedPositionContainer_test, EraseDoesNotCorruptTheContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "3116beab-53f7-41bd-9cfe-ff186bd8303d");

    fillSut();

    const std::vector<Sut::IndexType> erased{Sut::Index::FIRST, Sut::Index::LAST / 2, Sut::Index::LAST};

    for (const auto& i : erased)
    {
        sut.erase(i);
    }

    auto it = sut.begin();
    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        if (std::find(erased.begin(), erased.end(), i) == erased.end())
        {
            ASSERT_THAT(it, Ne(sut.end()));
            EXPECT_THAT(*it, Eq(i));
            ++it;
        }
    }
    EXPECT_THAT(it, Eq(sut.end()));
}

TEST_F(FixedPositionContainer_test, EraseWithPointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "99f6b756-9f24-412f-865d-24d5b5032a22");

    const std::vector<SutComplex::IndexType> erase{
        SutComplex::Index::FIRST, SutComplex::Index::LAST / 2U, SutComplex::Index::LAST};
    for (const auto INDEX_TO_ERASE : erase)
    {
        SCOPED_TRACE(std::string("Erase at index: ") + std::to_string(INDEX_TO_ERASE));

        sut_complex.clear();
        fillSutComplex();

        const auto it_erase = sut_complex.iter_from_index(INDEX_TO_ERASE);
        stats.reset();

        const auto it = sut_complex.erase(it_erase.to_ptr());

        EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE + 1U));

        EXPECT_THAT(stats.dTor, Eq(1));
        EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
    }
}

TEST_F(FixedPositionContainer_test, EraseWithIteratorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a31f6a44-1183-475b-8eab-ff8c7b2158c1");

    const std::vector<SutComplex::IndexType> erase{
        SutComplex::Index::FIRST, SutComplex::Index::LAST / 2U, SutComplex::Index::LAST};
    for (const auto INDEX_TO_ERASE : erase)
    {
        SCOPED_TRACE(std::string("Erase at index: ") + std::to_string(INDEX_TO_ERASE));

        sut_complex.clear();
        fillSutComplex();

        const SutComplex::Iterator it_erase = sut_complex.iter_from_index(INDEX_TO_ERASE);
        stats.reset();

        const auto it = sut_complex.erase(it_erase);

        EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE + 1U));

        EXPECT_THAT(stats.dTor, Eq(1));
        EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
    }
}

TEST_F(FixedPositionContainer_test, EraseWithConstIteratorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "14b268f0-7e57-4719-b7af-19434f7ce994");

    const std::vector<SutComplex::IndexType> erase{
        SutComplex::Index::FIRST, SutComplex::Index::LAST / 2U, SutComplex::Index::LAST};
    for (const auto INDEX_TO_ERASE : erase)
    {
        SCOPED_TRACE(std::string("Erase at index: ") + std::to_string(INDEX_TO_ERASE));

        sut_complex.clear();
        fillSutComplex();

        const SutComplex::ConstIterator it_erase = sut_complex.iter_from_index(INDEX_TO_ERASE);
        stats.reset();

        const auto it = sut_complex.erase(it_erase);

        EXPECT_THAT(it.to_index(), Eq(INDEX_TO_ERASE + 1U));

        EXPECT_THAT(stats.dTor, Eq(1));
        EXPECT_THAT(stats.classValue, Eq(INDEX_TO_ERASE));
    }
}

TEST_F(FixedPositionContainer_test, EraseOnEmptyContainerCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "943c9f2d-0ebd-4593-a721-884c952fef0d");

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(Sut::Index::FIRST); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseOnEmptySlotCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f03708a-4d26-4005-8e95-e710f14d1269");

    fillSut();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{Sut::Index::LAST / 2U};
    sut.erase(INDEX_TO_ERASE);

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(INDEX_TO_ERASE); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithOutOfBoundsIndexCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2d16f4a-c806-41c1-8051-b6eb0906994b");

    fillSut();

    constexpr SutComplex::IndexType INDEX_TO_ERASE{Sut::Index::LAST + 1U};

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(INDEX_TO_ERASE); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithNullptrCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ad256c9-c87c-45f0-9c08-4206ae20a5ee");

    fillSut();

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithPointerPointingOutOfContainerCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bc4791c-deda-452b-91fa-3d52468c4d3e");

    fillSut();

    auto* ptr_first = sut.begin().to_ptr();

    // NOLINTJUSTIFICATION required for test
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(ptr_first - 1U); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(ptr_first + CAPACITY); }, iox::er::ENFORCE_VIOLATION);
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

TEST_F(FixedPositionContainer_test, EraseWithUnalignedPointerCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "865a04c1-b5be-4436-8e7f-537d5861ac63");

    fillSut();

    auto* ptr_first = sut.begin().to_ptr();
    // NOLINTJUSTIFICATION required for test
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    auto* ptr_unaligned = reinterpret_cast<DataType*>(reinterpret_cast<uintptr_t>(ptr_first) + 1U);

    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(ptr_unaligned); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithEndIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "61e3f1cb-2e27-423a-8744-6b326b633e94");

    fillSut();

    Sut::Iterator it = sut.end();
    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(it); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithIteratorNotOriginatingFromContainerCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "cebb7500-acc7-4726-812a-5d9a3b9239b5");

    fillSut();

    Sut sut2;
    sut2.emplace(666U);

    Sut::Iterator it = sut2.begin();
    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(it); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithEndConstIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "504d8a43-f0b0-4ab2-aa93-9fb4d3ec42d8");

    fillSut();

    Sut::ConstIterator it = sut.end();
    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(it); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, EraseWithConstIteratorNotOriginatingFromContainerCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa5cb040-d2bb-4a19-ace2-4e04b8dcca41");

    fillSut();

    Sut sut2;
    sut2.emplace(666U);

    Sut::ConstIterator it = sut2.begin();
    IOX_EXPECT_FATAL_FAILURE([&] { sut.erase(it); }, iox::er::ENFORCE_VIOLATION);
}

// END test erase


// BEGIN test dTor

TEST_F(FixedPositionContainer_test, ContainerWithoutElementsDoesNotCallsDestructorOnElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "1baeb9cf-49c0-41a6-97cc-49f15696d213");

    {
        SutComplex s;
    }

    EXPECT_THAT(stats.dTor, Eq(0));
}

TEST_F(FixedPositionContainer_test, ContainerWithOneElementsCallsDestructorOnElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "99785123-36ab-4213-9093-09100345a49e");

    constexpr DataType EXPECTED_VALUE{37};

    {
        SutComplex s;
        s.emplace(EXPECTED_VALUE);
    }

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, FilledUpContainerCallsDestructorOnAllEelements)
{
    ::testing::Test::RecordProperty("TEST_ID", "08119caf-ec5a-4a3e-a200-8bba3ab1112e");

    {
        SutComplex s;
        fillComplex(s);
    }

    EXPECT_THAT(stats.dTor, Eq(CAPACITY));
    ASSERT_THAT(stats.dTorOrder.size(), Eq(CAPACITY));

    DataType expected_value{0};
    for (const auto value : stats.dTorOrder)
    {
        EXPECT_THAT(value, Eq(expected_value));
        ++expected_value;
    }
}

TEST_F(FixedPositionContainer_test, PartiallyFilledUpContainerCallsDestructorOnExistingEelements)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0c6511b-e4bc-477c-bd19-05962b518e69");

    const std::vector<SutComplex::IndexType> erased{
        SutComplex::Index::FIRST, SutComplex::Index::LAST / 2U, SutComplex::Index::LAST};

    {
        SutComplex s;
        fillComplex(s);

        for (const auto& i : erased)
        {
            s.erase(i);
        }
        stats.reset();
    }

    EXPECT_THAT(stats.dTor, Eq(CAPACITY - erased.size()));
    EXPECT_THAT(stats.dTorOrder.size(), Eq(CAPACITY - erased.size()));

    std::vector<DataType> expected_values;
    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        if (std::find(erased.begin(), erased.end(), i) == erased.end())
        {
            expected_values.emplace_back(i);
        }
    }

    size_t i = 0;
    for (const auto value : stats.dTorOrder)
    {
        EXPECT_THAT(value, Eq(expected_values[i]));
        ++i;
    }
}

// END test dTor


// BEGIN test clear

TEST_F(FixedPositionContainer_test, ClearOnNewlyCreatedContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d214a69-541a-42fb-8842-2d187cb9199a");

    sut.clear();

    EXPECT_THAT(sut.empty(), Eq(true));
    EXPECT_THAT(sut.full(), Eq(false));
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(FixedPositionContainer_test, ClearAfterAddingOneElementResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "8280433a-3ed7-4128-a624-88474a907412");

    sut.emplace(42U);

    sut.clear();

    EXPECT_THAT(sut.empty(), Eq(true));
    EXPECT_THAT(sut.full(), Eq(false));
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(FixedPositionContainer_test, ClearAfterFillingUpResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "a7c5bdf1-3bd2-4fb6-986a-adc58068586a");

    fillSut();

    sut.clear();

    EXPECT_THAT(sut.empty(), Eq(true));
    EXPECT_THAT(sut.full(), Eq(false));
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(FixedPositionContainer_test, ClearOnPartiallyFillUpContainerResultsInEmptyContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "de927932-f774-45fb-9095-44942d5db894");

    fillSut();

    const std::vector<Sut::IndexType> erased{Sut::Index::FIRST, Sut::Index::LAST / 2U, Sut::Index::LAST};

    for (const auto& i : erased)
    {
        sut.erase(i);
    }

    sut.clear();

    EXPECT_THAT(sut.empty(), Eq(true));
    EXPECT_THAT(sut.full(), Eq(false));
    EXPECT_THAT(sut.size(), Eq(0U));
}

TEST_F(FixedPositionContainer_test, ClearAfterAddingOneElementCallsDestructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "fd776cc1-2c69-460d-b874-038908d066e6");

    constexpr DataType EXPECTED_VALUE{73};
    sut_complex.emplace(EXPECTED_VALUE);

    sut_complex.clear();

    EXPECT_THAT(stats.dTor, Eq(1));
    EXPECT_THAT(stats.classValue, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, ClearAfterFillingUpCallsDestructorOnAllElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "3339d266-901a-4d08-8058-7a980ec9540c");

    fillSutComplex();

    sut_complex.clear();

    EXPECT_THAT(stats.dTor, Eq(CAPACITY));
    EXPECT_THAT(stats.dTorOrder.size(), Eq(CAPACITY));

    DataType expected_value{0};
    for (const auto value : stats.dTorOrder)
    {
        EXPECT_THAT(value, Eq(expected_value));
        ++expected_value;
    }
}

TEST_F(FixedPositionContainer_test, ClearAfterPartiallyFillingContainerUpCallsDestructorOnAllElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c84ef512-4ed1-41ce-9a82-3832495513e0");

    fillSutComplex();

    const std::vector<SutComplex::IndexType> erased{
        SutComplex::Index::FIRST, SutComplex::Index::LAST / 2U, SutComplex::Index::LAST};

    for (const auto& i : erased)
    {
        sut_complex.erase(i);
    }
    stats.reset();

    sut_complex.clear();

    EXPECT_THAT(stats.dTor, Eq(CAPACITY - erased.size()));
    EXPECT_THAT(stats.dTorOrder.size(), Eq(CAPACITY - erased.size()));

    std::vector<DataType> expected_values;
    for (uint64_t i = 0; i < CAPACITY; ++i)
    {
        if (std::find(erased.begin(), erased.end(), i) == erased.end())
        {
            expected_values.emplace_back(i);
        }
    }

    size_t i = 0;
    for (const auto value : stats.dTorOrder)
    {
        EXPECT_THAT(value, Eq(expected_values[i]));
        ++i;
    }
}

// END test clear


// BEGIN test iter_from_index

TEST_F(FixedPositionContainer_test, IterFromIndexWithIndexPointingToEmptySlotReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "9de27168-53e4-4da7-aec5-3fac1b3783a5");

    EXPECT_THAT(sut.iter_from_index(Sut::Index::LAST / 2U), Eq(sut.end()));
}

TEST_F(FixedPositionContainer_test, IterFromIndexWithOutOfBoundsIndexReturnsEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec6984df-b93e-48eb-9205-c8c921b6629a");

    EXPECT_THAT(sut.iter_from_index(Sut::Index::LAST + 1U), Eq(sut.end()));
}

TEST_F(FixedPositionContainer_test, IterFromIndexWithValidIndexReturnsIteratorTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "24f3eb09-36db-4515-a0f7-1322e2277042");

    fillSut();

    EXPECT_THAT(sut.iter_from_index(Sut::Index::LAST / 2U).to_index(), Eq(Sut::Index::LAST / 2U));
}

// END test iter_from_index


// BEGIN test iterator

TEST_F(FixedPositionContainer_test, NewlyCreatedContainerHasEndIteratorPointingToEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "2705fdcd-fdcb-41de-8d4f-4a2d708ea019");

    auto it_from_end = sut.end();
    auto const_it_from_end = static_cast<const Sut&>(sut).end();
    auto const_it_from_cend = sut.cend();

    EXPECT_TRUE((std::is_same<Sut::Iterator, decltype(it_from_end)>::value));
    EXPECT_TRUE((std::is_same<Sut::ConstIterator, decltype(const_it_from_end)>::value));
    EXPECT_TRUE((std::is_same<Sut::ConstIterator, decltype(const_it_from_cend)>::value));

    EXPECT_THAT(it_from_end.to_index(), Eq(CAPACITY));
    EXPECT_THAT(const_it_from_end.to_index(), Eq(CAPACITY));
    EXPECT_THAT(const_it_from_cend.to_index(), Eq(CAPACITY));
}

TEST_F(FixedPositionContainer_test, NewlyCreatedContainerHasBeginIteratorPointingToEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "c147fffc-a373-484a-a802-e89fae60bfd9");

    auto it_from_begin = sut.begin();
    auto const_it_from_begin = static_cast<const Sut&>(sut).begin();
    auto const_it_from_cbegin = sut.cbegin();

    EXPECT_TRUE((std::is_same<Sut::Iterator, decltype(it_from_begin)>::value));
    EXPECT_TRUE((std::is_same<Sut::ConstIterator, decltype(const_it_from_begin)>::value));
    EXPECT_TRUE((std::is_same<Sut::ConstIterator, decltype(const_it_from_cbegin)>::value));

    EXPECT_THAT(it_from_begin.to_index(), Eq(CAPACITY));
    EXPECT_THAT(const_it_from_begin.to_index(), Eq(CAPACITY));
    EXPECT_THAT(const_it_from_cbegin.to_index(), Eq(CAPACITY));
}

TEST_F(FixedPositionContainer_test, BeginIteratorPointsToBeginOfContainerAfterInsertingTheFirstElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "10c6b680-4ba1-4927-8544-506cb73a460b");

    constexpr DataType EXPECTED_VALUE{42};
    constexpr Sut::IndexType EXPECTED_INDEX{Sut::Index::FIRST};

    sut.emplace(EXPECTED_VALUE);

    auto it_from_begin = sut.begin();
    auto const_it_from_begin = static_cast<const Sut&>(sut).begin();
    auto const_it_from_cbegin = sut.cbegin();

    EXPECT_THAT(it_from_begin.to_index(), Eq(EXPECTED_INDEX));
    EXPECT_THAT(const_it_from_begin.to_index(), Eq(EXPECTED_INDEX));
    EXPECT_THAT(const_it_from_cbegin.to_index(), Eq(EXPECTED_INDEX));

    EXPECT_THAT(*it_from_begin, Eq(EXPECTED_VALUE));
    EXPECT_THAT(*const_it_from_begin, Eq(EXPECTED_VALUE));
    EXPECT_THAT(*const_it_from_cbegin, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, BeginIteratorPointsToFirstUsedSlotWhenSlotAtPositionZeroIsFree)
{
    ::testing::Test::RecordProperty("TEST_ID", "91a22130-d166-4919-a9a3-50b32d5ee7be");

    constexpr DataType DUMMY_VALUE{0};
    constexpr DataType EXPECTED_VALUE{13};
    constexpr Sut::IndexType DUMMY_INDEX{0};
    constexpr Sut::IndexType EXPECTED_INDEX{1};

    sut.emplace(DUMMY_VALUE);
    sut.emplace(EXPECTED_VALUE);
    sut.erase(DUMMY_INDEX);

    auto it_from_begin = sut.begin();
    auto const_it_from_begin = static_cast<const Sut&>(sut).begin();
    auto const_it_from_cbegin = sut.cbegin();

    EXPECT_THAT(it_from_begin.to_index(), Eq(EXPECTED_INDEX));
    EXPECT_THAT(const_it_from_begin.to_index(), Eq(EXPECTED_INDEX));
    EXPECT_THAT(const_it_from_cbegin.to_index(), Eq(EXPECTED_INDEX));

    EXPECT_THAT(*it_from_begin, Eq(EXPECTED_VALUE));
    EXPECT_THAT(*const_it_from_begin, Eq(EXPECTED_VALUE));
    EXPECT_THAT(*const_it_from_cbegin, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, IteratorToConstIteratorViaConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4440ac7-d802-4cbc-912a-2829c86f0140");
    constexpr DataType EXPECTED_VALUE{13};

    sut.emplace(0U);
    auto it = sut.emplace(EXPECTED_VALUE);
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));

    Sut::ConstIterator cit{it};
    EXPECT_THAT(*cit, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, IteratorToConstIteratorViaAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9543dde5-bcb0-4aad-afeb-86a9c0d289e2");
    constexpr DataType EXPECTED_VALUE{37};

    sut.emplace(0U);
    auto it = sut.emplace(EXPECTED_VALUE);
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));

    Sut::ConstIterator cit{sut.end()};
    cit = it;
    EXPECT_THAT(*cit, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, IteratorPreIncrementOnEndIteratorLeadsToEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e0fb1c7-744d-4d90-9524-56220ccc72bd");

    auto it = sut.end();
    auto cit = sut.cend();

    ++it;
    ++cit;

    EXPECT_THAT(it, Eq(sut.end()));
    EXPECT_THAT(cit, Eq(sut.cend()));
}

TEST_F(FixedPositionContainer_test, IteratorPreIncrementLeadsToEndIteratorWhenContainerHasOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7e8a6f1-72ce-4a84-93d7-611be1e05445");

    sut.emplace(123U);

    auto it = sut.begin();
    auto cit = sut.cbegin();

    ++it;
    ++cit;

    EXPECT_THAT(it, Eq(sut.end()));
    EXPECT_THAT(cit, Eq(sut.cend()));
}

TEST_F(FixedPositionContainer_test, IteratorPreIncrementLeadsToIteratorForNextElementWhenContainerHasRemainingElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "e00b9667-25a8-453d-8819-8e39bce8b62c");

    sut.emplace(456U);
    sut.emplace(769U);

    auto it = sut.begin();
    auto cit = sut.cbegin();

    ++it;
    ++cit;

    EXPECT_THAT(it.to_index(), Eq(Sut::Index::FIRST + 1U));
    EXPECT_THAT(cit.to_index(), Eq(Sut::Index::FIRST + 1U));
}

TEST_F(FixedPositionContainer_test, IteratorPreIncrementAccessesAllElementsInFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "aefe17f5-d764-45f9-92af-c65bb6512ef6");

    fillSut();

    Sut::IndexType expected_index{Sut::Index::FIRST};
    auto it = sut.begin();
    do
    {
        EXPECT_THAT(it.to_index(), Eq(expected_index));
        ASSERT_THAT(expected_index, Le(Sut::Index::LAST));
        ++expected_index;
    } while (++it != sut.end());
    --expected_index;
    EXPECT_THAT(expected_index, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, IteratorPreIncrementAccessesAllElementsInPartiallyFilledUpContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6dd67a93-636e-4f63-b2e2-b34777e16b56");

    const std::vector<Sut::IndexType> erased{Sut::Index::FIRST, Sut::Index::LAST / 2U, Sut::Index::LAST};
    fillSut();
    for (const auto& i : erased)
    {
        sut.erase(i);
    }

    Sut::IndexType expected_index{Sut::Index::FIRST + 1U};
    auto it = sut.begin();
    do
    {
        EXPECT_THAT(it.to_index(), Eq(expected_index));
        ASSERT_THAT(expected_index, Le(Sut::Index::LAST));
        ++expected_index;
        if (std::find(erased.begin(), erased.end(), expected_index) != erased.end())
        {
            ++expected_index;
        }
    } while (++it != sut.end());
    --expected_index;
    EXPECT_THAT(expected_index, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, IteratorPostIncrementOnEndIteratorLeadsToEndIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba4dd951-fe2b-4ba1-bf88-4a2ef832c15f");

    auto it = sut.end();
    auto cit = sut.cend();

    auto old = it++;
    auto cold = cit++;

    EXPECT_THAT(it, Eq(sut.end()));
    EXPECT_THAT(cit, Eq(sut.cend()));

    EXPECT_THAT(old, Eq(sut.end()));
    EXPECT_THAT(cold, Eq(sut.cend()));
}

TEST_F(FixedPositionContainer_test, IteratorPostIncrementLeadsToEndIteratorWhenContainerHasOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "b214c524-f149-451b-b8ee-f1804f9f4884");

    sut.emplace(123U);

    auto it = sut.begin();
    auto cit = sut.cbegin();
    const auto old_expected = it;

    auto old = it++;
    auto cold = cit++;

    EXPECT_THAT(it, Eq(sut.end()));
    EXPECT_THAT(cit, Eq(sut.cend()));

    EXPECT_THAT(old, Eq(old_expected));
    EXPECT_THAT(cold, Eq(old_expected));
}

TEST_F(FixedPositionContainer_test, IteratorPostIncrementLeadsToIteratorForNextElementWhenContainerHasRemainingElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8a7be35-b61e-48cd-8375-b5654de688ab");

    sut.emplace(456U);
    sut.emplace(769U);

    auto it = sut.begin();
    auto cit = sut.cbegin();
    const auto old_expected = it;

    auto old = it++;
    auto cold = cit++;

    EXPECT_THAT(it.to_index(), Eq(Sut::Index::FIRST + 1U));
    EXPECT_THAT(cit.to_index(), Eq(Sut::Index::FIRST + 1U));

    EXPECT_THAT(old, Eq(old_expected));
    EXPECT_THAT(cold, Eq(old_expected));
}

TEST_F(FixedPositionContainer_test, IteratorPostIncrementAccessesAllElementsInFullContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "d298293b-924f-4aea-b408-2d463f4f9a5e");

    fillSut();

    Sut::IndexType expected_index{Sut::Index::FIRST};
    auto it = sut.begin();
    auto old = it;
    do
    {
        old = it++;
        EXPECT_THAT(old.to_index(), Eq(expected_index));
        ASSERT_THAT(expected_index, Le(Sut::Index::LAST));
        ++expected_index;
        EXPECT_THAT(it.to_index(), Eq(expected_index));
    } while (it != sut.end());
    --expected_index;
    EXPECT_THAT(expected_index, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, IteratorPostIncrementAccessesAllElementsInPartiallyFilledUpContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8ec4dc8-6f9a-4856-871e-f43385045cb3");

    const std::vector<Sut::IndexType> erased{Sut::Index::FIRST, Sut::Index::LAST / 2U, Sut::Index::LAST};
    fillSut();
    for (const auto& i : erased)
    {
        sut.erase(i);
    }

    Sut::IndexType expected_index{Sut::Index::FIRST + 1U};
    auto it = sut.begin();
    auto old = it;
    do
    {
        old = it++;
        EXPECT_THAT(old.to_index(), Eq(expected_index));
        ASSERT_THAT(expected_index, Le(Sut::Index::LAST));
        ++expected_index;
        if (std::find(erased.begin(), erased.end(), expected_index) != erased.end())
        {
            ++expected_index;
        }
        EXPECT_THAT(it.to_index(), Eq(expected_index));
    } while (it != sut.end());
    --expected_index;
    EXPECT_THAT(expected_index, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, DereferencingNonConstItertorLeadsToNonConstReference)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7b43292-94d1-44b2-8496-26d2abcf38f4");

    auto is_non_const = std::is_same<decltype(std::declval<Sut::Iterator>().operator*()), DataType&>::value;
    EXPECT_TRUE(is_non_const);
}

TEST_F(FixedPositionContainer_test, DereferencingConstIteratorLeadsToConstReference)
{
    ::testing::Test::RecordProperty("TEST_ID", "43330807-9ca8-4b82-a518-b7ab59dbf373");

    auto is_const = std::is_same<decltype(std::declval<Sut::ConstIterator>().operator*()), const DataType&>::value;
    EXPECT_TRUE(is_const);
}

TEST_F(FixedPositionContainer_test, DereferencingIteratorAccessesUnderlyingValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd595860-74f8-4b54-890f-e20f4396d696");

    constexpr DataType EXPECTED_VALUE{1111};
    auto it = sut.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it, Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, DereferencingIteratorOnFullContainerAccessesAllUnderlyingValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4442b60-2265-480f-ac5c-07a7a4116fd6");

    fillSut();

    DataType expected_value{Sut::Index::FIRST};
    auto it = sut.begin();
    EXPECT_THAT(*it, Eq(expected_value));
    while (++it != sut.end())
    {
        ++expected_value;
        EXPECT_THAT(*it, Eq(expected_value));
        ASSERT_THAT(expected_value, Le(Sut::Index::LAST));
    }
    EXPECT_THAT(expected_value, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, DereferencingEndIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2ccf248-97f8-4265-9bb4-9c8e7cb79e67");

    IOX_EXPECT_FATAL_FAILURE([&] { auto _ [[maybe_unused]] = *sut.end(); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { auto _ [[maybe_unused]] = *sut.cend(); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, DereferencingInvalidIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "72c83dc7-ddc2-4c84-a64d-df9630ebc54b");

    auto it = sut.emplace(135U);
    sut.erase(it);

    IOX_EXPECT_FATAL_FAILURE([&] { auto _ [[maybe_unused]] = *it; }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnNonConstItertorLeadsToNonConstPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ee50ed6-7c5a-494c-9832-26d8e3e62bfe");

    auto is_non_const = std::is_same<decltype(std::declval<SutComplex::Iterator>().operator->()), ComplexType*>::value;
    EXPECT_TRUE(is_non_const);
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnConstItertorLeadsToConstReference)
{
    ::testing::Test::RecordProperty("TEST_ID", "e09991fe-0358-40ad-8dad-af6f0940ec6d");

    auto is_const =
        std::is_same<decltype(std::declval<SutComplex::ConstIterator>().operator->()), const ComplexType*>::value;
    EXPECT_TRUE(is_const);
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnIteratorAccessesUnderlyingValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "9de3d4ff-30e9-43a3-b54e-3ed318c96654");

    constexpr DataType EXPECTED_VALUE{2222};
    auto it = sut_complex.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut_complex.end()));
    EXPECT_THAT(it->ref(), Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnIteratorOnFullContainerAccessesAllUnderlyingValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "30060b3f-6952-4f5e-89ff-b28d4cc35a39");

    fillSutComplex();

    DataType expected_value{SutComplex::Index::FIRST};
    auto it = sut_complex.begin();
    EXPECT_THAT(it->ref(), Eq(expected_value));
    while (++it != sut_complex.end())
    {
        ++expected_value;
        EXPECT_THAT(it->ref(), Eq(expected_value));
        ASSERT_THAT(expected_value, Le(SutComplex::Index::LAST));
    }
    EXPECT_THAT(expected_value, Eq(SutComplex::Index::LAST));
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnEndIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "74e20989-69dd-451c-9d6d-f65044a7d7b6");

    IOX_EXPECT_FATAL_FAILURE([&] { sut_complex.end()->ref(); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { sut_complex.cend()->ref(); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, ArrowOperatorOnInvalidIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "282b090b-f66b-41a4-9a38-dbade26cb998");

    auto it = sut_complex.emplace(135U);
    sut_complex.erase(it);

    IOX_EXPECT_FATAL_FAILURE([&] { it->ref(); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, ToPtrOnNonConstItertorLeadsToNonConstPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "a388be7f-759e-4946-85ea-7a47f06c553d");

    auto is_non_const = std::is_same<decltype(std::declval<Sut::Iterator>().to_ptr()), DataType*>::value;
    EXPECT_TRUE(is_non_const);
}

TEST_F(FixedPositionContainer_test, ToPtrOnConstIteratorLeadsToConstPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba7621dc-7e03-4185-a83a-f6d9e33df2e2");

    auto is_const = std::is_same<decltype(std::declval<Sut::ConstIterator>().to_ptr()), const DataType*>::value;
    EXPECT_TRUE(is_const);
}

TEST_F(FixedPositionContainer_test, ToPtrOnIteratorAccessesUnderlyingValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "516dcc3c-fede-4894-9864-f06b0330828b");

    constexpr DataType EXPECTED_VALUE{1111};
    auto it = sut.emplace(EXPECTED_VALUE);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(*it.to_ptr(), Eq(EXPECTED_VALUE));
}

TEST_F(FixedPositionContainer_test, ToPtrOnIteratorOnFullContainerAccessesAllUnderlyingValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff640b88-e136-41dc-91cc-97c550fbafb1");

    fillSut();

    DataType expected_value{Sut::Index::FIRST};
    auto it = sut.begin();
    EXPECT_THAT(*it.to_ptr(), Eq(expected_value));
    while (++it != sut.end())
    {
        ++expected_value;
        EXPECT_THAT(*it.to_ptr(), Eq(expected_value));
        ASSERT_THAT(expected_value, Le(Sut::Index::LAST));
    }
    EXPECT_THAT(expected_value, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, ToPtrOnEndIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "51b76d04-6c8c-486e-88c9-8b6b760c41d4");

    IOX_EXPECT_FATAL_FAILURE([&] { auto* _ [[maybe_unused]] = sut.end().to_ptr(); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { const auto* _ [[maybe_unused]] = sut.cend().to_ptr(); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, ToPtrOnInvalidIteratorCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "38df6619-65f3-4eee-aa4a-8c35aba13c1b");

    auto it = sut.emplace(135U);
    sut.erase(it);

    IOX_EXPECT_FATAL_FAILURE([&] { auto* _ [[maybe_unused]] = it.to_ptr(); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(FixedPositionContainer_test, ToIndexOnIteratorReturnsCorrespondingIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7d820cd-56f2-4165-a85c-c400f03f0e06");

    auto it = sut.emplace(159U);

    ASSERT_THAT(it, Ne(sut.end()));
    EXPECT_THAT(it.to_index(), Eq(Sut::Index::FIRST));
}

TEST_F(FixedPositionContainer_test, ToIndexOnIteratorOnFullContainerReturnsAllCorrespondingIndices)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ba5a0cb-66fe-4ece-8ffa-d55532d9be1c");

    fillSut();

    Sut::IndexType expected_index{Sut::Index::FIRST};
    auto it = sut.begin();
    EXPECT_THAT(it.to_index(), Eq(expected_index));
    while (++it != sut.end())
    {
        ++expected_index;
        EXPECT_THAT(it.to_index(), Eq(expected_index));
        ASSERT_THAT(expected_index, Le(Sut::Index::LAST));
    }
    EXPECT_THAT(expected_index, Eq(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, ToIndexOnEndIteratorReturnsIndexBeyondLast)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fe6b23f-aae8-41d4-b5ad-ce8385709061");

    EXPECT_THAT(sut.end().to_index(), Gt(Sut::Index::LAST));
    EXPECT_THAT(sut.cend().to_index(), Gt(Sut::Index::LAST));
}

TEST_F(FixedPositionContainer_test, ToIndexOnInvalidIteratorReturnsStoredIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "3027523b-85fb-49eb-b0ef-b3d6f3cad5a7");

    sut.emplace(531U);
    auto it = sut.emplace(369U);
    sut.erase(it);

    EXPECT_THAT(it.to_index(), Eq(Sut::Index::FIRST + 1U));
}

TEST_F(FixedPositionContainer_test, OriginsFromReturnsTrueWhenIteratorOriginsFromContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "3d462756-8cc8-42be-af02-83361ec527e0");

    auto it = sut.emplace(121U);

    EXPECT_TRUE(it.origins_from(sut));
}

TEST_F(FixedPositionContainer_test, OriginsFromReturnsFalseWhenIteratorDoesNotOriginFromContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb400b16-b705-4512-8701-5cf62cf82534");

    auto it = sut.emplace(213U);
    Sut sut2;

    EXPECT_FALSE(it.origins_from(sut2));
}

TEST_F(FixedPositionContainer_test, CompareForEqualityReturnsTrueWhenIteratorPointToTheSameElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "846b2153-ebcd-4810-b86a-56ba36b141e6");

    auto it1 = sut.emplace(987U);
    auto it2 = sut.begin();

    EXPECT_TRUE(it1 == it2);
}

TEST_F(FixedPositionContainer_test, CompareForEqualityReturnsFalseWhenIteratorDoesNotPointToTheSameElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "2801a787-bda2-4a6d-8cea-88ca7a59c075");

    auto it1 = sut.emplace(963U);
    auto it2 = sut.emplace(963U);

    EXPECT_FALSE(it1 == it2);
}

TEST_F(FixedPositionContainer_test, CompareForEqualityReturnsFalseWhenIteratorDoesNotOriginFromTheSameContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f4547de-5acc-458f-9cfc-95e472fa3732");

    auto it1 = sut.emplace(842U);
    Sut sut2;
    auto it2 = sut2.emplace(842U);

    EXPECT_FALSE(it1 == it2);
}

TEST_F(FixedPositionContainer_test, CompareForNonEqualityReturnsFalseWhenIteratorPointToTheSameElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "4301a29c-b90c-4dba-9431-1d668e2de2f5");

    auto it1 = sut.emplace(987U);
    auto it2 = sut.begin();

    EXPECT_FALSE(it1 != it2);
}

TEST_F(FixedPositionContainer_test, CompareForNonEqualityReturnsTrueWhenIteratorDoesNotPointToTheSameElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "a76db7ad-0d4e-4d15-92c8-9a5b53d431e9");

    auto it1 = sut.emplace(963U);
    auto it2 = sut.emplace(963U);

    EXPECT_TRUE(it1 != it2);
}

TEST_F(FixedPositionContainer_test, CompareForNonEqualityReturnsTrueWhenIteratorDoesNotOriginFromTheSameContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "03d3bc8e-03da-4b1c-ac25-8606e132f7ac");

    auto it1 = sut.emplace(842U);
    Sut sut2;
    auto it2 = sut2.emplace(842U);

    EXPECT_TRUE(it1 != it2);
}

TEST_F(FixedPositionContainer_test, IteratorDestructorDoesNotDestroyObjectItPointsTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "a52e9d6e-c763-4953-95c2-1f05d605d180");

    fillSutComplex();

    {
        auto it [[maybe_unused]] = sut_complex.begin();
        auto cit [[maybe_unused]] = sut_complex.cbegin();
    }

    EXPECT_THAT(stats.dTor, Eq(0));
}

// END test iterator

} // namespace
