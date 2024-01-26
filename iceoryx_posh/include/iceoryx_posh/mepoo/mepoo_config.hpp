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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_MEPOO_MEPOO_CONFIG_HPP
#define IOX_POSH_MEPOO_MEPOO_CONFIG_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/vector.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class PortManager;
}
namespace mepoo
{
struct MePooConfig
{
  public:
    struct Entry
    {
        /// @brief set the size and count of memory chunks
        Entry(uint64_t size, uint32_t chunkCount) noexcept
            : m_size(size)
            , m_chunkCount(chunkCount)
        {
        }
        uint64_t m_size{0};
        uint32_t m_chunkCount{0};
    };

    using MePooConfigContainerType = vector<Entry, MAX_NUMBER_OF_MEMPOOLS>;
    MePooConfigContainerType m_mempoolConfig;

    /// @brief Default constructor to set the configuration for memory pools
    MePooConfig() noexcept = default;

    /// @brief Get function for receiving memory pool configuration
    /// @return iox::vector of config information size and count of chunks
    const MePooConfigContainerType* getMemPoolConfig() const noexcept;

    /// @brief Function for adding new entry
    /// @param[in] Entry structure of mempool configuration
    void addMemPool(Entry entry) noexcept;

    /// @brief Function for creating default memory pools
    MePooConfig& setDefaults() noexcept;

    /// @brief Function for optimizing the size of memory pool according to new entry
    MePooConfig& optimize() noexcept;
};

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEPOO_CONFIG_HPP
