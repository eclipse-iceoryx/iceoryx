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

#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

class GenericRAII_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(GenericRAII_test, InitFunctionIsCalledInCtorWhenSet)
{
    bool hasCalledInit = false;
    GenericRAII sut([&] { hasCalledInit = true; }, std::function<void()>());
    EXPECT_TRUE(hasCalledInit);
}

TEST_F(GenericRAII_test, InitFunctionIsCalledInCtorWhenSetWithCleanupFunction)
{
    bool hasCalledInit = false;
    bool hasCalledCleanup = false;
    GenericRAII sut([&] { hasCalledInit = true; }, [&] { hasCalledCleanup = true; });
    EXPECT_TRUE(hasCalledInit);
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtor)
{
    bool hasCalledInit = false;
    bool hasCalledCleanup = false;

    {
        GenericRAII sut([&] { hasCalledInit = true; }, [&] { hasCalledCleanup = true; });
    }

    EXPECT_TRUE(hasCalledInit);
    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtorWhenUsingCleanupOnlyCTor)
{
    bool hasCalledCleanup = false;

    {
        GenericRAII sut([&] { hasCalledCleanup = true; });
    }

    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtorWithEmptyInitFunction)
{
    bool hasCalledCleanup = false;

    {
        GenericRAII sut(std::function<void()>(), [&] { hasCalledCleanup = true; });
    }

    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, MoveCTorDoesNotCallCleanupFunctionOfOrigin)
{
    bool hasCalledCleanup = false;

    GenericRAII sut([&] { hasCalledCleanup = true; });
    GenericRAII sut2(std::move(sut));

    EXPECT_FALSE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, MoveConstructedDoesCallCleanupFunctionWhenDestroyed)
{
    bool hasCalledCleanup = false;

    {
        GenericRAII sut([&] { hasCalledCleanup = true; });
        GenericRAII sut2(std::move(sut));
    }

    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, MoveAssignmentCallsCleanup)
{
    bool hasCalledCleanup = false;
    bool hasCalledCleanup2 = false;

    GenericRAII sut([&] { hasCalledCleanup = true; });
    GenericRAII sut2([&] { hasCalledCleanup2 = true; });

    sut = std::move(sut2);

    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, MoveAssignedCallsCleanupWhenOutOfScope)
{
    bool hasCalledCleanup = false;
    bool hasCalledCleanup2 = false;

    {
        GenericRAII sut([&] { hasCalledCleanup = true; });
        GenericRAII sut2([&] { hasCalledCleanup2 = true; });

        sut = std::move(sut2);
    }

    EXPECT_TRUE(hasCalledCleanup);
    EXPECT_TRUE(hasCalledCleanup2);
}
