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

#include "iox/stack.hpp"
#include "test.hpp"

#include <vector>

namespace
{
using namespace iox;
using namespace ::testing;

struct CompareOrder
{
    // NOLINTJUSTIFICATION only used in this test case
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    CompareOrder(uint32_t a, uint32_t b, uint32_t c) noexcept
        : a(a)
        , b(b)
        , c(c)
    {
    }

    uint32_t a;
    uint32_t b;
    uint32_t c;
};

class TestClass
{
  public:
    static uint32_t cTor;
    static uint32_t copyCTor;
    static uint32_t moveCTor;
    static uint32_t copyAssignment;
    static uint32_t moveAssignment;
    static uint32_t dTor;
    static std::vector<CompareOrder> dTorOrder;

    TestClass() noexcept = default;
    // NOLINTJUSTIFICATION only used in this test case
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    TestClass(const uint32_t a, const uint32_t b, const uint32_t c) noexcept
        : m_a(a)
        , m_b(b)
        , m_c(c)
    {
        cTor++;
    }

    TestClass(const TestClass& rhs) noexcept
        : TestClass(rhs.m_a, rhs.m_b, rhs.m_c)
    {
        copyCTor++;
    }

    TestClass(TestClass&& rhs) noexcept
        : TestClass(rhs.m_a, rhs.m_b, rhs.m_c)
    {
        moveCTor++;
    }

    TestClass& operator=(const TestClass& rhs) noexcept
    {
        if (this != &rhs)
        {
            copyAssignment++;
            m_a = rhs.m_a;
            m_b = rhs.m_b;
            m_c = rhs.m_c;
        }
        return *this;
    }

    TestClass& operator=(TestClass&& rhs) noexcept
    {
        if (this != &rhs)
        {
            moveAssignment++;
            m_a = rhs.m_a;
            m_b = rhs.m_b;
            m_c = rhs.m_c;
        }
        return *this;
    }

    ~TestClass() noexcept
    {
        dTor++;
        dTorOrder.emplace_back(m_a, m_b, m_c);
    }

    bool operator==(const TestClass& rhs) const noexcept
    {
        return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
    }

    bool operator==(const CompareOrder& rhs) const noexcept
    {
        return m_a == rhs.a && m_b == rhs.b && m_c == rhs.c;
    }

    uint32_t m_a = 0, m_b = 0, m_c = 0;
};

uint32_t TestClass::cTor;
uint32_t TestClass::copyCTor;
uint32_t TestClass::moveCTor;
uint32_t TestClass::copyAssignment;
uint32_t TestClass::moveAssignment;
uint32_t TestClass::dTor;

std::vector<CompareOrder> TestClass::dTorOrder;

class stack_test : public Test
{
  public:
    static constexpr uint32_t STACK_SIZE = 10U;
    stack<TestClass, STACK_SIZE> m_sut;

    void pushElements(const uint32_t numberOfElements)
    {
        for (uint32_t i = 0U; i < numberOfElements; ++i)
        {
            ASSERT_TRUE(m_sut.push(i + 1, i + 2, i + 3));
            EXPECT_THAT(m_sut.size(), Eq(static_cast<uint64_t>(i) + 1U));
            EXPECT_THAT(m_sut.capacity(), Eq(STACK_SIZE));
        }
    }

    void SetUp() override
    {
        TestClass::cTor = 0;
        TestClass::copyCTor = 0;
        TestClass::moveCTor = 0;
        TestClass::copyAssignment = 0;
        TestClass::moveAssignment = 0;
        TestClass::dTor = 0;
        TestClass::dTorOrder.clear();
    }
};

TEST_F(stack_test, isEmptyOnCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a9ce587-9daf-479d-95da-0df96325023f");
    EXPECT_THAT(m_sut.size(), Eq(0U));
    EXPECT_THAT(m_sut.capacity(), Eq(STACK_SIZE));
    EXPECT_THAT(m_sut.pop(), Eq(nullopt));
}

TEST_F(stack_test, pushingOneElementWithDefaultCtorSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "f4e91f32-fb20-4502-b0c8-d4b4d4c2bec0");
    ASSERT_TRUE(m_sut.push());
    EXPECT_THAT(m_sut.size(), Eq(1U));
    EXPECT_THAT(m_sut.capacity(), Eq(STACK_SIZE));

