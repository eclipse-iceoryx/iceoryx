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

#include "iceoryx_hoofs/internal/concurrent/sofi.hpp"

#include <gtest/gtest.h>
#include <stdlib.h>

namespace
{
class CUnitTestContainerSoFi : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    /// @brief returns the ID of the current test in the format "TestCaseName.TestName"
    /// @return std::string: the id of the test
    std::string testId();

    /// @brief pushes some serial numbers to the SoFi with the expectation to not overflow
    /// @param[in] serNumStart is the offset for the serial numbers to push into the SoFi
    /// @param[in] numberOfItems is the number of items to push into the SoFi
    /// @return int: the first not pushed serial number
    int pushSome(int serNumStart, uint32_t numberOfItems);

    /// @brief pops some items from the SoFi with the expectation of a non empty SoFi at the last pop
    /// @param[in] serNumOldest is the oldest serial numbers in the SoFi
    /// @param[in] numberOfItems is the number of items to pop from the SoFi
    void popSome(int serNumOldest, uint32_t numberOfItems);


    /// @brief pops all items from the SoFi
    /// @param[in] serNumOldest is the oldest serial numbers in the SoFi
    void popAll(int serNumOldest);

    /// @brief checks the SoFi empty behaviour; pop on empty SoFi, pushing and popping the same amount of items
    /// @param[in] scope is the identifier for gTest SCOPED_TRACE to trace the failure when subroutines are used
    /// @param[in] serNumStart is the offset for the serial numbers to push into the SoFi
    void checkEmpty(const std::string& scope, int serNumStart);

    /// @brief checks the capacity ot the SoFi
    /// @param[in] scope is the identifier for gTest SCOPED_TRACE to trace the failure when subroutines are used
    /// @param[in] serNumStart is the offset for the serial numbers to push into the SoFi
    void checkCapacity(const std::string& scope, int serNumStart);

    /// @brief checks if the SoFi overflow works as expected with one overflow
    /// @param[in] scope is the identifier for gTest SCOPED_TRACE to trace the failure when subroutines are used
    /// @param[in] serNumStart is the offset for the serial numbers to push into the SoFi
    void checkOverflow(const std::string& scope, int serNumStart);

    /// @brief checks if the SoFi overflow works as expected with multiple overflowing
    /// @param[in] scope is the identifier for gTest SCOPED_TRACE to trace the failure when subroutines are used
    /// @param[in] serNumStart is the offset for the serial numbers to push into the SoFi
    void checkMultiOverflow(const std::string& scope, int serNumStart);

    static constexpr uint64_t TEST_SOFI_CAPACITY = 10;
    iox::concurrent::SoFi<int, TEST_SOFI_CAPACITY> m_sofi;
};

std::string CUnitTestContainerSoFi::testId()
{
    return std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()) + "."
           + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name());
}

int CUnitTestContainerSoFi::pushSome(int serNumStart, uint32_t numberOfItems)
{
    int valIn = serNumStart;
    int valOut;

    // fill the SoFi; SoFi has an internal capacity with two more items than specified the ones
    // the write position must always point to an empty position
    // -> SoFi is full when we are at TEST_INTERNAL_SOFI_CAPACITY-1
    for (uint32_t i = 0; i < numberOfItems; i++, valIn++)
    {
        valOut = -1; // we push only positive values, so this is a value we shouldn't get from the SoFi
        EXPECT_TRUE(this->m_sofi.push(valIn, valOut)) << "There shouldn't be an overflow here!";
        EXPECT_EQ(-1, valOut); // valOut should not be changed if there was no overflow
    }

    return valIn;
}

void CUnitTestContainerSoFi::popSome(int serNumOldest, uint32_t numberOfItems)
{
    int valOut;
    for (uint32_t i = 0; i < numberOfItems; i++)
    {
        valOut = -2; // set valOut to a value not present in the SoFi
        EXPECT_TRUE(m_sofi.pop(valOut)) << "SoFi shouldn't be empty here!";
        EXPECT_EQ(serNumOldest + static_cast<int>(i), valOut); // check if the serial numbers are really consecutive
    }
}

