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

#pragma once

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

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

    uint32_t m_size{0u};
    uint32_t m_invalidIndex{0u};
    std::atomic<Node> m_head{{0u, 1u}};
    iox::relative_ptr<uint32_t> m_nextFreeIndex;

  public:
    LoFFLi() = default;
    /// @todo: why init not in ctor
    /// @todo: f_size = capacity

    /// Initializes the lock-free free-list
    /// @param [in] f_freeIndicesMemory pointer to a memory with the size calculated by requiredMemorySize()
    /// @param [in] f_size is the number of elements of the free-list; must be the same used at requiredMemorySize()
    void init(cxx::not_null<uint32_t*> f_freeIndicesMemory, const uint32_t f_size);

    /// Pop a value from the free-list
    /// @param [out] index for an element to use
    /// @return true if index is valid, false otherwise
    bool pop(uint32_t& index);

    /// Push previously poped element
    /// @param [in] index to previously poped element
    /// @return true if index is valid or not yet pushed, false otherwise
    bool push(const uint32_t index);

    /// Calculates the required memory size for a free-list
    /// @param [in] f_size is the number of elements of the free-list
    /// @return the required memory size for a free-list with f_size elements
    static inline constexpr std::size_t requiredMemorySize(const uint32_t f_size)
    {
        return (static_cast<size_t>(f_size) + 1) * sizeof(uint32_t);
    }
};

} // namespace concurrent
} // namespace iox
