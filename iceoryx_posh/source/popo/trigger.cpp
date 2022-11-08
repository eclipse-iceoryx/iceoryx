// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/trigger.hpp"

namespace iox
{
namespace popo
{
constexpr uint64_t Trigger::INVALID_TRIGGER_ID;

Trigger::~Trigger() noexcept
{
    reset();
}

bool Trigger::isStateConditionSatisfied() const noexcept
{
    switch (getTriggerType())
    {
    case TriggerType::STATE_BASED:
        return (isValid()) ? m_hasTriggeredCallback() : false;
    case TriggerType::EVENT_BASED:
        return isValid();
    case TriggerType::INVALID:
        return false;
    };
    return false;
}

void Trigger::reset() noexcept
{
    if (!isValid())
    {
        return;
    }

    m_resetCallback(m_uniqueId);

    invalidate();
}

const NotificationInfo& Trigger::getNotificationInfo() const noexcept
{
    return m_notificationInfo;
}

void Trigger::invalidate() noexcept
{
    m_hasTriggeredCallback = [] { return false; };
    m_resetCallback = [](auto) {};
    m_uniqueId = INVALID_TRIGGER_ID;
    m_triggerType = TriggerType::INVALID;
    m_originTriggerType = INVALID_TRIGGER_ID;
    m_originTriggerTypeHash = INVALID_TRIGGER_ID;
}

Trigger::operator bool() const noexcept
{
    return isValid();
}

bool Trigger::isValid() const noexcept
{
    return m_uniqueId != INVALID_TRIGGER_ID;
}

bool Trigger::isLogicalEqualTo(const void* const notificationOrigin,
                               const uint64_t originTriggerType,
                               const uint64_t originTriggerTypeHash) const noexcept
{
    return isValid() && m_notificationInfo.m_notificationOrigin == notificationOrigin
           && m_originTriggerType == originTriggerType && m_originTriggerTypeHash == originTriggerTypeHash;
}

Trigger::Trigger(Trigger&& rhs) noexcept
    : m_notificationInfo{rhs.m_notificationInfo}
    , m_hasTriggeredCallback{std::move(rhs.m_hasTriggeredCallback)}
    , m_resetCallback{std::move(rhs.m_resetCallback)}
    , m_uniqueId{rhs.m_uniqueId}
    , m_triggerType{rhs.m_triggerType}
    , m_originTriggerType{rhs.m_originTriggerType}
    , m_originTriggerTypeHash{rhs.m_originTriggerTypeHash}
{
    rhs.invalidate();
}

Trigger& Trigger::operator=(Trigger&& rhs) noexcept
{
    if (this != &rhs)
    {
        reset();

        // NotificationInfo
        m_notificationInfo = std::move(rhs.m_notificationInfo);

        // Trigger
        m_resetCallback = std::move(rhs.m_resetCallback);
        m_hasTriggeredCallback = std::move(rhs.m_hasTriggeredCallback);
        m_uniqueId = rhs.m_uniqueId;
        m_triggerType = rhs.m_triggerType;
        m_originTriggerType = rhs.m_originTriggerType;
        m_originTriggerTypeHash = rhs.m_originTriggerTypeHash;

        rhs.invalidate();
    }
    return *this;
}

uint64_t Trigger::getUniqueId() const noexcept
{
    return m_uniqueId;
}

TriggerType Trigger::getTriggerType() const noexcept
{
    return m_triggerType;
}


} // namespace popo
} // namespace iox
