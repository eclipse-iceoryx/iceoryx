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

#include "test.hpp"

#include "iceoryx_posh/internal/roudi/roudi_lock.hpp"

using namespace ::testing;
using ::testing::Return;


class RouDiLock_test : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};

    iox::roudi::RouDiLock sut;
};

TEST_F(RouDiLock_test, Lock)
{
}

TEST_F(RouDiLock_test, TryDoubleLock)
{
    EXPECT_DEATH({ iox::roudi::RouDiLock sut2; }, ".*");
}

TEST_F(RouDiLock_test, LockAfterUnlock)
{
    sut.~RouDiLock();

    iox::roudi::RouDiLock sut2;
}
