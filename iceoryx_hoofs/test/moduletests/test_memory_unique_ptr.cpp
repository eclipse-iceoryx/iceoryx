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

#include "iox/function.hpp"
#include "iox/unique_ptr.hpp"
#include "test.hpp"

#include <iostream>

namespace
{
using namespace ::testing;

constexpr double_t X_POS = 0.0;
constexpr double_t Y_POS = 1.1;
constexpr double_t Z_POS = 2.2;

struct Position
{
    double_t x{X_POS};
    double_t y{Y_POS};
    double_t z{Z_POS};
};

class UniquePtrTest : public Test
{
  public:
    void SetUp() override
    {
        m_deleterCalled = false;
        m_anotherDeleterCalled = false;
    }

    bool m_deleterCalled{false};
    bool m_anotherDeleterCalled{false};
    Position object;
    Position anotherObject;

    using DeleterType = void(Position* const);

    iox::function<DeleterType> deleter = [this](Position* const) { m_deleterCalled = true; };
    iox::function<DeleterType> anotherDeleter = [this](Position* const) { m_anotherDeleterCalled = true; };
};

TEST_F(UniquePtrTest, CtorWithObjectPtrAndDeleterSetsPtrToObjectAndCallsDeleter)
{
    ::testing::Test::RecordProperty("TEST_ID", "85a90fc3-e8b1-4c3d-a15c-ee7f64070b57");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        EXPECT_EQ(sut.get(), &object);
    }
    // SUT is out of scope and should have called deleter
    EXPECT_TRUE(m_deleterCalled);
}

TEST_F(UniquePtrTest, CtorUsingMoveWithObjectPtrAndDeleterSetsPtrToObjectAndCallsDeleter)
{
    ::testing::Test::RecordProperty("TEST_ID", "88ae1d4c-d893-4633-9256-766d7e42bcc6");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        {
            auto anotherSut = std::move(sut);

            // no deleter called during move
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(anotherSut.get(), &object);
        }
        // anotherSUT is out of scope and should have called deleter
        EXPECT_TRUE(m_deleterCalled);

        // reset deleter as it shouldn't be called again when SUT goes out of scope
        m_deleterCalled = false;
    }
    // no deleter called when SUT goes out of scope as it was moved
    EXPECT_FALSE(m_deleterCalled);
}

TEST_F(UniquePtrTest, MoveAssignmentUniquePtrsSetsPtrToObjectAndCallsDeleter)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3b67548-bd69-4a6f-a867-f9aaa6d869b1");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        {
            auto anotherSut = std::move(sut);

            // no deleter called during move
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(anotherSut.get(), &object);
        }
        // anotherSUT is out of scope and should have called deleter
        EXPECT_TRUE(m_deleterCalled);

        // reset deleter as it shouldn't be called again when SUT goes out of scope
        m_deleterCalled = false;
    }
    // no deleter called when SUT goes out of scope as it was moved
    EXPECT_FALSE(m_deleterCalled);
}

TEST_F(UniquePtrTest, MoveAssignmentOverwriteAUniquePtrWithAnotherOneAndCallsAnotherDeleterOnMove)
{
    ::testing::Test::RecordProperty("TEST_ID", "75a853ef-fd0e-41bd-9ce7-af63e0f67fa9");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        {
            auto anotherSut = iox::unique_ptr<Position>(&anotherObject, anotherDeleter);

            anotherSut = std::move(sut);

            // anotherSUT is overwritten so it should have called its anotherDeleter
            EXPECT_TRUE(m_anotherDeleterCalled);
            // SUT deleter not called during move
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(anotherSut.get(), &object);
        }
        // anotherSUT is out of scope and should have called deleter that has been moved to it
        EXPECT_TRUE(m_deleterCalled);

        // reset deleter as it shouldn't be called again when SUT goes out of scope
        m_deleterCalled = false;
    }
    // no deleter called when SUT goes out of scope as it was moved
    EXPECT_FALSE(m_deleterCalled);
}

TEST_F(UniquePtrTest, AccessUnderlyingObjectResultsInCorrectValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a3cc8f1-0744-4e79-85cf-02eb6c5cab9b");
    auto sut = iox::unique_ptr<Position>(&object, deleter);

    EXPECT_EQ(sut->x, X_POS);
}

TEST_F(UniquePtrTest, AccessUnderlyingObjectViaGetResultsInCorrectValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b795fa9d-b980-4987-8b94-9ea752a4e71e");
    auto sut = iox::unique_ptr<Position>(&object, deleter);

    auto* objectPtr = sut.get();

    EXPECT_EQ(objectPtr->x, X_POS);
}

