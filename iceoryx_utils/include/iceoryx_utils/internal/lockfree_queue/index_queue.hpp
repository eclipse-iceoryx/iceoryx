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

#include "iceoryx_utils/internal/lockfree_queue/buffer.hpp"
#include "iceoryx_utils/internal/lockfree_queue/cyclic_index.hpp"
#include "iceoryx_utils/internal/lockfree_queue/unique.hpp"

#include <atomic>

namespace iox
{
/// @brief lockfree queue capable of storing indices 0,1,... Capacity-1
template <uint64_t Capacity, typename ValueType = uint64_t>
class IndexQueue
{
  public:
    class UniqueIndex : public unique<ValueType>
    {
      public:
        using invalid_t = typename unique<ValueType>::invalid_t;
        static constexpr invalid_t invalid{};

        friend class IndexQueue<Capacity, ValueType>;

        // only an invalid index can be constructed by anyone other than the IndexQueue itself
        UniqueIndex(invalid_t)
            : unique<ValueType>(invalid)
        {
        }

      private:
        // valid construction is made private and only accessible by the IndexQueue
        UniqueIndex(const ValueType& value)
            : unique<ValueType>(value)
        {
        }
    };

    using value_t = ValueType;

    struct ConstructFull_t
    {
    };

    struct ConstructEmpty_t
    {
    };

    using Index = CyclicIndex<Capacity>;

    // @todo: a compile time check whether Index is actually lock free would be nice
    // note: there is a way  with is_always_lock_free in c++17 (which we cannot use here)


  public:
    static constexpr ConstructFull_t ConstructFull{};
    static constexpr ConstructEmpty_t ConstructEmpty{};

    IndexQueue(const IndexQueue&) = delete;
    IndexQueue(IndexQueue&&) = delete;
    IndexQueue& operator=(const IndexQueue&) = delete;
    IndexQueue& operator=(IndexQueue&&) = delete;

    /// @brief constructs an empty IndexQueue
    IndexQueue(ConstructEmpty_t = ConstructEmpty);

    /// @brief constructs IndexQueue filled with all indices 0,1,...capacity-1
    IndexQueue(ConstructFull_t);

    /// @brief get the capacity of the IndexQueue
    /// @return capacity of the IndexQueue
    /// threadsafe, lockfree
    constexpr uint64_t capacity();

    /// @brief push index into the queue in FIFO order
    /// constraint: pushing more indices than capacity is not allowed
    /// constraint: only indices in the range [0, Capacity-1] are allowed
    /// threadsafe, lockfree
    void push(ValueType index);

    /// @brief tries to remove index in FIFO order
    /// @return true iff removal was successful (i.e. queue was not empty)
    /// value is only valid if the function returns true
    /// threadsafe, lockfree
    bool pop(ValueType& index);

    /// @brief tries to remove index in FIFO order iff the queue is full
    /// @return true iff removal was successful (i.e. queue was full)
    /// value is only valid if the function returns true
    /// threadsafe, lockfree
    bool popIfFull(ValueType& index);

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    /// (but it was at some point during the call)
    bool empty();

    // @todo: finalize interface
    void push(const UniqueIndex& index);

    UniqueIndex pop();

    UniqueIndex popIfFull();

  private:
    using Cell = std::atomic<Index>;
    Cell m_cells[Capacity];

    std::atomic<Index> m_readPosition;
    std::atomic<Index> m_writePosition;

  private:
    Index loadNextReadPosition() const;
    Index loadNextWritePosition() const;
    Index loadValueAt(Index position) const;
    bool tryToPublishAt(Index writePosition, Index& oldValue, Index newValue);
    bool tryToGainOwnershipAt(Index& readPosition);
    void updateNextWritePosition(Index& oldWritePosition);
};
} // namespace iox

#include "index_queue.inl"
