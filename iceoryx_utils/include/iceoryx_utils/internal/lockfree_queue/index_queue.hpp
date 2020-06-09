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
#include "iceoryx_utils/internal/lockfree_queue/buffer.hpp"
#include "iceoryx_utils/internal/lockfree_queue/cyclic_index.hpp"

#include <atomic>

namespace iox
{
/// @brief lockfree queue capable of storing indices 0,1,... Capacity-1
template <uint64_t Capacity, typename ValueType = uint64_t>
class IndexQueue
{
  public:
    class UniqueIndex
    {
      public:
        using value_t = ValueType;

        // for some reason, using nullopt_t instead with an alias and constructing a constexpr static is not possible
        // UniqueIndex is scheduled to be refactored in the capacity change feature, leave it like this for now

        struct invalid_t
        {
        };

        static constexpr invalid_t invalid{};

        friend class IndexQueue<Capacity, ValueType>;

        /// @brief only an invalid index can be constructed by anyone other than the IndexQueue itself
        UniqueIndex(invalid_t) noexcept
            : m_value(cxx::nullopt_t{})
        {
        }

        // no copies
        UniqueIndex(const UniqueIndex&) = delete;
        UniqueIndex& operator=(const UniqueIndex&) = delete;

        // only move
        UniqueIndex(UniqueIndex&&) = default;
        UniqueIndex& operator=(UniqueIndex&&) = default;

        /// @brief returns whether it contains valid index
        /// @return true if the index is valid, false otherwise
        bool isValid()
        {
            return m_value.has_value();
        }

        /// @brief converts to a reference to the underlying value if it is valid, undefined behaviour otherwise
        /// this allows using it as the underlying type in certain circumstances (without copying)
        /// @return reference to the underlying value
        operator const ValueType&() const
        {
            return m_value.value();
        }

        /// @brief releases the underlying value if it is valid, undefined behaviour otherwise
        ///  the object is invalid afterwards
        /// @return the underlying value
        ValueType release()
        {
            auto value = std::move(m_value.value());
            m_value.reset();
            return value;
        }

        /// @brief returns a reference to the underlying value if it is valid, undefined behaviour otherwise
        /// @return reference to the underlying value
        ValueType& operator*()
        {
            return m_value.value();
        }

      private:
        // valid construction is made private and only accessible by the IndexQueue
        // (that is also a reason why we use a nested class)
        UniqueIndex(ValueType value) noexcept
            : m_value(std::move(value))
        {
        }

        // composition is preferred here to inheritance to control the (minimal) interface,
        // we use the optional to benefit from its move behaviour
        iox::cxx::optional<ValueType> m_value;
    };

    using value_t = ValueType;

    struct ConstructFull_t
    {
    };

    struct ConstructEmpty_t
    {
    };

    static constexpr ConstructFull_t ConstructFull{};
    static constexpr ConstructEmpty_t ConstructEmpty{};

    IndexQueue(const IndexQueue&) = delete;
    IndexQueue(IndexQueue&&) = delete;
    IndexQueue& operator=(const IndexQueue&) = delete;
    IndexQueue& operator=(IndexQueue&&) = delete;

    /// @brief constructs an empty IndexQueue
    constexpr IndexQueue(ConstructEmpty_t = ConstructEmpty) noexcept;

    /// @brief constructs IndexQueue filled with all indices 0,1,...capacity-1
    IndexQueue(ConstructFull_t) noexcept;

    /// @brief get the capacity of the IndexQueue
    /// @return capacity of the IndexQueue
    /// threadsafe, lockfree
    constexpr uint64_t capacity() const noexcept;

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    /// (but it was at some point during the call)
    bool empty() const noexcept;

    // The advantage of the UniqueIndex interface is that it prevents us from returning
    // an index multiple times by design, i.e. it enforces that only indices popped from
    // an IndexQueue can be returned.
    // This works by preventing copies and construction of UniqueIndex outside of the IndexQueue.
    // In particular, the user is free to get and copy the raw index, but he *cannot* construct a new
    // UniqueIndex from it.

    /// @brief push index into the queue in FIFO order
    /// always succeeds if the UniqueIndex to be pushed is popped from another equallysized
    /// IndexQueue (and should only be used this way)
    /// threadsafe, lockfree
    /// @param index to be pushed, any index can only be pushed once by design
    void push(UniqueIndex& index) noexcept;

    /// @brief tries to remove index in FIFO order
    /// threadsafe, lockfree
    /// @return valid UniqueIndex if removal was successful (i.e. queue was not empty),
    /// invalid UnqiueIndex otherwise
    UniqueIndex pop() noexcept;

    /// @brief tries to remove index in FIFO order iff the queue is full
    /// threadsafe, lockfree
    /// @return valid UniqueIndex if removal was successful (i.e. queue was full),
    /// invalid UniqueIndex otherwise
    UniqueIndex popIfFull() noexcept;

  private:
    // remark: a compile time check whether Index is actually lock free would be nice
    // note: there is a way  with is_always_lock_free in c++17 (which we cannot use here)
    using Index = CyclicIndex<Capacity>;

    using Cell = std::atomic<Index>;
    Cell m_cells[Capacity];

    std::atomic<Index> m_readPosition;
    std::atomic<Index> m_writePosition;

  private:
    Index loadValueAt(const Index position) const noexcept;

    // internal raw value (ValueType) interface
    // private, since it does not prevent multiple of the same index pushes by design

    // push index into the queue in FIFO order
    void push(const ValueType index) noexcept;

    // tries to remove index in FIFO order, succeeds if the queue is not empty
    bool pop(ValueType& index) noexcept;

    // tries to remove index in FIFO order if the queue is full
    bool popIfFull(ValueType& index) noexcept;
};
} // namespace iox

#include "index_queue.inl"
