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
// limitations under the License

#include "iceoryx_posh/internal/popo/base_port_data.hpp"

namespace iox
{
namespace popo
{
BasePortData::BasePortData() noexcept
    : m_uniqueId(s_uniqueIdCounter.fetch_add(1, std::memory_order_relaxed))
{
}

BasePortData::BasePortData(const capro::ServiceDescription& serviceDescription,
                           const BasePortType& portType,
                           const cxx::CString100& processName,
                           const Interfaces interface,
                           runtime::RunnableData* const runnable) noexcept
    : m_portType(portType)
    , m_serviceDescription(serviceDescription)
    , m_processName(processName)
    , m_interface(interface)
    , m_uniqueId(s_uniqueIdCounter.fetch_add(1, std::memory_order_relaxed))
    , m_runnable(runnable)
{
}
} // namespace popo
} // namespace iox
