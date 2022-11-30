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

#include "iceoryx_hoofs/design_pattern/polymorphic_handler.hpp"
#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"

#include "test.hpp"
#include <iostream>

namespace
{
using namespace ::testing;

struct Interface
{
    virtual ~Interface() = default;

    virtual uint32_t id() = 0;

    void reset()
    {
        value = 0;
    }

    uint32_t value{0};
};

constexpr uint32_t DEFAULT_ID = 73;
constexpr uint32_t ALTERNATE_ID = 21;

struct Default : public Interface
{
    uint32_t id() override
    {
        return DEFAULT_ID;
    }
};

struct Alternate : public Interface
{
    uint32_t id() override
    {
        return ALTERNATE_ID;
    }
};

template <typename T>
using Guard = iox::design_pattern::StaticLifetimeGuard<T>;

// should the handler instances be accessed, they will live at least as
// long as the guard objects
Guard<Default> defaultGuard;
Guard<Alternate> alternateGuard;

// will live at least as long as the corresponding guards
Default& defaultHandler = Guard<Default>::instance();
Alternate& alternateHandler = Guard<Alternate>::instance();

struct Hooks
{
    // to check whether the arguments are used correctly
    static void onSetAfterFinalize(Interface& currentInstance, Interface& newInstance)
    {
        currentInstance.value = currentInstance.id();
        newInstance.value = newInstance.id();
    }
};

using Handler = iox::design_pattern::PolymorphicHandler<Interface, Default, Hooks>;

class PolymorphicHandler_test : public Test
{
  public:
    void SetUp() override
    {
        Handler::reset();
    }

    void TearDown() override
    {
    }
};

TEST_F(PolymorphicHandler_test, handlerIsInitializedWithDefault)
{
    ::testing::Test::RecordProperty("TEST_ID", "41bb4a5e-a916-4a6d-80c4-fed3a3d8d78b");
    auto& handler = Handler::get();

    EXPECT_EQ(handler.id(), DEFAULT_ID);
}

TEST_F(PolymorphicHandler_test, settingAlternateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b2f0cfe-f13c-4fa0-aa93-5ddd4f0904d1");
    auto* prevHandler = Handler::set(alternateGuard);
    auto& handler = Handler::get();

    EXPECT_EQ(handler.id(), ALTERNATE_ID);

    ASSERT_NE(prevHandler, nullptr);
    EXPECT_EQ(prevHandler->id(), DEFAULT_ID);
}

TEST_F(PolymorphicHandler_test, alternatePointsToExternalMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "85ce0e51-a1fe-490c-9012-7d539512ed38");
    Handler::set(alternateGuard);

    auto& handler = Handler::get();
    auto* ptr = static_cast<Interface*>(&alternateHandler);

    EXPECT_EQ(&handler, ptr);
}

TEST_F(PolymorphicHandler_test, explicitlySettingToDefaultWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "32e4d808-c848-4bf9-b878-e163ca825539");
    Handler::set(alternateGuard);
    auto* prevHandler = Handler::set(defaultGuard);

    auto& handler = Handler::get();
    auto* ptr = static_cast<Interface*>(&defaultHandler);

    EXPECT_EQ(&handler, ptr);
    ASSERT_NE(prevHandler, nullptr);
    EXPECT_EQ(prevHandler->id(), ALTERNATE_ID);
}

TEST_F(PolymorphicHandler_test, returnValueOfSetPointsToPreviousInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "96447d94-ea27-4d51-8959-12e7752728ae");
    auto& expectedHandler = Handler::get();

    auto* prevHandler = Handler::set(alternateGuard);

    ASSERT_NE(prevHandler, nullptr);
    EXPECT_EQ(&expectedHandler, prevHandler);
}

TEST_F(PolymorphicHandler_test, resetToDefaultWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef8a99da-22a6-497e-b2ec-bf72cc3ae943");
    Handler::set(alternateGuard);
    auto& prevHandler = Handler::get();
    EXPECT_EQ(prevHandler.id(), ALTERNATE_ID);

    // note that we have to use reset to set it back to the internal default
    Handler::reset();

    auto& handler = Handler::get();
    EXPECT_EQ(handler.id(), DEFAULT_ID);
}

TEST_F(PolymorphicHandler_test, setToCurrentHandlerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "54e22290-a7b4-4552-a18f-953571381d38");

    // change to alternateHandler
    Handler::set(alternateGuard);

    // set to alternateHandler again
    // while this is a useless operation, we cannot forbid it via interface
    auto* prevHandler = Handler::set(alternateGuard);
    auto& handler = Handler::get();

    EXPECT_EQ(&handler, prevHandler);
    EXPECT_EQ(prevHandler, &alternateHandler);
}

TEST_F(PolymorphicHandler_test, settingAfterFinalizeCallsHook)
{
    ::testing::Test::RecordProperty("TEST_ID", "171ac802-01b9-4e08-80a6-6f2defecaf6d");

    auto& handler = Handler::get();

    // reset the handler value to non-zero and check later whether they are set to non-zero as expecteded
    handler.reset();
    alternateHandler.reset();

    // note that all following tests will also call the after finalize
    // hook but we only check if we care whether it was called
    Handler::finalize();
    auto* prevHandler = Handler::set(alternateGuard);
    EXPECT_EQ(prevHandler, nullptr);

    // does the hook set the values to the corresponding arguments?
    EXPECT_EQ(handler.value, DEFAULT_ID);
    EXPECT_EQ(alternateHandler.value, ALTERNATE_ID);
}

TEST_F(PolymorphicHandler_test, resetAfterFinalizeCallsHook)
{
    ::testing::Test::RecordProperty("TEST_ID", "996220e3-7985-4d57-bd3f-844987cf99dc");

    auto& handler = Handler::get();
    handler.reset();
    alternateHandler.reset();

    Handler::finalize();
    Handler::reset();

    EXPECT_EQ(handler.value, DEFAULT_ID);
    EXPECT_EQ(alternateHandler.value, 0);
}

TEST_F(PolymorphicHandler_test, obtainingGuardWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "694f7399-598a-4918-b1e8-4b8546484245");
    EXPECT_EQ(Guard<Handler>::count(), 1);
    Guard<Handler> guard(Handler::guard());
    EXPECT_EQ(Guard<Handler>::count(), 2);
}

} // namespace
