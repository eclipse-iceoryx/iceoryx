// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/gateway/gateway_base.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace gw
{
GatewayBase::GatewayBase(const capro::Interfaces f_interface) noexcept
    : m_interfaceImpl(runtime::PoshRuntime::getInstance().getMiddlewareInterface(f_interface))
{
}

GatewayBase::~GatewayBase() noexcept
{
    if (m_interfaceImpl)
    {
        m_interfaceImpl.destroy();
    }
}

capro::Interfaces GatewayBase::getInterface() const noexcept
{
    return m_interfaceImpl.getCaProServiceDescription().getSourceInterface();
}

bool GatewayBase::getCaProMessage(CaproMessage& msg) noexcept
{
    auto maybeCaproMessage = m_interfaceImpl.tryGetCaProMessage();
    if (maybeCaproMessage.has_value())
    {
        msg = maybeCaproMessage.value();
        return true;
    }
    else
    {
        msg.m_type = capro::CaproMessageType::NOTYPE;
        return false;
    }
}
} // namespace gw
} // namespace iox
