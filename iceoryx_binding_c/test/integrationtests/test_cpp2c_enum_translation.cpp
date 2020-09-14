// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"

#include "test.hpp"

using namespace ::testing;

TEST(cpp2c_enum_translation_test, SubscribeState)
{
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::NOT_SUBSCRIBED), SubscribeState_NOT_SUBSCRIBED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::SUBSCRIBE_REQUESTED), SubscribeState_SUBSCRIBE_REQUESTED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::SUBSCRIBED), SubscribeState_SUBSCRIBED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::UNSUBSCRIBE_REQUESTED), SubscribeState_UNSUBSCRIBE_REQUESTED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::WAIT_FOR_OFFER), SubscribeState_WAIT_FOR_OFFER);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::SubscribeState(static_cast<iox::SubscribeState>(-1)), SubscribeState_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}