void CUnitTestContainerSoFi::popAll(int serNumOldest)
{
    int valOut{-2}; // set valOut to a value not present in the SoFi
    int serNum = serNumOldest;
    while (m_sofi.pop(valOut))
    {
        EXPECT_EQ(serNum, valOut); // check if we have valid data
        serNum++;
        valOut = -2;
    }
}

void CUnitTestContainerSoFi::checkEmpty(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    int valOut{-1}; // set valOut to a value not present in the SoFi

    EXPECT_TRUE(m_sofi.empty()) << "SoFi should be empty!";

    EXPECT_FALSE(m_sofi.pop(valOut)) << "It shouldn't be possible to pop from empty SoFi!";

    // push an item
    valOut = -1;                                   // set valOut to a value not present in the SoFi
    EXPECT_TRUE(m_sofi.push(serNumStart, valOut)); // if empty, it should be possible to push
    EXPECT_EQ(-1, valOut);                         // if push successful, valOut should not change

    EXPECT_FALSE(m_sofi.empty()) << "SoFi shouldn't be empty anymore!";

    valOut = -1;                     // set valOut to a value not present in the SoFi
    EXPECT_TRUE(m_sofi.pop(valOut)); // if not empty, pop should be successful
    EXPECT_EQ(serNumStart, valOut);  // if pop successful, valOut should be serNumStart

    EXPECT_TRUE(m_sofi.empty()) << "SoFi should be empty again!";
}

void CUnitTestContainerSoFi::checkCapacity(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    // fill the SoFi
    int valIn = pushSome(serNumStart, TEST_SOFI_CAPACITY);
    // one more element should cause an overflow, which means the SoFi was already full
    int valOut{-1}; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(m_sofi.push(valIn, valOut)) << "No overflow occured! SoFi is not full yet!";
    EXPECT_EQ(serNumStart, valOut); // in the case of an overflow, the oldest item is returned

    // empty the SoFi
    popAll(valOut + 1);
}

void CUnitTestContainerSoFi::checkOverflow(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    int valIn{0};
    int valOut{-2};

    // fill the SoFi and return the first not pushed serial number
    valIn = pushSome(serNumStart, TEST_SOFI_CAPACITY);
    // pushing another item, should cause an overflow and returning the oldest pushed item
    EXPECT_FALSE(m_sofi.push(valIn, valOut)) << "Expected overflow didn't occur";
    EXPECT_EQ(serNumStart, valOut);

    // popping should return the remaining item
    popSome(serNumStart + 1, TEST_SOFI_CAPACITY); // we had an overflow, so the serial number is off by one

    // SoFi should now be empty
    valOut = -2; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(m_sofi.pop(valOut)) << "SoFi is not empty as expected!";
    EXPECT_EQ(-2, valOut); // valOut not changed if empty
}

void CUnitTestContainerSoFi::checkMultiOverflow(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    int valIn{0};
    int valOut{-2};

    // fill the SoFi and return the first not pushed serial number
    valIn = pushSome(serNumStart, TEST_SOFI_CAPACITY);
    // pushing additional items, should cause an overflow and returning the oldest pushed item
    // lets run three times throug the container
    int serNumExp = serNumStart;
    for (uint32_t i = 0; i < 3 * TEST_SOFI_CAPACITY; i++)
    {
        valOut = -2; // set valOut to a value not present in the SoFi
        EXPECT_FALSE(m_sofi.push(valIn, valOut)) << "Expected overflow didn't occur at iteration " << i << "!";
        EXPECT_EQ(serNumExp, valOut); // in case of an overflow, we should get the oldest item out of the SoFi
        valIn++;
        serNumExp++;
    }

    // popping should return the remaining item
    popSome(serNumExp, TEST_SOFI_CAPACITY); // we had an overflow, so the serial number is off by one

    // SoFi should now be empty
    valOut = -2; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(m_sofi.pop(valOut)) << "SoFi is not empty as expected!";
    EXPECT_EQ(-2, valOut); // valOut not changed if empty
}

TEST_F(CUnitTestContainerSoFi, Empty)
{
    ::testing::Test::RecordProperty("TEST_ID", "557d4e60-b214-4170-a07a-bf7ccbc38ba6");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(m_sofi.empty());

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkEmpty("first", serNumStart);
    // repeat the thest with a non zero initial read and write position
    serNumStart = 2000;
    checkEmpty("second", serNumStart);
}

