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

struct Interface : public iox::design_pattern::Activatable
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
    EXPECT_TRUE(handler.isActive());
}

TEST_F(PolymorphicHandler_test, settingAlternateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b2f0cfe-f13c-4fa0-aa93-5ddd4f0904d1");
    auto* prevHandler = Handler::set(alternateGuard);
    auto& handler = Handler::get();

    EXPECT_EQ(handler.id(), ALTERNATE_ID);
    EXPECT_TRUE(handler.isActive());

    ASSERT_NE(prevHandler, nullptr);
    EXPECT_EQ(prevHandler->id(), DEFAULT_ID);
    EXPECT_FALSE(prevHandler->isActive());
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
    EXPECT_FALSE(prevHandler->isActive());
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
    EXPECT_TRUE(handler.isActive());

    EXPECT_FALSE(prevHandler.isActive());
}

TEST_F(PolymorphicHandler_test, setToCurrentHandlerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "54e22290-a7b4-4552-a18f-953571381d38");

    // change to alternateHandler
    Handler::set(alternateGuard);
    EXPECT_TRUE(alternateHandler.isActive());

    // set to alternateHandler again, should stay active
    // while this is a useless operation, we cannot forbid it via interface
    auto* prevHandler = Handler::set(alternateGuard);
    auto& handler = Handler::get();

    EXPECT_EQ(&handler, prevHandler);
    EXPECT_EQ(prevHandler, &alternateHandler);
    EXPECT_TRUE(handler.isActive());
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

class Activatable_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    iox::design_pattern::Activatable sut;
};

TEST_F(Activatable_test, isActiveAfterConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "874b600a-7976-4c97-a800-55bac11c4eaa");
    EXPECT_TRUE(sut.isActive());
}

TEST_F(Activatable_test, copyCtorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8e6b2c7-a8bf-441d-8066-66096329b21f");
    {
        auto copy(sut);
        EXPECT_TRUE(copy.isActive());
    }

    {
        sut.deactivate();
        auto copy(sut);
        EXPECT_FALSE(copy.isActive());
    }
}

TEST_F(Activatable_test, copyAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "241ad501-2295-4da7-accd-50872264997d");
    iox::design_pattern::Activatable other;

    sut.deactivate();
    EXPECT_FALSE(sut.isActive());
    sut = other;
    EXPECT_TRUE(sut.isActive());
    other.deactivate();
    sut = other;
    EXPECT_FALSE(sut.isActive());
}

TEST_F(Activatable_test, isNotActiveAfterDeactivate)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9f052b1-33dd-4e71-9887-26581d219492");
    sut.deactivate();
    EXPECT_FALSE(sut.isActive());
}

TEST_F(Activatable_test, isNotActiveAfterMultiDeactivate)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ab19dd3-83a4-4e95-a4d2-3c9d973ab28b");
    sut.deactivate();
    sut.deactivate();
    EXPECT_FALSE(sut.isActive());
}

TEST_F(Activatable_test, isActiveAfterReactivation)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec26ea62-d979-4f28-89a2-59d4639b52b2");
    sut.deactivate();
    sut.activate();
    EXPECT_TRUE(sut.isActive());
}

TEST_F(Activatable_test, isActiveAfterMultiActivation)
{
    ::testing::Test::RecordProperty("TEST_ID", "5593d002-394b-4e30-908c-d56d9b56c58e");
    sut.activate();
    EXPECT_TRUE(sut.isActive());

    sut.deactivate();
    sut.activate();
    sut.activate();
    EXPECT_TRUE(sut.isActive());
}

} // namespace