    auto element = m_sut.pop();
    ASSERT_TRUE(element.has_value());
    EXPECT_THAT(*element, Eq(TestClass(0, 0, 0)));
}

TEST_F(stack_test, pushingOneElementWithCustomCtorSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9112cdb-af63-4a55-b8ee-94010dc2d819");
    pushElements(1U);

    auto element = m_sut.pop();
    ASSERT_TRUE(element.has_value());
    EXPECT_THAT(*element, Eq(TestClass(1, 2, 3)));
}

TEST_F(stack_test, pushingElementsTillStackIsFullAndPoppingInLIFOOrderSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d12fd5d-ded8-482d-86dd-094660c65f9c");
    pushElements(STACK_SIZE);

    for (uint32_t i = 0U; i < STACK_SIZE; ++i)
    {
        auto element = m_sut.pop();
        EXPECT_THAT(m_sut.size(), Eq(static_cast<uint64_t>(STACK_SIZE - i) - 1U));
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
}

TEST_F(stack_test, ifCapacityIsExceededPushFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6844bc2d-f8e9-4614-ad14-62744a7421f7");
    pushElements(STACK_SIZE);

    EXPECT_FALSE(m_sut.push());
}

TEST_F(stack_test, popCreatesSpaceForAnotherElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ebf7f6d-81ef-45d6-83a6-80f8588cbba6");
    pushElements(STACK_SIZE);

    EXPECT_TRUE(m_sut.pop().has_value());
    EXPECT_TRUE(m_sut.push());

// clang >= 15 performs mandatory elision of copy/move operations (C++17 requirement)
#if defined(__clang__) && __clang_major__ >= 15
    constexpr uint64_t ELISION_CORRECTION{0};
#else
    constexpr uint64_t ELISION_CORRECTION{1};
#endif

    EXPECT_THAT(TestClass::dTor, Eq(2 + ELISION_CORRECTION));
}

TEST_F(stack_test, TestClassDTorIsCalledWhenStackGoesOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c496cb7-898b-4a65-a405-c42cbd7f0d7b");
    {
        stack<TestClass, STACK_SIZE> sut;
        sut.push();
        sut.push(1U, 2U, 3U);
        EXPECT_THAT(TestClass::dTor, Eq(0));
    }
    EXPECT_THAT(TestClass::dTor, Eq(2));
}

TEST_F(stack_test, StackDestroysElementsInReverseOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb38b063-4921-46ae-bdf2-922f49a9ab41");
    {
        stack<TestClass, STACK_SIZE> sut;
        for (uint32_t i{0}; i < STACK_SIZE; ++i)
        {
            sut.push(i + 3, i + 1, i + 2);
        }
    }
    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    ASSERT_THAT(TestClass::dTorOrder.size(), Eq(STACK_SIZE));
    for (uint32_t i{0}; i < STACK_SIZE; ++i)
    {
        EXPECT_THAT(TestClass(i + 3, i + 1, i + 2), Eq(TestClass::dTorOrder[STACK_SIZE - 1 - i]));
    }
}

TEST_F(stack_test, CopyConstructorWorksAndCallsTestClassCopyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e9d78a8-9553-42c1-b7f1-a9b26c4fc23b");
    constexpr uint32_t ELEMENT{13};
    m_sut.push(ELEMENT, ELEMENT, ELEMENT);

    stack<TestClass, STACK_SIZE> testStack(m_sut);
    EXPECT_THAT(TestClass::copyCTor, Eq(1));
    ASSERT_THAT(testStack.size(), Eq(1));
    EXPECT_THAT(testStack.pop().value(), Eq(TestClass(ELEMENT, ELEMENT, ELEMENT)));
}

TEST_F(stack_test, CopyCtorWithOneElementLeadsToEqualCtorAndDtorCalls)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc44fcdc-ced6-4345-822b-58b9e58baf85");
    constexpr uint32_t ELEMENT{37};
    {
        stack<TestClass, STACK_SIZE> other;
        other.push(TestClass(ELEMENT, ELEMENT, ELEMENT));

        stack<TestClass, STACK_SIZE> sut(other);

        EXPECT_EQ(other.size(), 1);
        EXPECT_EQ(sut.size(), 1);

        EXPECT_THAT(TestClass::dTor, Eq(1));
        EXPECT_THAT(TestClass::copyCTor, Eq(1));
        EXPECT_THAT(TestClass::moveCTor, Eq(1));

        auto element = sut.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(other.size(), Eq(1));
        EXPECT_THAT(sut.size(), Eq(0));
    }

// clang >= 15 performs mandatory elision of copy/move operations (C++17 requirement)
#if defined(__clang__) && __clang_major__ >= 15
    constexpr uint64_t ELISION_CORRECTION{0};
#else
    constexpr uint64_t ELISION_CORRECTION{1};
#endif

    EXPECT_THAT(TestClass::dTor, Eq(4 + ELISION_CORRECTION));
    EXPECT_THAT(TestClass::cTor, Eq(4 + ELISION_CORRECTION));
    EXPECT_THAT(TestClass::copyCTor, Eq(1));
    EXPECT_THAT(TestClass::moveCTor, Eq(2 + ELISION_CORRECTION));
}

