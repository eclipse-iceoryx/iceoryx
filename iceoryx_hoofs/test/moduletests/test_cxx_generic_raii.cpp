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

#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "test.hpp"

namespace
{
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
    ::testing::Test::RecordProperty("TEST_ID", "9314e17c-5f02-4e5b-8d46-e324aa2cb88f");
    int hasCalledInit = 0;
    GenericRAII sut([&] { ++hasCalledInit; }, function<void()>());
    EXPECT_THAT(hasCalledInit, Eq(1));
}

TEST_F(GenericRAII_test, InitFunctionIsCalledInCtorWhenSetWithCleanupFunction)
{
    ::testing::Test::RecordProperty("TEST_ID", "22ff682e-e328-4696-8a38-3598365dcc31");
    int hasCalledInit = 0;
    int hasCalledCleanup = 0;
    GenericRAII sut([&] { ++hasCalledInit; }, [&] { ++hasCalledCleanup; });
    EXPECT_THAT(hasCalledInit, Eq(1));
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "110bc888-0433-465f-8324-8b7149524bf7");
    int hasCalledInit = 0;
    int hasCalledCleanup = 0;

    {
        GenericRAII sut([&] { ++hasCalledInit; }, [&] { ++hasCalledCleanup; });
    }

    EXPECT_THAT(hasCalledInit, Eq(1));
    EXPECT_THAT(hasCalledCleanup, Eq(1));
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtorWhenUsingCleanupOnlyCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "74fbd0d6-c69f-4951-a193-e30c37d0d1bd");
    bool hasCalledCleanup = false;

    {
        GenericRAII sut([&] { hasCalledCleanup = true; });
    }

    EXPECT_TRUE(hasCalledCleanup);
}

TEST_F(GenericRAII_test, CleanupFunctionIsCalledInDtorWithEmptyInitFunction)
{
    ::testing::Test::RecordProperty("TEST_ID", "e49f4d86-98e1-4562-81ef-0f672d271111");
    int hasCalledCleanup = 0;

    {
        GenericRAII sut([&] { ++hasCalledCleanup; });
    }

    EXPECT_THAT(hasCalledCleanup, Eq(1));
}

TEST_F(GenericRAII_test, MoveCTorDoesNotCallCleanupFunctionOfOrigin)
{
    ::testing::Test::RecordProperty("TEST_ID", "cdaeb5da-fe45-4139-80bc-18caf32e2364");
    int hasCalledCleanup = 0;

    GenericRAII sut([&] { ++hasCalledCleanup; });
    GenericRAII sut2(std::move(sut));

    EXPECT_THAT(hasCalledCleanup, Eq(0));
}

TEST_F(GenericRAII_test, MoveConstructedDoesCallCleanupFunctionWhenDestroyed)
{
    ::testing::Test::RecordProperty("TEST_ID", "afbf48e1-5868-47a8-8157-d0000c23efc7");
    int hasCalledCleanup = 0;

    {
        iox::cxx::optional<GenericRAII> sut(GenericRAII([&] { ++hasCalledCleanup; }));
        GenericRAII sut2(std::move(*sut));
        sut.reset();
        EXPECT_THAT(hasCalledCleanup, Eq(0));
    }
    EXPECT_THAT(hasCalledCleanup, Eq(1));
}

TEST_F(GenericRAII_test, MoveAssignmentCallsCleanup)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0e596af-569b-41c6-b03f-6f8028272f85");
    int hasCalledCleanup = 0;
    int hasCalledCleanup2 = 0;

    GenericRAII sut([&] { ++hasCalledCleanup; });
    GenericRAII sut2([&] { ++hasCalledCleanup2; });

    sut = std::move(sut2);

    EXPECT_THAT(hasCalledCleanup, Eq(1));
    EXPECT_THAT(hasCalledCleanup2, Eq(0));
}

TEST_F(GenericRAII_test, MoveAssignedCallsCleanupWhenOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f142656-ae86-47f2-a1e1-8ed471543d0e");
    int hasCalledCleanup = 0;
    int hasCalledCleanup2 = 0;

    {
        GenericRAII sut([&] { ++hasCalledCleanup; });
        GenericRAII sut2([&] { ++hasCalledCleanup2; });

        sut = std::move(sut2);
        EXPECT_THAT(hasCalledCleanup, Eq(1));
        EXPECT_THAT(hasCalledCleanup2, Eq(0));
    }

    EXPECT_THAT(hasCalledCleanup2, Eq(1));
}
} // namespace
