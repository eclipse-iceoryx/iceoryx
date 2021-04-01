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

namespace c2cpp
{
iox::popo::SubscriberEvent subscriberEvent(const iox_SubscriberEvent value) noexcept
{
    switch (value)
    {
    case SubscriberEvent_DATA_RECEIVED:
        return iox::popo::SubscriberEvent::DATA_RECEIVED;
    }

    iox::LogFatal() << "invalid iox_SubscriberEvent value";
    errorHandler(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE);
    return iox::popo::SubscriberEvent::DATA_RECEIVED;
}

iox::popo::SubscriberState subscriberState(const iox_SubscriberState value) noexcept
{
    switch (value)
    {
    case SubscriberState_HAS_DATA:
        return iox::popo::SubscriberState::HAS_DATA;
    }

    iox::LogFatal() << "invalid iox_SubscriberState value";
    errorHandler(iox::Error::kBINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE);
    return iox::popo::SubscriberState::HAS_DATA;
}

} // namespace c2cpp
