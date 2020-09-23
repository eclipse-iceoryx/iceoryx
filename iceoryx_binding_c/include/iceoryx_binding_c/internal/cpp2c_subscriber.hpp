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

#ifndef IOX_BINDING_C_CPP2C_SUBSCRIBER_H
#define IOX_BINDING_C_CPP2C_SUBSCRIBER_H

#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/condition.hpp"

struct cpp2c_Subscriber : public iox::popo::Condition
{
    bool setConditionVariable(iox::popo::ConditionVariableData* const conditionVariableDataPtr) noexcept override;
    bool hasTriggered() const noexcept override;
    bool unsetConditionVariable() noexcept override;

    iox::popo::SubscriberPortData* m_portData{nullptr};
    bool m_wasTriggered{false};
};
#endif
