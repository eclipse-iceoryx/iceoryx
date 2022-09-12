// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
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

    iox::cxx::function<void(Position* const)> deleter = [this](Position* const p) {
        m_deleterCalled = true;
        delete p;
    };

    iox::cxx::function<void(Position* const)> anotherDeleter = [this](Position* const p) {
        m_anotherDeleterCalled = true;
        delete p;
    };
};

TEST_F(UniquePtrTest, CtorWithObjectPtrAndDeleterSetsPtrToObjectAndCallsDeleter)
{
    ::testing::Test::RecordProperty("TEST_ID", "85a90fc3-e8b1-4c3d-a15c-ee7f64070b57");
    {
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        EXPECT_TRUE(sut);
        EXPECT_EQ(sut.get(), object);
    }
    // SUT is out of scope and should have called deleter
    EXPECT_TRUE(m_deleterCalled);
}

TEST_F(UniquePtrTest, CtorUsingMoveWithObjectPtrAndDeleterSetsPtrToObjectAndCallsDeleter)
{
    ::testing::Test::RecordProperty("TEST_ID", "88ae1d4c-d893-4633-9256-766d7e42bcc6");
    {
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        {
            auto anotherSut = std::move(sut);

            // no deleter called during move
            EXPECT_FALSE(m_deleterCalled);
            // NOLINTJUSTIFICATION check if move is invalidating the object
            // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
            EXPECT_FALSE(sut);
            EXPECT_EQ(anotherSut.get(), object);
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
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        {
            auto anotherSut = std::move(sut);

            // no deleter called during move
            EXPECT_FALSE(m_deleterCalled);
            // NOLINTJUSTIFICATION check if move is invalidating the object
            // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
            EXPECT_FALSE(sut);
            EXPECT_EQ(anotherSut.get(), object);
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
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        {
            // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
            auto* anotherObject = new Position();
            auto anotherSut = iox::cxx::unique_ptr<Position>(anotherObject, anotherDeleter);

            anotherSut = std::move(sut);

            // anotherSUT is overwritten so it should have called its anotherDeleter
            EXPECT_TRUE(m_anotherDeleterCalled);
            // SUT deleter not called during move
            EXPECT_FALSE(m_deleterCalled);
            // NOLINTJUSTIFICATION check if move is invalidating the object
            // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
            EXPECT_FALSE(sut);
            EXPECT_EQ(anotherSut.get(), object);
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
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_EQ(sut->x, X_POS);
}

TEST_F(UniquePtrTest, AccessUnderlyingObjectViaGetResultsInCorrectValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b795fa9d-b980-4987-8b94-9ea752a4e71e");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    auto* objectPtr = sut.get();

    EXPECT_EQ(objectPtr->x, X_POS);
}

TEST_F(UniquePtrTest, ReleaseAnObjectResultsInUniquePtrBeingInvalidAndReturnOfObjectPtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a1413a5-15cd-42ff-a05e-9dff158aa047");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_EQ(sut.release(), object);
    EXPECT_FALSE(sut);
    delete object;
    EXPECT_FALSE(m_deleterCalled);
}

TEST_F(UniquePtrTest, ReleaseNullObjectResultsInUniquePtrBeingInvalidAndReturnOfNull)
{
    ::testing::Test::RecordProperty("TEST_ID", "056697e8-16e1-4a42-94a4-500cd2169cf7");
    auto sut = iox::cxx::unique_ptr<Position>(nullptr, deleter);

    EXPECT_EQ(sut.release(), nullptr);
    EXPECT_FALSE(sut);
}

TEST_F(UniquePtrTest, ResetToAnExistingObjectPtrResultsInDeleterCalledTwice)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5da7713-e71d-49b2-8bf6-d6108aab6366");
    {
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto* anotherObject = new Position();
        // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

        sut.reset(anotherObject);

        // deleter called first time then SUT is resetted
        EXPECT_TRUE(m_deleterCalled);
        EXPECT_EQ(sut.get(), anotherObject);

        // reset deleter as it's called again when SUT goes out of scope
        m_deleterCalled = false;
    }
    // deleter called second time when SUT is going of scope
    EXPECT_TRUE(m_deleterCalled);
}

TEST_F(UniquePtrTest, SwapTwoValidUniquePtrsWithDifferentDeletersSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4d5ed18-2d92-44f3-93d9-753bd09f5c1b");
    {
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        {
            // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
            auto* anotherObject = new Position();
            auto anotherSut = iox::cxx::unique_ptr<Position>(anotherObject, anotherDeleter);

            sut.swap(anotherSut);

            // no deleter calls during swap
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_EQ(sut.get(), anotherObject);
            EXPECT_EQ(anotherSut.get(), object);
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
        // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
        // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* object = new Position();
        auto* anotherObject = new Position();
        // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
        {
            auto anotherSut = iox::cxx::unique_ptr<Position>(anotherObject, anotherDeleter);

            sut.swap(anotherSut);

            // no deleter calls during swap
            EXPECT_FALSE(m_deleterCalled);
            EXPECT_TRUE(sut);
            EXPECT_EQ(anotherSut.get(), object);
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
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_TRUE(sut == sut);
}

TEST_F(UniquePtrTest, CompareAUniquePtrWithNullIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "45e6ca29-8164-414c-af01-dc2cbb38de57");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_FALSE(sut == nullptr);
    EXPECT_FALSE(nullptr == sut);
}

TEST_F(UniquePtrTest, CompareAUniquePtrWithAnotherOneOfAnotherObjectIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a6135d2-1a79-49fa-a142-7e19327b6a9f");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position;
    auto* anotherObject = new Position;
    // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
    auto anotherSut = iox::cxx::unique_ptr<Position>(anotherObject, anotherDeleter);

    EXPECT_FALSE(sut == anotherSut);
    EXPECT_FALSE(anotherSut == sut);
}

TEST_F(UniquePtrTest, NotEqualCompareOfAUniquePtrWithItselfIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6305a2d9-28d7-41a0-bb0b-866912a39205");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_FALSE(sut != sut);
}

TEST_F(UniquePtrTest, NotEqualCompareOfAUniquePtrWithAnotherOneOfAnotherObjectIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "58b9cd12-82f9-4e3a-b033-8c57afbd31d7");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position;
    auto* anotherObject = new Position;
    // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
    auto anotherSut = iox::cxx::unique_ptr<Position>(anotherObject, anotherDeleter);

    EXPECT_TRUE(sut != anotherSut);
    EXPECT_TRUE(anotherSut != sut);
}

TEST_F(UniquePtrTest, NotEqualCompareAUniquePtrWithNullIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fe92923-dd5b-4389-92fc-5f7987cdc5ee");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);

    EXPECT_TRUE(sut != nullptr);
    EXPECT_TRUE(nullptr != sut);
}

TEST_F(UniquePtrTest, CanGetUnderlyingPtrFromConstUniquePtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "75727c11-f721-4a52-816a-a9a3a61e2b43");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    const auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
    EXPECT_TRUE(sut.get() != nullptr);
}

TEST_F(UniquePtrTest, CanUseArrowOperatorToAccessObjectInConstUniquePtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "045a9026-74f5-41ad-9881-14c2502527c4");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    const auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
    EXPECT_EQ(X_POS, sut->x);
    EXPECT_EQ(Y_POS, sut->y);
    EXPECT_EQ(Z_POS, sut->z);
}

TEST_F(UniquePtrTest, AssigningUniquePtrToNullptrDeletesTheManagedObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "42821e13-c28c-4274-9f89-10ab342bf372");
    // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    auto* object = new Position();
    auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
    sut = nullptr;
    EXPECT_TRUE(m_deleterCalled);
}

TEST_F(UniquePtrTest, AssigningUniquePtrToNullptrSetsUnderlyingObjectToNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "eacf4bf4-0fa8-42dd-b0a7-c343a1959282");
    EXPECT_DEATH(
        {
            // NOLINTJUSTIFICATION no memory leak, object is deleted in the dtor deleter callback
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
            auto* object = new Position();
            auto sut = iox::cxx::unique_ptr<Position>(object, deleter);
            sut = nullptr;
            EXPECT_EQ(nullptr, sut.get());
        },
        ".*");
}
} // namespace
