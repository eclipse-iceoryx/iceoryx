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

#include "iceoryx_posh/roudi/memory/default_roudi_memory.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace roudi
{
DefaultRouDiMemory::DefaultRouDiMemory(const RouDiConfig_t& roudiConfig) noexcept
    : m_introspectionMemPoolBlock(introspectionMemPoolConfig())
    , m_segmentManagerBlock(roudiConfig)
    , m_managementShm(SHM_NAME, posix::AccessMode::readWrite, posix::OwnerShip::mine)

{
    m_managementShm.addMemoryBlock(&m_introspectionMemPoolBlock);
    m_managementShm.addMemoryBlock(&m_segmentManagerBlock);
}
mepoo::MePooConfig DefaultRouDiMemory::introspectionMemPoolConfig() const
{
    mepoo::MePooConfig mempoolConfig;
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(
             cxx::align(static_cast<uint64_t>(sizeof(roudi::MemPoolIntrospectionTopic)), SHARED_MEMORY_ALIGNMENT)),
         250});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(
             cxx::align(static_cast<uint64_t>(sizeof(roudi::ProcessIntrospectionFieldTopic)), SHARED_MEMORY_ALIGNMENT)),
         10});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(
             cxx::align(static_cast<uint64_t>(sizeof(roudi::PortIntrospectionFieldTopic)), SHARED_MEMORY_ALIGNMENT)),
         10});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(cxx::align(static_cast<uint64_t>(sizeof(roudi::PortThroughputIntrospectionFieldTopic)),
                                          SHARED_MEMORY_ALIGNMENT)),
         10});
    mempoolConfig.m_mempoolConfig.push_back(
        {static_cast<uint32_t>(
             cxx::align(static_cast<uint64_t>(sizeof(roudi::ReceiverPortChangingIntrospectionFieldTopic)),
                        SHARED_MEMORY_ALIGNMENT)),
         10});

    mempoolConfig.optimize();
    return mempoolConfig;
}

} // namespace roudi
} // namespace iox