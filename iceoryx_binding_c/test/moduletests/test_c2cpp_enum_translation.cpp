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

TEST(c2cpp_enum_translation_test, QueueFullPolicy)
{
    EXPECT_EQ(c2cpp::queueFullPolicy(QueueFullPolicy_BLOCK_PUBLISHER), iox::popo::QueueFullPolicy::BLOCK_PUBLISHER);
    EXPECT_EQ(c2cpp::queueFullPolicy(QueueFullPolicy_DISCARD_OLDEST_DATA),
              iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(c2cpp::queueFullPolicy(static_cast<iox_QueueFullPolicy>(-1)),
              iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA);
#pragma GCC diagnostic pop
}

TEST(c2cpp_enum_translation_test, SubscriberTooSlowPolicy)
{
    EXPECT_EQ(c2cpp::subscriberTooSlowPolicy(SubscriberTooSlowPolicy_WAIT_FOR_SUBSCRIBER),
              iox::popo::SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER);
    EXPECT_EQ(c2cpp::subscriberTooSlowPolicy(SubscriberTooSlowPolicy_DISCARD_OLDEST_DATA),
              iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(c2cpp::subscriberTooSlowPolicy(static_cast<iox_SubscriberTooSlowPolicy>(-1)),
              iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA);
#pragma GCC diagnostic pop
}

