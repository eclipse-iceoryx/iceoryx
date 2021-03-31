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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"

#include "test.hpp"

using namespace ::testing;

template <typename CPP, typename C>
struct EnumMapping
{
    CPP cpp;
    C c;
};

TEST(c2cpp_enum_translation_test, SubscriberState)
{
    constexpr EnumMapping<iox::popo::SubscriberState, iox_SubscriberState> SUBSCRIBER_STATES[]{
        {iox::popo::SubscriberState::HAS_DATA, SubscriberState_HAS_DATA}};

    for (const auto subscriberState : SUBSCRIBER_STATES)
    {
        switch (subscriberState.cpp)
        {
        case iox::popo::SubscriberState::HAS_DATA:
            EXPECT_EQ(c2cpp::subscriberState(subscriberState.c), subscriberState.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
#if 0
    bool hasTerminated = false;
    iox::Error error = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error e, const std::function<void()>&, const iox::ErrorLevel) {
            hasTerminated = true;
            error = e;
        });
    iox_SubscriberState invalidState = iox_SubscriberState::SubscriberState_HAS_DATA;
    int invalidStateValue = -1;
    memcpy(&invalidState, &invalidStateValue, sizeof(int));
    EXPECT_EQ(c2cpp::subscriberState(invalidState), iox::popo::SubscriberState::HAS_DATA);
    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(error, Eq(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE));
#endif
#pragma GCC diagnostic pop
}

TEST(c2cpp_enum_translation_test, SubscriberEvent)
{
    constexpr EnumMapping<iox::popo::SubscriberEvent, iox_SubscriberEvent> SUBSCRIBER_EVENTS[]{
        {iox::popo::SubscriberEvent::DATA_RECEIVED, SubscriberEvent_DATA_RECEIVED}};

    for (const auto subscriberEvent : SUBSCRIBER_EVENTS)
    {
        switch (subscriberEvent.cpp)
        {
        case iox::popo::SubscriberEvent::DATA_RECEIVED:
            EXPECT_EQ(c2cpp::subscriberEvent(subscriberEvent.c), subscriberEvent.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
#if 0
    bool hasTerminated = false;
    iox::Error error = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error e, const std::function<void()>&, const iox::ErrorLevel) {
            hasTerminated = true;
            error = e;
        });
    iox_SubscriberEvent invalidEvent = iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED;
    int invalidEventValue = -1;
    memcpy(&invalidEvent, &invalidEventValue, sizeof(int));
    EXPECT_EQ(c2cpp::subscriberEvent(static_cast<iox_SubscriberEvent>(-1)), iox::popo::SubscriberEvent::DATA_RECEIVED);
    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(error, Eq(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE));
#endif
#pragma GCC diagnostic pop
}
