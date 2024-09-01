// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/detail/spsc_sofi.hpp"

#include <cstdlib>
#include <gtest/gtest.h>

namespace
{
struct SpscSofiTest : public ::testing::Test
{
    /// @brief returns the ID of the current test in the format "TestCaseName.TestName"
    /// @return std::string: the id of the test
    static std::string testId();

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
    iox::concurrent::SpscSofi<int, TEST_SOFI_CAPACITY> sofi;
    int returnVal{-1}; // set to a value that should not be present in the SoFi
};

std::string SpscSofiTest::testId()
{
    return std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()) + "."
           + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name());
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) used only for test purposes
int SpscSofiTest::pushSome(int serNumStart, uint32_t numberOfItems)
{
    int valIn = serNumStart;
    int valOut{-1};

    // fill the SoFi; SoFi has an internal capacity with two more items than specified the ones
    // the write position must always point to an empty position
    // -> SoFi is full when we are at TEST_INTERNAL_SOFI_CAPACITY-1
    for (uint32_t i = 0; i < numberOfItems; i++, valIn++)
    {
        valOut = -1; // we push only positive values, so this is a value we shouldn't get from the SoFi
        EXPECT_TRUE(this->sofi.push(valIn, valOut)) << "There shouldn't be an overflow here!";
        EXPECT_EQ(-1, valOut); // valOut should not be changed if there was no overflow
    }

    return valIn;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) used only for test purposes
void SpscSofiTest::popSome(int serNumOldest, uint32_t numberOfItems)
{
    int valOut{-2};
    for (uint32_t i = 0; i < numberOfItems; i++)
    {
        valOut = -2; // set valOut to a value not present in the SoFi
        EXPECT_TRUE(sofi.pop(valOut)) << "SoFi shouldn't be empty here!";
        EXPECT_EQ(serNumOldest + static_cast<int>(i), valOut); // check if the serial numbers are really consecutive
    }
}

void SpscSofiTest::popAll(int serNumOldest)
{
    int valOut{-2}; // set valOut to a value not present in the SoFi
    int serNum = serNumOldest;
    while (sofi.pop(valOut))
    {
        EXPECT_EQ(serNum, valOut); // check if we have valid data
        serNum++;
        valOut = -2;
    }
}

void SpscSofiTest::checkEmpty(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    int valOut{-1}; // set valOut to a value not present in the SoFi

    EXPECT_TRUE(sofi.empty()) << "SoFi should be empty!";

    EXPECT_FALSE(sofi.pop(valOut)) << "It shouldn't be possible to pop from empty SoFi!";

    // push an item
    valOut = -1;                                 // set valOut to a value not present in the SoFi
    EXPECT_TRUE(sofi.push(serNumStart, valOut)); // if empty, it should be possible to push
    EXPECT_EQ(-1, valOut);                       // if push successful, valOut should not change

    EXPECT_FALSE(sofi.empty()) << "SoFi shouldn't be empty anymore!";

    valOut = -1;                    // set valOut to a value not present in the SoFi
    EXPECT_TRUE(sofi.pop(valOut));  // if not empty, pop should be successful
    EXPECT_EQ(serNumStart, valOut); // if pop successful, valOut should be serNumStart

    EXPECT_TRUE(sofi.empty()) << "SoFi should be empty again!";
}

void SpscSofiTest::checkCapacity(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    // fill the SoFi
    int valIn = pushSome(serNumStart, TEST_SOFI_CAPACITY);
    // one more element should cause an overflow, which means the SoFi was already full
    int valOut{-1}; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(sofi.push(valIn, valOut)) << "No overflow occured! SoFi is not full yet!";
    EXPECT_EQ(serNumStart, valOut); // in the case of an overflow, the oldest item is returned

    // empty the SoFi
    popAll(valOut + 1);
}

