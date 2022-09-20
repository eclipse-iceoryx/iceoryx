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

#include "test.hpp"
#include <iostream>

namespace
{
using namespace ::testing;

struct Interface : public iox::design::Activatable
{
    virtual ~Interface() = default;

    virtual uint32_t id() = 0;
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

Default& defaultHandler()
{
    static Default h;
    return h;
};

Alternate& alternateHandler()
{
    static Alternate h;
    return h;
};

using Handler = iox::design::PolymorphicHandler<Interface, Default>;

class PolymorphicHandler_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
        Handler::reset();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(PolymorphicHandler_test, handlerIsInitializedWithDefault)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    auto& handler = Handler::get();

    EXPECT_EQ(handler.id(), DEFAULT_ID);
}

TEST_F(PolymorphicHandler_test, settingAlternateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    Handler::set(alternateHandler());
    auto& handler = Handler::get();

    EXPECT_EQ(handler.id(), ALTERNATE_ID);
}

TEST_F(PolymorphicHandler_test, alternatePointsToExternalMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    Handler::set(alternateHandler());

    auto& handler = Handler::get();
    auto* ptr = static_cast<Interface*>(&alternateHandler());

    EXPECT_EQ(&handler, ptr);
}

TEST_F(PolymorphicHandler_test, explicitlySettingToDefaultWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    Handler::set(alternateHandler());
    Handler::set(defaultHandler());

    auto& handler = Handler::get();
    auto* ptr = static_cast<Interface*>(&defaultHandler());

    EXPECT_EQ(&handler, ptr);
}

TEST_F(PolymorphicHandler_test, resetToDefaultWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    Handler::set(alternateHandler());
    Handler::reset();

    auto& handler = Handler::get();
    EXPECT_EQ(handler.id(), DEFAULT_ID);
}

// TODO: death tests abort (regardless of finalize), why?
// how does the fork affect statics?
#if 0
TEST_F(PolymorphicHandler_test, settingAfterFinalizeTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");

    auto f = [&]() {
        Handler::finalize();
        Handler::set(alternateHandler());
    };

    EXPECT_DEATH(f(), "setting the polymorphic handler after finalize is not allowed");
}

TEST_F(PolymorphicHandler_test, resetAfterFinalizeTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");

    auto f = [&]() {
        Handler::finalize();
        Handler::reset();
    };

    EXPECT_DEATH(f(), "setting the polymorphic handler after finalize is not allowed");
}
#endif

class Activatable_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    iox::design::Activatable sut;
};

TEST_F(Activatable_test, isActiveAfterConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    EXPECT_TRUE(sut.isActive());
}

TEST_F(Activatable_test, isNotActiveAfterDeactivate)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    sut.deactivate();
    EXPECT_FALSE(sut.isActive());
}

TEST_F(Activatable_test, isNotActiveAfterMultiDeactivate)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    sut.deactivate();
    sut.deactivate();
    EXPECT_FALSE(sut.isActive());
}

TEST_F(Activatable_test, isActiveAfterReactivation)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    sut.deactivate();
    sut.activate();
    EXPECT_TRUE(sut.isActive());
}

TEST_F(Activatable_test, isActiveAfterMultiActivation)
{
    ::testing::Test::RecordProperty("TEST_ID", "123");
    sut.activate();
    EXPECT_TRUE(sut.isActive());

    sut.deactivate();
    sut.activate();
    sut.activate();
    EXPECT_TRUE(sut.isActive());
}

} // namespace
