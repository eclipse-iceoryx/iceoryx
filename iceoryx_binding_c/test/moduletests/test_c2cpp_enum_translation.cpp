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

TEST(c2cpp_enum_translation_test, SubscriberState)
{
    EXPECT_EQ(c2cpp::subscriberState(SubscriberState_HAS_DATA), iox::popo::SubscriberState::HAS_DATA);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    bool hasTerminated = false;
    iox::Error error = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error e, const std::function<void()>&, const iox::ErrorLevel) {
            hasTerminated = true;
            error = e;
        });
    EXPECT_EQ(c2cpp::subscriberState(static_cast<iox_SubscriberState>(-1)), iox::popo::SubscriberState::HAS_DATA);
    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(error, Eq(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE));
#pragma GCC diagnostic pop
}

TEST(c2cpp_enum_translation_test, SubscriberEvent)
{
    EXPECT_EQ(c2cpp::subscriberEvent(SubscriberEvent_DATA_RECEIVED), iox::popo::SubscriberEvent::DATA_RECEIVED);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    bool hasTerminated = false;
    iox::Error error = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error e, const std::function<void()>&, const iox::ErrorLevel) {
            hasTerminated = true;
            error = e;
        });
    EXPECT_EQ(c2cpp::subscriberEvent(static_cast<iox_SubscriberEvent>(-1)), iox::popo::SubscriberEvent::DATA_RECEIVED);
    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(error, Eq(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE));
#pragma GCC diagnostic pop
}
