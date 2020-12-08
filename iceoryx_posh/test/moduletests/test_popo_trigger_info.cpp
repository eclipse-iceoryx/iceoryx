// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
#include "iceoryx_posh/popo/trigger_info.hpp"

#include "test.hpp"
#include <thread>

using namespace iox;
using namespace iox::popo;
using namespace ::testing;

class TriggerInfo_test : public Test
{
  public:
    class TriggerOriginTest
    {
      public:
        static void callback(TriggerOriginTest* const origin)
        {
            origin->m_callbackOrigin = origin;
        }

        TriggerOriginTest* m_callbackOrigin = nullptr;
    };

    TriggerInfo_test()
    {
    }

    ~TriggerInfo_test()
    {
    }

    TriggerOriginTest m_origin;
    TriggerOriginTest m_falseOrigin;
    TriggerInfo m_sut{&m_origin, 1478, TriggerOriginTest::callback};
};

TEST_F(TriggerInfo_test, defaultCTorConstructsEmptyTriggerInfo)
{
    int bla;
    TriggerInfo sut;

    EXPECT_EQ(sut.getTriggerId(), TriggerInfo::INVALID_TRIGGER_ID);
    EXPECT_EQ(sut.doesOriginateFrom(&bla), false);
    EXPECT_EQ(sut(), false);
}

TEST_F(TriggerInfo_test, getTriggerIdReturnsValidTriggerId)
{
    EXPECT_EQ(m_sut.getTriggerId(), 1478);
}

TEST_F(TriggerInfo_test, doesOriginateFromStatesOriginCorrectly)
{
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_origin), true);
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_falseOrigin), false);
}

TEST_F(TriggerInfo_test, getOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    EXPECT_EQ(m_sut.getOrigin<TriggerOriginTest>(), &m_origin);
}

TEST_F(TriggerInfo_test, constGetOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    EXPECT_EQ(const_cast<const TriggerInfo&>(m_sut).getOrigin<TriggerOriginTest>(), &m_origin);
}

TEST_F(TriggerInfo_test, getOriginReturnsNullptrWithWrongType)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    m_sut.getOrigin<int>();

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, iox::Error::kPOPO__TRIGGER_STATE_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(TriggerInfo_test, constGetOriginReturnsNullptrWithWrongType)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    const_cast<TriggerInfo&>(m_sut).getOrigin<int>();

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, iox::Error::kPOPO__TRIGGER_STATE_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(TriggerInfo_test, triggerCallbackReturnsTrueAndCallsCallbackWithSettedCallback)
{
    EXPECT_TRUE(m_sut());
    EXPECT_EQ(m_origin.m_callbackOrigin, &m_origin);
}

TEST_F(TriggerInfo_test, triggerCallbackReturnsFalseWithUnsetCallback)
{
    m_sut = TriggerInfo{&m_origin, 9, TriggerInfo::Callback<TriggerOriginTest>()};
    EXPECT_FALSE(m_sut());
}