TEST_F(stack_test, CopyConstructorWithEmptyStackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "08bfe7d9-233e-47cc-a7ca-5520eb6b99df");
    stack<TestClass, STACK_SIZE> testStack(m_sut);
    EXPECT_THAT(TestClass::copyCTor, Eq(0));
    EXPECT_THAT(testStack.size(), Eq(0));
}

TEST_F(stack_test, CopyConstructorWithFullStackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5ff8a1c-8bd4-40a9-9b10-7e90f232d78a");
    pushElements(STACK_SIZE);

    stack<TestClass, STACK_SIZE> testStack(m_sut);
    EXPECT_THAT(TestClass::copyCTor, Eq(STACK_SIZE));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));

    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
}

TEST_F(stack_test, CopyAssignmentWithEmptySourceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b563f12-e565-49d8-ba62-7d9a2323afdb");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;

    m_sut = testStack;

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    EXPECT_THAT(TestClass::copyAssignment, Eq(0));
    EXPECT_THAT(TestClass::copyCTor, Eq(0));
    EXPECT_THAT(m_sut.size(), Eq(0));
}

TEST_F(stack_test, CopyAssignmentWithEmptyDestinationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "81ea12ea-14ad-474c-bfd7-72433c780ceb");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;

    testStack = m_sut;

    EXPECT_THAT(TestClass::dTor, Eq(0));
    EXPECT_THAT(TestClass::copyAssignment, Eq(0));
    EXPECT_THAT(TestClass::copyCTor, Eq(STACK_SIZE));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));

    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
}

TEST_F(stack_test, CopyAssignmentWithLargerDestinationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f07ec25-fd62-414f-bb41-9284ce9f69b2");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;
    testStack.push(9U, 11U, 13U);
    const auto srcSize = testStack.size();

    m_sut = testStack;

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE - srcSize));
    EXPECT_THAT(TestClass::copyAssignment, Eq(srcSize));
    EXPECT_THAT(TestClass::copyCTor, Eq(0));
    ASSERT_THAT(m_sut.size(), Eq(srcSize));
    EXPECT_THAT(m_sut.pop().value(), Eq(TestClass(9U, 11U, 13U)));
}

TEST_F(stack_test, CopyAssignmentWithLargerSourceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a001c09-abc2-4518-a47a-30d41aca3be4");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;
    testStack.push(17U, 19U, 23U);
    const auto destSize = testStack.size();

    testStack = m_sut;

    EXPECT_THAT(TestClass::dTor, Eq(0));
    EXPECT_THAT(TestClass::copyAssignment, Eq(destSize));
    EXPECT_THAT(TestClass::copyCTor, Eq(STACK_SIZE - destSize));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));
    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
}

