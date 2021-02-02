// Copyright (c) 2020, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"

cpp2c_Subscriber::~cpp2c_Subscriber()
{
    if (m_portData)
    {
        iox::popo::SubscriberPortUser(m_portData).destroy();
    }
}

void cpp2c_Subscriber::enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                                   const iox_SubscriberEvent subscriberEvent) noexcept
{
    static_cast<void>(subscriberEvent);

    m_trigger = std::move(triggerHandle);
    iox::popo::SubscriberPortUser(m_portData).setConditionVariable(m_trigger.getConditionVariableData());
}

iox::popo::WaitSetHasTriggeredCallback
cpp2c_Subscriber::getHasTriggeredCallbackForEvent(const iox_SubscriberEvent subscriberEvent) const noexcept
{
    switch (subscriberEvent)
    {
    case SubscriberEvent_HAS_SAMPLES:
        return {*this, &cpp2c_Subscriber::hasSamples};
    }

    return {};
}

void cpp2c_Subscriber::disableEvent(const iox_SubscriberEvent subscriberEvent) noexcept
{
    static_cast<void>(subscriberEvent);

    m_trigger.reset();
}

void cpp2c_Subscriber::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        iox::popo::SubscriberPortUser(m_portData).unsetConditionVariable();
        m_trigger.invalidate();
    }
}

bool cpp2c_Subscriber::hasSamples() const noexcept
{
    return iox::popo::SubscriberPortUser(m_portData).hasNewChunks();
}

