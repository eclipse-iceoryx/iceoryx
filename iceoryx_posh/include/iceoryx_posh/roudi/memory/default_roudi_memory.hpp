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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ROUDI_MEMORY_DEFAULT_ROUDI_MEMORY_HPP
#define IOX_POSH_ROUDI_MEMORY_DEFAULT_ROUDI_MEMORY_HPP

#include "iceoryx_posh/internal/roudi/memory/mempool_collection_memory_block.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_segment_manager_memory_block.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

namespace iox
{
namespace roudi
{
struct DefaultRouDiMemory
{
  public:
    DefaultRouDiMemory(const RouDiConfig_t& roudiConfig) noexcept;
    virtual ~DefaultRouDiMemory() noexcept = default;

    DefaultRouDiMemory(DefaultRouDiMemory&&) = delete;
    DefaultRouDiMemory& operator=(DefaultRouDiMemory&&) = delete;

    DefaultRouDiMemory(const DefaultRouDiMemory&) = delete;
    DefaultRouDiMemory& operator=(const DefaultRouDiMemory&) = delete;

    mepoo::MePooConfig introspectionMemPoolConfig() const noexcept;

    MemPoolCollectionMemoryBlock m_introspectionMemPoolBlock;
    MemPoolSegmentManagerMemoryBlock m_segmentManagerBlock;
    PosixShmMemoryProvider m_managementShm;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_DEFAULT_ROUDI_MEMORY_HPP