void SpscSofiTest::checkOverflow(const std::string& scope, int serNumStart)
{
    SCOPED_TRACE(scope); // just a helper to trace the failure when subroutines are used

    int valIn{0};
    int valOut{-2};

    // fill the SoFi and return the first not pushed serial number
    valIn = pushSome(serNumStart, TEST_SOFI_CAPACITY);
    // pushing another item, should cause an overflow and returning the oldest pushed item
    EXPECT_FALSE(sofi.push(valIn, valOut)) << "Expected overflow didn't occur";
    EXPECT_EQ(serNumStart, valOut);

    // popping should return the remaining item
    popSome(serNumStart + 1, TEST_SOFI_CAPACITY); // we had an overflow, so the serial number is off by one

    // SoFi should now be empty
    valOut = -2; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(sofi.pop(valOut)) << "SoFi is not empty as expected!";
    EXPECT_EQ(-2, valOut); // valOut not changed if empty
}

void SpscSofiTest::checkMultiOverflow(const std::string& scope, int serNumStart)
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
        EXPECT_FALSE(sofi.push(valIn, valOut)) << "Expected overflow didn't occur at iteration " << i << "!";
        EXPECT_EQ(serNumExp, valOut); // in case of an overflow, we should get the oldest item out of the SoFi
        valIn++;
        serNumExp++;
    }

    // popping should return the remaining item
    popSome(serNumExp, TEST_SOFI_CAPACITY); // we had an overflow, so the serial number is off by one

    // SoFi should now be empty
    valOut = -2; // set valOut to a value not present in the SoFi
    EXPECT_FALSE(sofi.pop(valOut)) << "SoFi is not empty as expected!";
    EXPECT_EQ(-2, valOut); // valOut not changed if empty
}

TEST_F(SpscSofiTest, Empty)
{
    ::testing::Test::RecordProperty("TEST_ID", "557d4e60-b214-4170-a07a-bf7ccbc38ba6");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(sofi.empty());

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkEmpty("first", serNumStart);
    // repeat the thest with a non zero initial read and write position
    serNumStart = 2000;
    checkEmpty("second", serNumStart);
}

TEST_F(SpscSofiTest, Capacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "693ea584-72b2-401a-8a52-b5159eecdb53");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if SoFi sets the right capacity
    EXPECT_EQ(static_cast<uint32_t>(TEST_SOFI_CAPACITY), sofi.capacity());

    // check if SoFi doesn't lie to us

    // test with an initial SoFi read and write position of zero
    int serNumStart{1000};
    checkCapacity("first", serNumStart);
    // repeat the thest with a non zero initial read and write position
    serNumStart = 2000;
    checkCapacity("second", serNumStart);
}

TEST_F(SpscSofiTest, NewlyCreatedSoFiIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e29ee14-c592-4d60-b7c0-c38bd390e518");
    EXPECT_TRUE(sofi.empty());
}

TEST_F(SpscSofiTest, NewlyCreatedSoFiHasSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "89f0ccea-2e96-4a8c-9279-d33aec95b4c9");
    EXPECT_EQ(sofi.size(), 0U);
}

TEST_F(SpscSofiTest, SoFiSizeEqualsNumberOfPushes)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf415600-d1f5-45bb-8e23-7d72a8212efe");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(sofi.empty());

    // Push 10 items till SoFi is full and check size
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; i++)
    {
        EXPECT_EQ(sofi.size(), i);
        sofi.push(static_cast<int>(i), returnVal);
        EXPECT_EQ(sofi.size(), i + 1);
    }
}

TEST_F(SpscSofiTest, SoFiSizeEqualsNumberOfPushesOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "be946957-dddc-4038-8b34-cea6f8931e5e");
    SCOPED_TRACE(testId()); // just a helper to trace the failure when subroutines are used

    // check if a new instace of the SoFi is empty
    EXPECT_TRUE(sofi.empty());

    // Push 11 items to provoke overflow and check size
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; i++)
    {
        EXPECT_EQ(sofi.size(), i);
        sofi.push(static_cast<int>(i), returnVal);
        EXPECT_EQ(sofi.size(), i + 1);
    }
}

TEST_F(SpscSofiTest, Overflow)
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

TEST_F(SpscSofiTest, MultiOverflow)
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

TEST_F(SpscSofiTest, ResizeFailsWhenContainingASingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c7c43d8-939c-4fa8-b1b9-b379515931e9");
    sofi.push(123, returnVal);
    EXPECT_EQ(sofi.setCapacity(4), false);
}