TEST_F(UniquePtrTest, ReleaseAnObjectResultsInDeleterNotBeingCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a1413a5-15cd-42ff-a05e-9dff158aa047");
    auto sut = iox::unique_ptr<Position>(&object, deleter);

    auto* ptr = iox::unique_ptr<Position>::release(std::move(sut));

    EXPECT_EQ(ptr, &object);
    EXPECT_FALSE(m_deleterCalled);
}

TEST_F(UniquePtrTest, SwapTwoValidUniquePtrsWithDifferentDeletersSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4d5ed18-2d92-44f3-93d9-753bd09f5c1b");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        {
            auto anotherSut = iox::unique_ptr<Position>(&anotherObject, anotherDeleter);

            sut.swap(anotherSut);

            // no deleter calls during swap
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(sut.get(), &anotherObject);
            EXPECT_EQ(anotherSut.get(), &object);
        }
        // anotherSUT is out of scope and calls its deleter, which has been swapped and is now 'deleter'
        EXPECT_TRUE(m_deleterCalled);
        EXPECT_FALSE(m_anotherDeleterCalled);
    }
    // SUT is out of scope calling its anotherDeleter as it was swapped
    EXPECT_TRUE(m_anotherDeleterCalled);
}

TEST_F(UniquePtrTest, SwapUniquePtrWithUniquePtrLeadsToCleanupOfBothInReverseOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "9017ba22-ff18-41d4-8590-ccb0d7729435");
    {
        auto sut = iox::unique_ptr<Position>(&object, deleter);
        {
            auto anotherSut = iox::unique_ptr<Position>(&anotherObject, anotherDeleter);

            sut.swap(anotherSut);

            // no deleter calls during swap
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(anotherSut.get(), &object);
        }
        // anotherSUT is out of scope and calls its deleter, which has been swapped and is now 'deleter'
        EXPECT_TRUE(m_deleterCalled);
        EXPECT_FALSE(m_anotherDeleterCalled);
    }
    // SUT is out of scope and calling anotherDeleter
    EXPECT_TRUE(m_anotherDeleterCalled);
}

TEST_F(UniquePtrTest, CompareAUniquePtrWithItselfIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d12f8cf6-e37e-424a-9ed5-aea580b8bdc9");
    auto sut = iox::unique_ptr<Position>(&object, deleter);

    EXPECT_TRUE(sut == sut);
}

TEST_F(UniquePtrTest, CompareAUniquePtrWithAnotherOneOfAnotherObjectIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a6135d2-1a79-49fa-a142-7e19327b6a9f");
    auto sut = iox::unique_ptr<Position>(&object, deleter);
    auto anotherSut = iox::unique_ptr<Position>(&anotherObject, anotherDeleter);

    EXPECT_FALSE(sut == anotherSut);
    EXPECT_FALSE(anotherSut == sut);
}

TEST_F(UniquePtrTest, NotEqualCompareOfAUniquePtrWithItselfIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6305a2d9-28d7-41a0-bb0b-866912a39205");
    auto sut = iox::unique_ptr<Position>(&object, deleter);

    EXPECT_FALSE(sut != sut);
}

TEST_F(UniquePtrTest, NotEqualCompareOfAUniquePtrWithAnotherOneOfAnotherObjectIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "58b9cd12-82f9-4e3a-b033-8c57afbd31d7");
    auto sut = iox::unique_ptr<Position>(&object, deleter);
    auto anotherSut = iox::unique_ptr<Position>(&anotherObject, anotherDeleter);

    EXPECT_TRUE(sut != anotherSut);
    EXPECT_TRUE(anotherSut != sut);
}

TEST_F(UniquePtrTest, CanGetUnderlyingPtrFromConstUniquePtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "75727c11-f721-4a52-816a-a9a3a61e2b43");
    const auto sut = iox::unique_ptr<Position>(&object, deleter);
    EXPECT_EQ(sut.get(), &object);
}

TEST_F(UniquePtrTest, CanUseArrowOperatorToAccessObjectInConstUniquePtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "045a9026-74f5-41ad-9881-14c2502527c4");
    const auto sut = iox::unique_ptr<Position>(&object, deleter);
    EXPECT_EQ(X_POS, sut->x);
    EXPECT_EQ(Y_POS, sut->y);
    EXPECT_EQ(Z_POS, sut->z);
}
} // namespace
