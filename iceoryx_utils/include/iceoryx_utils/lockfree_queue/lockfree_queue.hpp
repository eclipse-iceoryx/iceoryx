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


namespace iox
{
/// @todo finalize interface, allow late configuration of actual capacity

/// @brief implements a lock free queue (i.e. container with FIFO order) of elements of type T
/// with Capacity
template <typename T, uint64_t Capacity>
class LockFreeQueue
{
  public:
    /// @brief creates and initalizes an empty LockFreeQueue
    /// internally m_freeIndices are initialized as a full queue with indices 0...Capacity-1
    /// and m_usedIndices are initialized as an empty queue
    LockFreeQueue() noexcept;

    /// @todo: a thread-safe and lockfree implementation of copy seems impossible
    /// but unsafe copying (i.e. where synchronization is up to the user) would be possible
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    /// @brief returns the capacity of the queue
    /// threadsafe, lockfree
    constexpr uint64_t capacity() noexcept;

    /// @brief tries to insert value in FIFO order
    /// @return true iff insertion was successful (i.e. queue was not full during push), false otherwise
    /// threadsafe, lockfree
    bool try_push(const T& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @return removed value iff an overflow occured, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<T> push(const T& value) noexcept;

    /// @brief tries to remove value in FIFO order
    /// @return value iff removal was successful, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<T> pop() noexcept;

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    ///  (but it was at some point during the call)
    bool empty();

  private:
    using UniqueIndex = typename IndexQueue<Capacity>::UniqueIndex;
    // actually m_freeIndices do not have to be in a queue, it could be another
    // multi-push multi-pop capable lockfree container (e.g. a stack or a list)
    // @todo: replace with more efficient lockfree structure once available
    IndexQueue<Capacity> m_freeIndices;

    // required to be a queue for LockFreeQueue to exhibit FIFO behaviour
    IndexQueue<Capacity> m_usedIndices;


    Buffer<T, Capacity> m_buffer;
};
} // namespace iox

#include "iceoryx_utils/internal/lockfree_queue/lockfree_queue.inl"