TEST_F(SpscSofiTest, ResizeFailsWhenContainingAMultipleElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "a98bd656-7d39-4274-a77f-bc918a2c1301");
    sofi.push(123, returnVal);
    sofi.push(13, returnVal);
    sofi.push(23, returnVal);
    EXPECT_EQ(sofi.setCapacity(4), false);
}

TEST_F(SpscSofiTest, ResizeFailsWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f58b6dd-20ab-42c7-9006-fbbcadb04f42");
    while (!sofi.push(123, returnVal))
    {
    }
    EXPECT_EQ(sofi.setCapacity(4), false);
}

TEST_F(SpscSofiTest, ResizingLargeThanCapacityFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "609918f3-56aa-4e7e-8f7c-d171f2ca4602");
    EXPECT_EQ(sofi.setCapacity(TEST_SOFI_CAPACITY + 1), false);
}

TEST_F(SpscSofiTest, ResizingToZeroIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "6675b4c4-7866-43d3-b3b2-aa1bff6b3053");
    EXPECT_EQ(sofi.setCapacity(0), true);
}

TEST_F(SpscSofiTest, ResizingDefault)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2371e2a-56f2-4ab1-a168-a53fa2440f0b");
    EXPECT_EQ(sofi.setCapacity(TEST_SOFI_CAPACITY - 1), true);
}

TEST_F(SpscSofiTest, ResizeAndSizeCheck)
{
    ::testing::Test::RecordProperty("TEST_ID", "b916cb44-303c-4dc3-8900-aea244482ef6");
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY; ++i)
    {
        EXPECT_EQ(sofi.setCapacity(i), true);
        EXPECT_EQ(sofi.capacity(), i);
    }
}

TEST_F(SpscSofiTest, ResizeAndSizeFillUp)
{
    ::testing::Test::RecordProperty("TEST_ID", "3db02cd3-68ac-4507-8437-6bdbe423babf");
    for (uint32_t i = 0; i < TEST_SOFI_CAPACITY - 1; ++i)
    {
        EXPECT_EQ(sofi.setCapacity(i), true);
        for (uint32_t k = 0; k < i; ++k)
        {
            returnVal = -1;
            EXPECT_EQ(sofi.push(static_cast<int>(k), returnVal), true);
        }
        for (uint32_t k = 0; k < i; ++k)
        {
            returnVal = -1;
            EXPECT_EQ(sofi.pop(returnVal), true);
            EXPECT_EQ(returnVal, static_cast<int>(k));
        }
    }
}

TEST_F(SpscSofiTest, Pop)
{
    ::testing::Test::RecordProperty("TEST_ID", "f149035c-21cc-4f7d-ba4d-564a645e933b");
    sofi.push(10, returnVal);
    sofi.push(11, returnVal);
    sofi.push(12, returnVal);

    int output{-1};
    bool result = sofi.pop(output);

    EXPECT_EQ(result, true);
    EXPECT_EQ(output, 10);
}


TEST_F(SpscSofiTest, PopOnEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "960ad78f-cb9b-4c34-a077-6adb343a841c");
    bool result = sofi.pop(returnVal);

    EXPECT_EQ(result, false);
}

TEST_F(SpscSofiTest, PopFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "167f2f01-f926-4442-bc4f-ff5e7cfe9fe0");
    constexpr int INITIAL_VALUE = 100;
    constexpr int OFFSET = 2;
    for (int i = 0; i < static_cast<int>(sofi.capacity()) + OFFSET; i++)
    {
        sofi.push(i + INITIAL_VALUE, returnVal);
    }

    bool result = sofi.pop(returnVal);

    EXPECT_EQ(result, true);
    EXPECT_EQ(returnVal, INITIAL_VALUE + OFFSET);
}

TEST_F(SpscSofiTest, PopEmptyAfter)
{
    ::testing::Test::RecordProperty("TEST_ID", "19444dcd-7746-4e6b-a3b3-398c9d62317d");

    sofi.push(2, returnVal);

    sofi.pop(returnVal);

    EXPECT_EQ(sofi.empty(), true);
}

} // namespace
