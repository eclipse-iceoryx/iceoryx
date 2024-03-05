// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP
#define IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/memory_block.hpp"
#include "iox/not_null.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class PortPoolMemoryBlock : public MemoryBlock
{
  public:
    /// @todo iox-#1709 the PortPool needs to be refactored to use a typed MemPool
    /// once that is done, the cTor needs a configuration similar to MemPoolCollectionMemoryProvider
    /// @param[in] uniqueRouDiId to tie the ports to
    PortPoolMemoryBlock(const roudi::UniqueRouDiId uniqueRouDiId) noexcept;
    ~PortPoolMemoryBlock() noexcept;

    PortPoolMemoryBlock(const PortPoolMemoryBlock&) = delete;
    PortPoolMemoryBlock(PortPoolMemoryBlock&&) = delete;
    PortPoolMemoryBlock& operator=(const PortPoolMemoryBlock&) = delete;
    PortPoolMemoryBlock& operator=(PortPoolMemoryBlock&&) = delete;

    /// @copydoc MemoryBlock::size
    /// @note The size of for all the ports
    uint64_t size() const noexcept override;

    /// @copydoc MemoryBlock::alignment
    /// @note The memory alignment for the ports
    uint64_t alignment() const noexcept override;

    /// @brief This function enables the access to the PortPool
    /// @return an optional pointer to the underlying type, nullopt_t if value is not initialized
    optional<PortPoolData*> portPool() const noexcept;

  protected:
    /// @copydoc MemoryBlock::onMemoryAvailable
    /// @note This will create the ports at the location 'memory' points to
    void onMemoryAvailable(not_null<void*> memory) noexcept override;

    /// @copydoc MemoryBlock::destroy
    /// @note This will clean up the ports
    void destroy() noexcept override;

  private:
    PortPoolData* m_portPoolData{nullptr};
    const roudi::UniqueRouDiId m_uniqueRouDiId;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP
