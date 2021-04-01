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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/event_info.hpp"

#include "test.hpp"
#include <thread>

using namespace iox;
using namespace iox::popo;
using namespace ::testing;

class EventInfo_test : public Test
{
  public:
    class EventOriginTest
    {
      public:
        static void callback(EventOriginTest* const origin)
        {
            origin->m_callbackOrigin = origin;
        }

        EventOriginTest* m_callbackOrigin = nullptr;
    };

    EventInfo_test()
    {
    }

    ~EventInfo_test()
    {
    }

    EventOriginTest m_origin;
    EventOriginTest m_falseOrigin;
    EventInfo m_sut{&m_origin, 1478U, EventOriginTest::callback};
};

TEST_F(EventInfo_test, defaultCTorConstructsEmptyEventInfo)
{
    int bla;
    EventInfo sut;

    EXPECT_EQ(sut.getEventId(), EventInfo::INVALID_ID);
    EXPECT_EQ(sut.doesOriginateFrom(&bla), false);
    EXPECT_EQ(sut(), false);
}

TEST_F(EventInfo_test, getEventIdReturnsValidEventId)
{
    EXPECT_EQ(m_sut.getEventId(), 1478U);
}

TEST_F(EventInfo_test, doesOriginateFromStatesOriginCorrectly)
{
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_origin), true);
    EXPECT_EQ(m_sut.doesOriginateFrom(&m_falseOrigin), false);
}

TEST_F(EventInfo_test, getOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    EXPECT_EQ(m_sut.getOrigin<EventOriginTest>(), &m_origin);
}

TEST_F(EventInfo_test, constGetOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    EXPECT_EQ(const_cast<const EventInfo&>(m_sut).getOrigin<EventOriginTest>(), &m_origin);
}

TEST_F(EventInfo_test, getOriginReturnsNullptrWithWrongType)
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
    EXPECT_EQ(errorHandlerType, iox::Error::kPOPO__EVENT_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(EventInfo_test, constGetOriginReturnsNullptrWithWrongType)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    const_cast<EventInfo&>(m_sut).getOrigin<int>();

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, iox::Error::kPOPO__EVENT_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN);
}

TEST_F(EventInfo_test, triggerCallbackReturnsTrueAndCallsCallbackWithSettedCallback)
{
    EXPECT_TRUE(m_sut());
    EXPECT_EQ(m_origin.m_callbackOrigin, &m_origin);
}

TEST_F(EventInfo_test, triggerCallbackReturnsFalseWithUnsetCallback)
{
    m_sut = EventInfo{&m_origin, 9U, EventInfo::Callback<EventOriginTest>()};
    EXPECT_FALSE(m_sut());
}
