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

#include "iceoryx_utils/cxx/optional.hpp"

#include <atomic>

namespace iox
{
namespace concurrent
{
/// @brief single pusher single pop'er thread safe fifo
template <typename ValueType, uint32_t Capacity>
class FiFo
{
  public:
    /// @brief pushes a value into the fifo
    /// @return if the values was pushed successfully into the fifo it returns
    ///         true, otherwise false
    bool push(const ValueType& f_value);

    /// @brief returns the oldest value from the fifo and removes it
    /// @return if the fifo was not empty the optional contains the value,
    ///         otherwise it contains a nullopt
    cxx::optional<ValueType> pop();

    /// @brief returns true when the fifo is empty, otherwise false
    bool empty() const;

  private:
    bool is_full() const;

  private:
    ValueType m_data[Capacity];
    std::atomic<uint64_t> m_write_pos{0};
    std::atomic<uint64_t> m_read_pos{0};
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/fifo.inl"

