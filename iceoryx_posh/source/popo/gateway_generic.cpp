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

#include "iceoryx_posh/popo/gateway_generic.hpp"

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
GatewayGeneric::GatewayGeneric(const Interfaces f_interface) noexcept
    : m_interfaceImpl(runtime::PoshRuntime::getInstance().getMiddlewareInterface(f_interface))
{
}

bool GatewayGeneric::getCaProMessage(CaproMessage& msg) noexcept
{
    return m_interfaceImpl.getCaProMessage(msg);
}

} // namespace popo
} // namespace iox
