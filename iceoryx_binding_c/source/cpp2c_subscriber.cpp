// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

cpp2c_Subscriber::cpp2c_Subscriber(cpp2c_Subscriber&& rhs) noexcept
{
    *this = std::move(rhs);
}

cpp2c_Subscriber& cpp2c_Subscriber::operator=(cpp2c_Subscriber&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (m_portData)
        {
            iox::popo::SubscriberPortUser(m_portData).destroy();
        }

        m_trigger = std::move(rhs.m_trigger);
        m_portData = rhs.m_portData;
        rhs.m_portData = nullptr;
    }
    return *this;
}

iox_WaitSetResult
cpp2c_Subscriber::attachToWaitset(iox::popo::WaitSet& waitset,
                                  const iox_SubscriberEvent subscriberEvent,
                                  const uint64_t triggerId,
                                  const iox::popo::Trigger::Callback<cpp2c_Subscriber> callback) noexcept
{
    static_cast<void>(subscriberEvent);

    auto result = std::move(
        waitset
            .acquireTrigger(this,
                            {this, &cpp2c_Subscriber::hasNewSamples},
                            {this, &cpp2c_Subscriber::unsetTrigger},
                            triggerId,
                            callback)
            .and_then([this](iox::popo::Trigger& trigger) {
                m_trigger = std::move(trigger);
                iox::popo::SubscriberPortUser(m_portData).setConditionVariable(m_trigger.getConditionVariableData());
            }));

    return (result.has_error()) ? cpp2c::WaitSetResult(result.get_error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void cpp2c_Subscriber::detachWaitset() noexcept
{
    m_trigger.reset();
}

void cpp2c_Subscriber::unsetTrigger(const iox::popo::Trigger& trigger) noexcept
{
    if (trigger.isLogicalEqualTo(m_trigger))
    {
        iox::popo::SubscriberPortUser(m_portData).unsetConditionVariable();
        m_trigger.reset();
    }
}

bool cpp2c_Subscriber::hasNewSamples() const noexcept
{
    return iox::popo::SubscriberPortUser(m_portData).hasNewChunks();
}

