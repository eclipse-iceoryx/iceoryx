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
#ifndef IOX_UTILS_CONCURRENT_LOFFLI_HPP
#define IOX_UTILS_CONCURRENT_LOFFLI_HPP

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_pointer.hpp"

#include <atomic>
#include <cstdint>

namespace iox
{
namespace concurrent
{
class LoFFLi
{
  private:
    /// @todo std::atomic_is_lock_free check
    struct alignas(8) Node
    {
        uint32_t indexToNextFreeIndex;
        uint32_t abaCounter;
    };

    /// @todo introduce typesafe indices with the properties listed below
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
    uint32_t m_invalidIndex{0U};
    std::atomic<Node> m_head{{0U, 1U}};
    iox::rp::RelativePointer<uint32_t> m_nextFreeIndex;

  public:
    LoFFLi() = default;
    /// @todo: why init not in ctor
    /// @todo: size = capacity

    /// Initializes the lock-free free-list
    /// @param [in] freeIndicesMemory pointer to a memory with the size calculated by requiredMemorySize()
    /// @param [in] size is the number of elements of the free-list; must be the same used at requiredMemorySize()
    void init(cxx::not_null<uint32_t*> freeIndicesMemory, const uint32_t size) noexcept;

    /// Pop a value from the free-list
    /// @param [out] index for an element to use
    /// @return true if index is valid, false otherwise
    bool pop(uint32_t& index) noexcept;

    /// Push previously poped element
    /// @param [in] index to previously poped element
    /// @return true if index is valid or not yet pushed, false otherwise
    bool push(const uint32_t index) noexcept;

    /// Calculates the required memory size for a free-list
    /// @param [in] size is the number of elements of the free-list
    /// @return the required memory size for a free-list with size elements
    static inline constexpr std::size_t requiredMemorySize(const uint32_t size) noexcept
    {
        return (static_cast<size_t>(size) + 1U) * sizeof(uint32_t);
    }
};

} // namespace concurrent
} // namespace iox

#endif // IOX_UTILS_CONCURRENT_LOFFLI_HPP