TEST_F(stack_test, MoveConstructorWorksAndCallsTestClassMoveConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3ad8e37-a95a-4f35-bee0-c83961c626a7");
    constexpr uint32_t ELEMENT{46};
    m_sut.push(ELEMENT, ELEMENT, ELEMENT);
    stack<TestClass, STACK_SIZE> testStack(std::move(m_sut));

    EXPECT_THAT(TestClass::moveCTor, Eq(1));
    ASSERT_THAT(testStack.size(), Eq(1));
    EXPECT_THAT(testStack.pop().value(), Eq(TestClass(ELEMENT, ELEMENT, ELEMENT)));
    EXPECT_THAT(m_sut.size(), Eq(0));
}

TEST_F(stack_test, MoveConstructorWithEmptyStackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2cb3bfc8-ef86-4648-90e0-c0d4375834cc");
    stack<TestClass, STACK_SIZE> testStack(std::move(m_sut));
    EXPECT_THAT(TestClass::moveCTor, Eq(0));
    EXPECT_THAT(testStack.size(), Eq(0));
    EXPECT_THAT(m_sut.size(), Eq(0));
}

TEST_F(stack_test, MoveConstructorWithFullStackWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "08661d3d-89d7-4ada-ba78-757e92eeb8d4");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack(std::move(m_sut));

    EXPECT_THAT(TestClass::moveCTor, Eq(STACK_SIZE));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));

    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
    EXPECT_THAT(m_sut.size(), Eq(0));
}

TEST_F(stack_test, MoveAssignmentWithEmptySourceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a06a3f8e-a886-43e2-b119-adc66c2af799");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;

    m_sut = std::move(testStack);

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    EXPECT_THAT(TestClass::moveAssignment, Eq(0));
    EXPECT_THAT(TestClass::moveCTor, Eq(0));
    EXPECT_THAT(m_sut.size(), Eq(0));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved object
    // NOLINTNEXTLINE(bugprone-use-after-move, hicpp-invalid-access-moved, clang-analyzer-cplusplus.Move)
    EXPECT_THAT(testStack.size(), Eq(0));
}

TEST_F(stack_test, MoveAssignmentWithEmptyDestinationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b67825b9-2053-453d-a2b7-5fe544f7b16d");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;

    testStack = std::move(m_sut);

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    EXPECT_THAT(TestClass::moveAssignment, Eq(0));
    EXPECT_THAT(TestClass::moveCTor, Eq(STACK_SIZE));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));

    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
    EXPECT_THAT(m_sut.size(), Eq(0));
}

TEST_F(stack_test, MoveAssignmentWithLargerDestinationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "de0b02d2-762e-41e7-a179-bd3f57da5dc6");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;
    testStack.push(9U, 11U, 13U);
    const auto srcSize = testStack.size();

    m_sut = std::move(testStack);

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    EXPECT_THAT(TestClass::moveAssignment, Eq(srcSize));
    EXPECT_THAT(TestClass::moveCTor, Eq(0));
    ASSERT_THAT(m_sut.size(), Eq(srcSize));
    EXPECT_THAT(m_sut.pop().value(), Eq(TestClass(9U, 11U, 13U)));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved object
    // NOLINTNEXTLINE(bugprone-use-after-move, hicpp-invalid-access-moved, clang-analyzer-cplusplus.Move)
    EXPECT_THAT(testStack.size(), Eq(0));
}

TEST_F(stack_test, MoveAssignmentWithLargerSourceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ff5a53d-18f7-447d-8bc8-04d2f9e38caa");
    pushElements(STACK_SIZE);
    stack<TestClass, STACK_SIZE> testStack;
    testStack.push(17U, 19U, 23U);
    const auto destSize = testStack.size();

    testStack = std::move(m_sut);

    EXPECT_THAT(TestClass::dTor, Eq(STACK_SIZE));
    EXPECT_THAT(TestClass::moveAssignment, Eq(destSize));
    EXPECT_THAT(TestClass::moveCTor, Eq(STACK_SIZE - destSize));
    EXPECT_THAT(testStack.size(), Eq(STACK_SIZE));
    for (uint32_t i = 0; i < STACK_SIZE; ++i)
    {
        auto element = testStack.pop();
        ASSERT_TRUE(element.has_value());
        EXPECT_THAT(*element, Eq(TestClass(STACK_SIZE - i, 1 + STACK_SIZE - i, 2 + STACK_SIZE - i)));
    }
    EXPECT_THAT(m_sut.size(), Eq(0));
}
} // namespace
