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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"

namespace c2cpp
{
iox::popo::SubscriberTooSlowPolicy SubscriberTooSlowPolicy(const ENUM iox_SubscriberTooSlowPolicy policy)
{
    switch (policy)
    {
    case SubscriberTooSlowPolicy_WAIT_FOR_SUBSCRIBER:
        return iox::popo::SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER;
    default:
        return iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA;
    }
}

iox::popo::QueueFullPolicy QueueFullPolicy(const ENUM iox_QueueFullPolicy policy)
{
    switch (policy)
    {
    case QueueFullPolicy_BLOCK_PUBLISHER:
        return iox::popo::QueueFullPolicy::BLOCK_PUBLISHER;
    default:
        return iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA;
    }
}
} // namespace c2cpp

