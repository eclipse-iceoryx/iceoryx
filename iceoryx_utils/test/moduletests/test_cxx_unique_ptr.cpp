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

#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "test.hpp"

#include <iostream>

using namespace ::testing;

struct Position
{
    double_t x = 0.0;
    double_t y = 0.0;
    double_t z = 0.0;
};

class UniquePtrTest : public Test
{
  public:
    UniquePtrTest()
    {
    }

    void SetUp()
    {
        m_deleterCalled = false;
    }

    void TearDown()
    {
    }

    bool m_deleterCalled;

    std::function<void(Position* const)> deleter = [this](Position* const p) {
        m_deleterCalled = true;
        delete p;
    };
};


TEST_F(UniquePtrTest, CtorWithNullptrSetsPtrToNull)
{
    auto ptr = iox::cxx::unique_ptr<Position>(nullptr);
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST_F(UniquePtrTest, CtorWithOnlyDeleterSetsPtrToNullAndDoesntCallDeleter)
{
    {
        auto ptr = iox::cxx::unique_ptr<Position>(deleter);
        EXPECT_EQ(ptr.get(), nullptr);
    }

    EXPECT_FALSE(m_deleterCalled);
}


TEST_F(UniquePtrTest, CtorWithObjectPtrAndDeleterSetsPtrToObjectAndCallsDeleter)
{
    {
        auto object = new Position();
        auto ptr = iox::cxx::unique_ptr<Position>(object, deleter);
        EXPECT_EQ(ptr.get(), object);
    }

    EXPECT_TRUE(m_deleterCalled);
}

TEST_F(UniquePtrTest, DeleterIsProperlySet)
{
}

TEST_F(UniquePtrTest, DeleterNotCalledOnReleasedPointers)
{
}

TEST_F(UniquePtrTest, DeleterNotCalledOnNullptrs)
{
}

TEST_F(UniquePtrTest, CanResetToNullptr)
{
}

TEST_F(UniquePtrTest, CanResetToAnExistingRawPtr)
{
}
