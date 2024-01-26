// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_HPP
#define IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_HPP

#include "iox/optional.hpp"
#include "iox/uninitialized_array.hpp"

#include <atomic>

namespace iox
{
namespace concurrent
{
/// @brief single producer single consumer thread safe fifo
/// @note there are at most one push and one pop threads that can work concurrently on the fifo
template <typename ValueType, uint64_t Capacity>
class SpscFifo
{
  public:
    /// @brief pushes a value into the fifo
    /// @note restricted thread-safe: can be called from different threads but not concurrently (i.e.
    /// simultaneously)
    /// @return if the value was pushed successfully into the fifo, returns
    ///         true, otherwise false
    bool push(const ValueType& value) noexcept;

    /// @brief returns the oldest value from the fifo and removes it
    /// @note restricted thread-safe: can be called from different threads but not concurrently (i.e.
    /// simultaneously)
    /// @return if the fifo was not empty, the optional contains the value,
    ///         otherwise it contains a nullopt
    optional<ValueType> pop() noexcept;

    /// @brief returns true when the fifo is empty, otherwise false
    /// @note thread safe (the result might already be outdated when used)
    bool empty() const noexcept;

    /// @brief returns the size of the fifo
    /// @note thread safe (the result might already be outdated when used)
    uint64_t size() const noexcept;

    /// @brief returns the capacity of the fifo
    static constexpr uint64_t capacity() noexcept;

  private:
    // thread safe (the result might already be outdated when used)
    bool is_full() const noexcept;
    std::pair<uint64_t, uint64_t> get_read_write_positions() const noexcept;


  private:
    UninitializedArray<ValueType, Capacity> m_data;
    std::atomic<uint64_t> m_writePos{0};
    std::atomic<uint64_t> m_readPos{0};
};

} // namespace concurrent
} // namespace iox

#include "iox/detail/spsc_fifo.inl"

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_HPP
