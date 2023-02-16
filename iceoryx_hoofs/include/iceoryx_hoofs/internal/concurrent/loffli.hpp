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
#ifndef IOX_HOOFS_CONCURRENT_LOFFLI_HPP
#define IOX_HOOFS_CONCURRENT_LOFFLI_HPP

#include "iox/not_null.hpp"
#include "iox/relative_pointer.hpp"

#include <atomic>
#include <cstdint>

namespace iox
{
namespace concurrent
{
constexpr uint32_t NODE_ALIGNMENT{8};
constexpr uint32_t NODE_SIZE{8};

class LoFFLi
{
  public:
    using Index_t = uint32_t;

  private:
    struct alignas(NODE_ALIGNMENT) Node
    {
        Index_t indexToNextFreeIndex;
        uint32_t abaCounter;
    };

    static_assert(sizeof(Node) <= NODE_SIZE,
                  "The size of 'Node' must not exceed 8 bytes in order to be lock-free on 64 bit systems!");

    /// @todo iox-#680 introduce typesafe indices with the properties listed below
    ///       id is required that not two loefflis with the same properties
    ///       mix up the id
    ///       value = index
    /// @code
    ///    class Id_t
    ///    {
    ///        Id_t() = default;
    ///        Id_t(const Id_t&) = delete;
    ///        Id_t(Id_t&&) = default;
    ///        ~Id_t() = default;

    ///        Id_t& operator=(const Id_t&) = delete;
    ///        Id_t& operator=(Id_t&) = default;

    ///        friend class LoFFLi;

    ///      private:
    ///        uint32_t value;
    ///        uint32_t id = MyLoeffliObject.id; //imaginary code
    ///    };
    /// @endcode

    uint32_t m_size{0U};
    Index_t m_invalidIndex{0U};
    std::atomic<Node> m_head{{0U, 1U}};
    iox::RelativePointer<Index_t> m_nextFreeIndex;

  public:
    LoFFLi() noexcept = default;
    /// @todo iox-#680 move 'init()' to the ctor, remove !m_nextfreeIndex checks

    /// Initializes the lock-free free-list
    /// @param [in] freeIndicesMemory pointer to a memory with the capacity calculated by requiredMemorySize()
    /// @param [in] capacity is the number of elements of the free-list; must be the same used at requiredMemorySize()
    void init(not_null<Index_t*> freeIndicesMemory, const uint32_t capacity) noexcept;

    /// Pop a value from the free-list
    /// @param [out] index for an element to use
    /// @return true if index is valid, false otherwise
    bool pop(Index_t& index) noexcept;

    /// Push previously poped element
    /// @param [in] index to previously poped element
    /// @return true if index is valid or not yet pushed, false otherwise
    bool push(const Index_t index) noexcept;

    /// Calculates the required memory size for a free-list
    /// @param [in] capacity is the number of elements of the free-list
    /// @return the required memory size for a free-list with the requested capacity
    static inline constexpr uint64_t requiredIndexMemorySize(const uint64_t capacity) noexcept;
};

} // namespace concurrent
} // namespace iox

#include "loffli.inl"

#endif // IOX_HOOFS_CONCURRENT_LOFFLI_HPP