TEST_F(CUnitTestContainerSoFi, Capacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "693ea584-72b2-401a-8a52-b5159eecdb53");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if SoFi sets the right capacity
    EXPECT_EQ(static_cast<uint32_t>(TEST_SOFI_CAPACITY), m_sofi.capacity());

    // check if SoFi doesn't lie to us

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkCapacity("first", serNumStart);
    // repeat the thest with a non zero initial read and write position
    serNumStart = 2000;
    checkCapacity("second", serNumStart);
}

TEST_F(CUnitTestContainerSoFi, NewlyCreatedSoFiIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e29ee14-c592-4d60-b7c0-c38bd390e518");
    EXPECT_TRUE(m_sofi.empty());
}

TEST_F(CUnitTestContainerSoFi, NewlyCreatedSoFiHasSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "89f0ccea-2e96-4a8c-9279-d33aec95b4c9");
    EXPECT_EQ(m_sofi.size(), 0);
}

TEST_F(CUnitTestContainerSoFi, SoFiSizeEqualsNumberOfPushes)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf415600-d1f5-45bb-8e23-7d72a8212efe");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(m_sofi.empty());

    int ret;

    // Push 10 items till SoFi is full and check size
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; i++)
    {
        EXPECT_EQ(m_sofi.size(), i);
        m_sofi.push(i, ret);
        EXPECT_EQ(m_sofi.size(), i + 1);
    }
}

TEST_F(CUnitTestContainerSoFi, SoFiSizeEqualsNumberOfPushesOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "be946957-dddc-4038-8b34-cea6f8931e5e");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(m_sofi.empty());

    int ret;

    // Push 11 items to provoke overflow and check size
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; i++)
    {
        EXPECT_EQ(m_sofi.size(), i);
        m_sofi.push(i, ret);
        EXPECT_EQ(m_sofi.size(), i + 1);
    }
}

TEST_F(CUnitTestContainerSoFi, Overflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "47548956-f8f6-4649-9a04-eb766a014171");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkOverflow("first", serNumStart);
    // repeat the thest with a non zero initial read and write position
    serNumStart = 2000;
    checkOverflow("second", serNumStart);
}

TEST_F(CUnitTestContainerSoFi, MultiOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b229258-250a-4cf6-b73f-ab5235a10624");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkMultiOverflow("first", serNumStart);
    // repeat the test with a non zero initial read and write position
    serNumStart = 2000;
    checkMultiOverflow("second", serNumStart);
}

TEST_F(CUnitTestContainerSoFi, ResizeFailsWhenContainingASingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c7c43d8-939c-4fa8-b1b9-b379515931e9");
    int ret;
    m_sofi.push(123, ret);
    EXPECT_EQ(m_sofi.setCapacity(4), false);
}

TEST_F(CUnitTestContainerSoFi, ResizeFailsWhenContainingAMultipleElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "a98bd656-7d39-4274-a77f-bc918a2c1301");
    int ret;
    m_sofi.push(123, ret);
    m_sofi.push(13, ret);
    m_sofi.push(23, ret);
    EXPECT_EQ(m_sofi.setCapacity(4), false);
}

TEST_F(CUnitTestContainerSoFi, ResizeFailsWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f58b6dd-20ab-42c7-9006-fbbcadb04f42");
    for (int ret; !m_sofi.push(123, ret);)
    {
    }
    EXPECT_EQ(m_sofi.setCapacity(4), false);
}

TEST_F(CUnitTestContainerSoFi, ResizingLargeThanCapacityFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "609918f3-56aa-4e7e-8f7c-d171f2ca4602");
    EXPECT_EQ(m_sofi.setCapacity(TEST_SOFI_CAPACITY + 1), false);
}

TEST_F(CUnitTestContainerSoFi, ResizingToZeroIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "6675b4c4-7866-43d3-b3b2-aa1bff6b3053");
    EXPECT_EQ(m_sofi.setCapacity(0), true);
}

TEST_F(CUnitTestContainerSoFi, ResizingDefault)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2371e2a-56f2-4ab1-a168-a53fa2440f0b");
    EXPECT_EQ(m_sofi.setCapacity(TEST_SOFI_CAPACITY - 1), true);
}

