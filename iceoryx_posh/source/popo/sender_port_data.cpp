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

#include "iceoryx_posh/internal/popo/sender_port_data.hpp"

namespace iox
{
namespace popo
{
SenderPortData::SenderPortData(mepoo::MemoryManager* const f_memoryMgr, mepoo::SharedChunk f_lastChunk) noexcept
    : BasePortData()
    , m_memoryMgr(f_memoryMgr)
    , m_lastChunk(f_lastChunk)
{
}
SenderPortData::SenderPortData(const capro::ServiceDescription& serviceDescription,
                               mepoo::MemoryManager* const memMgr,
                               const std::string& applicationName,
                               const Interfaces interface,
                               runtime::RunnableData* const runnable) noexcept
    : BasePortData(serviceDescription, BasePortType::SENDER_PORT, applicationName, interface, runnable)
    , m_memoryMgr(memMgr)
{
}

} // namespace popo
} // namespace iox
