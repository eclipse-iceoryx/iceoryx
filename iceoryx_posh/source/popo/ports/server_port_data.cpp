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
// limitations under the License

#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"

namespace iox
{
namespace popo
{
ServerPortData::ServerPortData(const capro::ServiceDescription& serviceDescription,
                               const ProcessName_t& processName,
                               mepoo::MemoryManager* const memoryManager,
                               const mepoo::MemoryInfo& memoryInfo) noexcept
    : BasePortData(serviceDescription, processName)
    , m_chunkSenderData(memoryManager, 0, memoryInfo)
    , m_chunkReceiverData(cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer)
{
}

} // namespace popo
} // namespace iox