TEST_F(CUnitTestContainerSoFi, ResizeAndSizeCheck)
{
    ::testing::Test::RecordProperty("TEST_ID", "b916cb44-303c-4dc3-8900-aea244482ef6");
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; ++i)
    {
        EXPECT_EQ(m_sofi.setCapacity(i), true);
        EXPECT_EQ(m_sofi.capacity(), i);
    }
}

TEST_F(CUnitTestContainerSoFi, ResizeAndSizeFillUp)
{
    ::testing::Test::RecordProperty("TEST_ID", "3db02cd3-68ac-4507-8437-6bdbe423babf");
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY - 1; ++i)
    {
        EXPECT_EQ(m_sofi.setCapacity(i), true);
        for (uint32_t k = 0; k < i; ++k)
        {
            int temp;
            EXPECT_EQ(m_sofi.push(static_cast<int>(k), temp), true);
        }
        for (uint32_t k = 0; k < i; ++k)
        {
            int temp;
            EXPECT_EQ(m_sofi.pop(temp), true);
            EXPECT_EQ(temp, static_cast<int>(k));
        }
    }
}

TEST_F(CUnitTestContainerSoFi, PopIfWithValidCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "f149035c-21cc-4f7d-ba4d-564a645e933b");
    int returnValue;
    m_sofi.push(10, returnValue);
    m_sofi.push(11, returnValue);
    m_sofi.push(12, returnValue);

    int output;
    bool result = m_sofi.popIf(output, [](const int& peek) { return peek < 20; });

    EXPECT_EQ(result, true);
    EXPECT_EQ(output, 10);
}

TEST_F(CUnitTestContainerSoFi, PopIfWithInvalidCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a494c28-928f-48f4-8b01-e68dfbd7563e");
    int returnValue;
    m_sofi.push(15, returnValue);
    m_sofi.push(16, returnValue);
    m_sofi.push(17, returnValue);

    int output;
    bool result = m_sofi.popIf(output, [](const int& peek) { return peek < 5; });

    EXPECT_EQ(result, false);
}

TEST_F(CUnitTestContainerSoFi, PopIfOnEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "960ad78f-cb9b-4c34-a077-6adb343a841c");
    int output;
    bool result = m_sofi.popIf(output, [](const int& peek) { return peek < 7; });

    EXPECT_EQ(result, false);
}

TEST_F(CUnitTestContainerSoFi, PopIfFullWithValidCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "167f2f01-f926-4442-bc4f-ff5e7cfe9fe0");
    int output;
    constexpr int INITIAL_VALUE = 100;
    constexpr int OFFSET = 2;
    for (int i = 0; i < static_cast<int>(m_sofi.capacity()) + OFFSET; i++)
        m_sofi.push(i + INITIAL_VALUE, output);

    bool result = m_sofi.popIf(output, [](const int& peek) { return peek < 150; });

    EXPECT_EQ(result, true);
    EXPECT_EQ(output, INITIAL_VALUE + OFFSET);
}

TEST_F(CUnitTestContainerSoFi, PopIfFullWithInvalidCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "672881b9-eebd-471d-9d62-e792a8b8013f");
    int output;
    for (int i = 0; i < static_cast<int>(m_sofi.capacity()) + 2; i++)
        m_sofi.push(i + 100, output);

    bool result = m_sofi.popIf(output, [](const int& peek) { return peek < 50; });

    EXPECT_EQ(result, false);
}

TEST_F(CUnitTestContainerSoFi, PopIfValidEmptyAfter)
{
    ::testing::Test::RecordProperty("TEST_ID", "19444dcd-7746-4e6b-a3b3-398c9d62317d");
    int output;
    m_sofi.push(2, output);

    m_sofi.popIf(output, [](const int& peek) { return peek < 50; });

    EXPECT_EQ(m_sofi.empty(), true);
}

TEST_F(CUnitTestContainerSoFi, PopIfInvalidNotEmptyAfter)
{
    ::testing::Test::RecordProperty("TEST_ID", "cadd7f02-6fe5-49a5-bd5d-837f5fcb2a71");
    int output;
    m_sofi.push(200, output);

    m_sofi.popIf(output, [](const int& peek) { return peek < 50; });

    EXPECT_EQ(m_sofi.empty(), false);
}

/// @todo popif empty test
} // namespace
