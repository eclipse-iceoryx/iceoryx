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
#ifndef IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP
#define IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/memory_block.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class PortPoolMemoryBlock : public MemoryBlock
{
  public:
    /// @todo the PortPool needs to be refactored to use a typed MemPool
    /// once that is done, the cTor needs a configuration similar to MemPoolCollectionMemoryProvider
    PortPoolMemoryBlock() noexcept = default;
    ~PortPoolMemoryBlock() noexcept;

    PortPoolMemoryBlock(const PortPoolMemoryBlock&) = delete;
    PortPoolMemoryBlock(PortPoolMemoryBlock&&) = delete;
    PortPoolMemoryBlock& operator=(const PortPoolMemoryBlock&) = delete;
    PortPoolMemoryBlock& operator=(PortPoolMemoryBlock&&) = delete;

    /// @brief Implementation of MemoryBlock::size
    /// @return the size of for all the ports
    uint64_t size() const noexcept override;

    /// @brief Implementation of MemoryBlock::alignment
    /// @return the memory alignment for the ports
    uint64_t alignment() const noexcept override;

    /// @brief Implementation of MemoryBlock::memoryAvailable
    /// This will create the ports
    /// @param [in] memory pointer to a valid memory location to place the mempools
    void memoryAvailable(void* memory) noexcept override;

    /// @brief Implementation of MemoryBlock::destroy
    /// This will clean up the ports
    void destroy() noexcept override;

    /// @brief This function enables the access to the PortPool
    /// @return an optional pointer to the underlying type, cxx::nullopt_t if value is not initialized
    cxx::optional<PortPoolData*> portPool() const noexcept;

  private:
    PortPoolData* m_portPoolData{nullptr};
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_PORT_POOL_MEMORY_BLOCK_HPP
