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
#include "iceoryx_utils/internal/lockfree_queue/index_queue.hpp"

#include <atomic>


namespace iox
{
/// @todo finalize interface, configuration of actual capacity at runtime (change capacity feature)

/// @brief implements a lock free queue (i.e. container with FIFO order) of elements of type T
/// with Capacity
template <typename ElementType, uint64_t Capacity>
class LockFreeQueue
{
  public:
    /// @brief creates and initalizes an empty LockFreeQueue
    LockFreeQueue() noexcept;

    /// @todo: a thread-safe and lockfree implementation of copy seems impossible
    /// but unsafe copying (i.e. where synchronization is up to the user) would be possible
    /// can be implemented when it is needed
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    /// @brief returns the capacity of the queue
    /// threadsafe, lockfree
    constexpr uint64_t capacity() noexcept;

    /// @brief tries to insert value in FIFO order
    /// @param value to be inserted (value semantics support move by the user)
    /// @return true if insertion was successful (i.e. queue was not full during push), false otherwise
    /// threadsafe, lockfree
    bool try_push(const ElementType value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted (value semantics support move by the user)
    /// @return removed value if an overflow occured, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<ElementType> push(const ElementType value) noexcept;

    /// @brief tries to remove value in FIFO order
    /// @return value if removal was successful, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<ElementType> pop() noexcept;

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    ///  (but it was at some point during the call)
    bool empty();

    /// @brief get the number of stored elements in the queue
    /// @return number of stored elements in the queue
    /// note that this will not be perfectly in sync with the actual number of contained elements
    /// during concurrent operation but will always be at most capacity
    uint64_t size();

  private:
    using Queue = IndexQueue<Capacity>;
    using UniqueIndex = typename Queue::UniqueIndex;
    using BufferIndex = typename Queue::value_t;

    // actually m_freeIndices do not have to be in a queue, it could be another
    // multi-push multi-pop capable lockfree container (e.g. a stack or a list)
    // @todo: replace with more efficient lockfree structure once available
    Queue m_freeIndices;

    // required to be a queue for LockFreeQueue to exhibit FIFO behaviour
    Queue m_usedIndices;

    Buffer<ElementType, Capacity, BufferIndex> m_buffer;

    std::atomic<uint64_t> m_size{0};

    // note that we now perform the memory synchronization not using the index queue anymore but
    // with those methods
    // this has the advantage of limiting unneccessary synchronization (e.g. due to CAS failure)
    // and keeps the responsibility inside the LockFreeQueue itself (which contains the buffer)

    void writeBufferAt(const UniqueIndex&, const ElementType&);
    cxx::optional<ElementType> readBufferAt(const UniqueIndex&);
};
} // namespace iox

#include "iceoryx_utils/internal/lockfree_queue/lockfree_queue.inl"