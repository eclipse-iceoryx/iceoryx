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

#include "iceoryx_hoofs/testing/lifetime_and_assignment_tracker.hpp"
#include "iox/move_and_copy_helper.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

struct MoveAndCopyHelper_test : public Test
{
    using DataType = uint64_t;
    static constexpr DataType DEFAULT_VALUE{10};
    static constexpr DataType EMPTY_VALUE{0};

    using MoveOnlySut = MoveOnlyLifetimeAndAssignmentTracker<DataType, 0>;
    using MoveCopyableSut = LifetimeAndAssignmentTracker<DataType, 0>;

    using Operation = iox::MoveAndCopyOperations;
    using CopyCtorHelper = iox::MoveAndCopyHelper<Operation::CopyConstructor>;
    using MoveCtorHelper = iox::MoveAndCopyHelper<Operation::MoveConstructor>;
    using CopyAssignmentHelper = iox::MoveAndCopyHelper<Operation::CopyAssignment>;
    using MoveAssignmentHelper = iox::MoveAndCopyHelper<Operation::MoveAssignment>;

    void SetUp() override
    {
        sut.value = DEFAULT_VALUE;
        move_only_sut.value = DEFAULT_VALUE;

        stats.reset();
        move_only_stats.reset();
    }

    MoveCopyableSut sut{DEFAULT_VALUE};
    MoveCopyableSut::Statistics& stats = MoveCopyableSut::stats;

    MoveOnlySut move_only_sut{DEFAULT_VALUE};
    MoveOnlySut::Statistics& move_only_stats = MoveOnlySut::move_only_stats;
};

// BEGIN test move_or_copy for MoveCopyableSut

TEST_F(MoveAndCopyHelper_test, CopyCtorHelperCanApplyOnMoveCopyableSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "23308260-7a28-4169-8816-3be9e4ef965f");

    MoveCopyableSut copy_sut{CopyCtorHelper::move_or_copy(sut)};

    EXPECT_THAT(stats.copyCTor, 1U);
    EXPECT_THAT(stats.moveCTor, 0U);
    EXPECT_THAT(stats.copyAssignment, 0U);
    EXPECT_THAT(stats.moveAssignment, 0U);

    EXPECT_THAT(sut.value, DEFAULT_VALUE);
    EXPECT_THAT(copy_sut.value, DEFAULT_VALUE);
}

TEST_F(MoveAndCopyHelper_test, CopyAssignmentHelperCanApplyOnMoveCopyableSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "a25b15ad-ef66-4cd9-a9f8-3da9a15fa7fc");

    MoveCopyableSut copy_sut{EMPTY_VALUE};
    copy_sut = CopyAssignmentHelper::move_or_copy(sut);

    EXPECT_THAT(stats.copyCTor, 0U);
    EXPECT_THAT(stats.moveCTor, 0U);
    EXPECT_THAT(stats.copyAssignment, 1U);
    EXPECT_THAT(stats.moveAssignment, 0U);

    EXPECT_THAT(sut.value, DEFAULT_VALUE);
    EXPECT_THAT(copy_sut.value, DEFAULT_VALUE);
}

TEST_F(MoveAndCopyHelper_test, MoveCtorHelperCanApplyOnMoveCopyableSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2aa6625-0483-447d-8aa1-4de9cd53c91e");

    MoveCopyableSut move_sut{MoveCtorHelper::move_or_copy(sut)};

    EXPECT_THAT(stats.copyCTor, 0U);
    EXPECT_THAT(stats.moveCTor, 1U);
    EXPECT_THAT(stats.copyAssignment, 0U);
    EXPECT_THAT(stats.moveAssignment, 0U);

    EXPECT_THAT(move_sut.value, DEFAULT_VALUE);
}

TEST_F(MoveAndCopyHelper_test, MoveAssignmentHelperCanApplyOnMoveCopyableSut)
{
    ::testing::Test::RecordProperty("TEST_ID", "5945174f-739c-4cb2-a485-4473baaf52e4");

    MoveCopyableSut move_sut{EMPTY_VALUE};
    move_sut = MoveAssignmentHelper::move_or_copy(sut);

    EXPECT_THAT(stats.copyCTor, 0U);
    EXPECT_THAT(stats.moveCTor, 0U);
    EXPECT_THAT(stats.copyAssignment, 0U);
    EXPECT_THAT(stats.moveAssignment, 1U);

    EXPECT_THAT(move_sut.value, DEFAULT_VALUE);
}

// END test move_or_copy for MoveCopyableSut


// BEGIN test move_or_copy for MoveOnlySut

TEST_F(MoveAndCopyHelper_test, MoveCtorHelperCanApplyOnMoveOnlySut)
{
    ::testing::Test::RecordProperty("TEST_ID", "54f03825-4fa2-4bb4-89b9-aafd7ddfc420");

    MoveOnlySut moved_move_only_sut{MoveCtorHelper::move_or_copy(move_only_sut)};

    EXPECT_THAT(move_only_stats.copyCTor, 0U);
    EXPECT_THAT(move_only_stats.moveCTor, 1U);
    EXPECT_THAT(move_only_stats.copyAssignment, 0U);
    EXPECT_THAT(move_only_stats.moveAssignment, 0U);

    EXPECT_THAT(moved_move_only_sut.value, DEFAULT_VALUE);
}

TEST_F(MoveAndCopyHelper_test, MoveAssignmentHelperCanApplyOnMoveOnlySut)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c70e8d5-d5b1-4d8e-83a6-d68c2ede89a0");

    MoveOnlySut moved_move_only_sut{EMPTY_VALUE};
    moved_move_only_sut = MoveCtorHelper::move_or_copy(move_only_sut);

    EXPECT_THAT(move_only_stats.copyCTor, 0U);
    EXPECT_THAT(move_only_stats.moveCTor, 0U);
    EXPECT_THAT(move_only_stats.copyAssignment, 0U);
    EXPECT_THAT(move_only_stats.moveAssignment, 1U);

    EXPECT_THAT(moved_move_only_sut.value, DEFAULT_VALUE);
}

// END test move_or_copy for MoveOnlySut

} // namespace