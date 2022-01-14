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

#include "iceoryx_hoofs/internal/cxx/reference_counter.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;

class ReferenceCounter_test : public Test
{
  public:
    uint64_t var{0};
    ReferenceCounter<uint64_t> sut{&var};
};

TEST_F(ReferenceCounter_test, CTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2fec1f5-7c59-40ef-9893-6b1f9aa2c56d");
    EXPECT_THAT(var, Eq(1));
}

TEST_F(ReferenceCounter_test, CopyCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "746fdfe6-80bb-4429-9bc2-2999a31d0891");
    ReferenceCounter<uint64_t> sut2(sut);
    EXPECT_THAT(var, Eq(2));
}

TEST_F(ReferenceCounter_test, MoveCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6146408-308a-44a2-bf52-3b807d36f6c4");
    ReferenceCounter<uint64_t> sut2(std::move(sut));
    EXPECT_THAT(var, Eq(1));
}

TEST_F(ReferenceCounter_test, CopyAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2117274-7550-4c7c-9f17-6bcf9d640acd");
    uint64_t var2{0};
    ReferenceCounter<uint64_t> sut2(&var);
    sut2 = sut;

    EXPECT_THAT(var, Eq(2));
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, MoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "b728ba0f-bb26-4146-a7ec-076f90426711");
    uint64_t var2{0};
    ReferenceCounter<uint64_t> sut2(&var);
    sut2 = std::move(sut);

    EXPECT_THAT(var, Eq(1));
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, Destructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "2eb96773-4543-4d73-9cfb-e4e456470d57");
    uint64_t var2{0};
    {
        ReferenceCounter<uint64_t> sut2(&var2);
    }
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, DestructorAfterCopyCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e0f8ecf-2380-4611-bd3a-29a881d38e87");
    uint64_t var2{0};
    {
        ReferenceCounter<uint64_t> sut2(&var2);
        ReferenceCounter<uint64_t> sut3(sut2);
    }
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, DestructorAfterMoveCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "1972f5f0-43bb-45da-b843-82f10a62e84a");
    uint64_t var2{0};
    {
        ReferenceCounter<uint64_t> sut2(&var2);
        ReferenceCounter<uint64_t> sut3(std::move(sut2));
    }
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, DestructorAfterCopyAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d44de2b-44be-4a67-8d92-9249122d6df7");
    uint64_t var2{0};
    {
        uint64_t var3{0};
        ReferenceCounter<uint64_t> sut2(&var2);
        {
            ReferenceCounter<uint64_t> sut3(&var3);
            sut3 = sut2;
            EXPECT_THAT(var2, Eq(2));
        }
        EXPECT_THAT(var2, Eq(1));
    }
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, DestructorAfterMoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "197d9f84-0fb3-49d0-8041-3e02b307800e");
    uint64_t var2{0};
    {
        uint64_t var3{0};
        ReferenceCounter<uint64_t> sut2(&var2);
        {
            ReferenceCounter<uint64_t> sut3(&var3);
            sut3 = std::move(sut2);
            EXPECT_THAT(var2, Eq(1));
        }
        EXPECT_THAT(var2, Eq(0));
    }
    EXPECT_THAT(var2, Eq(0));
}

TEST_F(ReferenceCounter_test, GetValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "615a8d5d-39a5-4003-be32-d8e7d3c94baa");
    EXPECT_THAT(sut.getValue(), Eq(1));
}
} // namespace
