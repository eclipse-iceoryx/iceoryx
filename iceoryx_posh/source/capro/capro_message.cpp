// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/capro/capro_message.hpp"

namespace iox
{
namespace capro
{
CaproMessage::CaproMessage() noexcept
    : m_requestPort(nullptr)
{
}

CaproMessage::CaproMessage(CaproMessageType f_type,
                           const ServiceDescription& f_serviceDescription,
                           CaproMessageSubType f_subType,
                           popo::ReceiverPortData* f_requestPort) noexcept
    : m_type(f_type)
    , m_subType(f_subType)
    , m_serviceDescription(f_serviceDescription)
    , m_requestPort(f_requestPort)
{
}

} // namespace capro
} // namespace iox
