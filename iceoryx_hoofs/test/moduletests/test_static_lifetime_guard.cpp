// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"

#include "test.hpp"
#include <iostream>

namespace
{
using namespace ::testing;

struct Foo
{
    Foo()
        : id(++instancesCreated)
    {
        ++ctorCalled;
    }

    ~Foo()
    {
        ++dtorCalled;
    }

    static void reset()
    {
        ctorCalled = 0;
        dtorCalled = 0;
    }

    static uint32_t ctorCalled;
    static uint32_t dtorCalled;
    static uint32_t instancesCreated;

    uint32_t id;
};

uint32_t Foo::ctorCalled{0};
uint32_t Foo::dtorCalled{0};
uint32_t Foo::instancesCreated{0};

using Guard = iox::design_pattern::StaticLifetimeGuard<Foo>;

// the first call to instance() creates a static instance,
// g_instance is guarded once implicitly
Foo& g_instance = Guard::instance();

class StaticLifetimeGuard_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// we rely on test order here

TEST_F(StaticLifetimeGuard_test, staticInitializationSucceeded)
{
    ::testing::Test::RecordProperty("TEST_ID", "d38b436b-f079-43fe-9d33-23d18cd08ffc");

    // g_instance was constructed and the instance still exists
    EXPECT_EQ(g_instance.id, 1);
    EXPECT_EQ(Guard::count(), 1);
    EXPECT_EQ(Foo::ctorCalled, 1);
    EXPECT_EQ(Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, setCountWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1db790f9-d49e-44b2-b7e9-af50dd6a7d67");

    auto oldCount = Guard::setCount(73);
    EXPECT_EQ(Guard::count(), 73);
    EXPECT_EQ(oldCount, 1);

    Guard::setCount(oldCount);
    EXPECT_EQ(Guard::count(), oldCount);
}

TEST_F(StaticLifetimeGuard_test, guardPreventsDestruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a8c5953-f2d7-4539-89ba-b4686bbb6319");
    Foo::reset();
    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 0);
    EXPECT_EQ(g_instance.id, 1);
    {
        Guard guard;
        EXPECT_EQ(Guard::count(), 2);
        auto& instance = Guard::instance();

        EXPECT_EQ(Foo::ctorCalled, 0);
        EXPECT_EQ(Foo::dtorCalled, 0);

        // still the same instance as g_instance
        EXPECT_EQ(instance.id, 1);
        EXPECT_EQ(&instance, &g_instance);
    }

    // the implicit guard of g_instance prevents destruction
    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 0);
    EXPECT_EQ(g_instance.id, 1);
}

TEST_F(StaticLifetimeGuard_test, copyIncreasesLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ab6396d-7c63-4626-92ed-c7f3ea67bbf1");
    Foo::reset();
    Guard guard;
    {
        EXPECT_EQ(Guard::count(), 2);
        // NOLINTJUSTIFICATION ctor and dtor side effects are tested
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        Guard copy(guard);
        EXPECT_EQ(Guard::count(), 3);
    }
    EXPECT_EQ(Guard::count(), 2);

    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, moveIncreasesLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "32a2fdbf-cb02-408c-99a3-373aa66b2764");
    Foo::reset();
    Guard guard;
    {
        EXPECT_EQ(Guard::count(), 2);
        // NOLINTJUSTIFICATION ctor and dtor side effects are tested
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        Guard movedGuard(std::move(guard));
        EXPECT_EQ(Guard::count(), 3);
    }
    EXPECT_EQ(Guard::count(), 2);

    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, assignmentDoesNotChangeLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c04ac75-d47a-44da-b8dc-6f567a53d3fc");
    Foo::reset();
    Guard guard1;
    Guard guard2;

    EXPECT_EQ(Guard::count(), 3);
    guard1 = guard2;
    EXPECT_EQ(Guard::count(), 3);
    guard1 = std::move(guard2);
    EXPECT_EQ(Guard::count(), 3);

    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, destructionAtZeroCountWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b5a22a9-87bc-434b-9d07-9f3c20a6944e");
    Foo::reset();
    {
        Guard guard;
        auto& instance = Guard::instance();

        // count is expected to be 2,
        // we ignore the guard of g_instance by setting it to 1,
        // hence when guard is destroyed the instance will be destroyed as well
        auto oldCount = Guard::setCount(1);
        EXPECT_EQ(oldCount, 2);

        EXPECT_EQ(Foo::ctorCalled, 0);
        EXPECT_EQ(Foo::dtorCalled, 0);
        EXPECT_EQ(instance.id, 1);
    }

    EXPECT_EQ(Guard::count(), 0);
    EXPECT_EQ(Foo::ctorCalled, 0);
    EXPECT_EQ(Foo::dtorCalled, 1);
}

TEST_F(StaticLifetimeGuard_test, constructionAfterDestructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0077e73d-ddf5-47e7-a7c6-93819f376175");

    // ensure that the old instance is destroyed if it exists
    if (Guard::count() > 0)
    {
        Guard guard;
        Guard::setCount(1);
        // now the instance will be destroyed once the guard is destroyed
    }

    Foo::reset();
    EXPECT_EQ(Guard::count(), 0);
    {
        Guard guard;
        auto& instance = Guard::instance();

        EXPECT_EQ(Foo::ctorCalled, 1);
        EXPECT_EQ(Foo::dtorCalled, 0);
        EXPECT_EQ(instance.id, 2);
    }

    EXPECT_EQ(Guard::count(), 0);
    EXPECT_EQ(Foo::ctorCalled, 1);
    EXPECT_EQ(Foo::dtorCalled, 1);
}

} // namespace
