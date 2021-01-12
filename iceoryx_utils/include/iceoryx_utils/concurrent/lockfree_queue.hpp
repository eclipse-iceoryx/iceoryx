// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_CONCURRENT_LOCKFREE_QUEUE_HPP
#define IOX_UTILS_CONCURRENT_LOCKFREE_QUEUE_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/concurrent/lockfree_queue/buffer.hpp"
#include "iceoryx_utils/internal/concurrent/lockfree_queue/index_queue.hpp"

#include <atomic>


namespace iox
{
namespace concurrent
{
/// @brief implements a lock free queue (i.e. container with FIFO order) of elements of type T
/// with a fixed Capacity
template <typename ElementType, uint64_t Capacity>
class LockFreeQueue
{
  public:
    using element_t = ElementType;

    /// @brief creates and initalizes an empty LockFreeQueue
    LockFreeQueue() noexcept;

    ~LockFreeQueue() = default;

    // remark: a thread-safe and lockfree implementation of copy seems impossible
    // but unsafe copying (i.e. where synchronization is up to the user) would be possible
    // can be implemented when it is needed
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    /// @brief returns the capacity of the queue
    /// @note threadsafe, lockfree
    constexpr uint64_t capacity() const noexcept;

    /// @brief tries to insert value in FIFO order, moves the value internally
    /// @param value to be inserted
    /// @return true if insertion was successful (i.e. queue was not full during push), false otherwise
    /// @note threadsafe, lockfree
    bool tryPush(ElementType&& value) noexcept;

    /// @brief tries to insert value in FIFO order, copies the value internally
    /// @param value to be inserted
    /// @return true if insertion was successful (i.e. queue was not full during push), false otherwise
    /// @note threadsafe, lockfree
    bool tryPush(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted is copied into the queue
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::cxx::optional<ElementType> push(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted is moved into the queue if possible
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::cxx::optional<ElementType> push(ElementType&& value) noexcept;

    /// @brief tries to remove value in FIFO order
    /// @return value if removal was successful, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::cxx::optional<ElementType> pop() noexcept;

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// @note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    ///  (but it was at some point during the call)
    /// @note threadsafe, lockfree
    bool empty() const noexcept;

    /// @brief get the number of stored elements in the queue
    /// @return number of stored elements in the queue
    /// @note that this will not be perfectly in sync with the actual number of contained elements
    /// during concurrent operation but will always be at most capacity
    /// @note threadsafe, lockfree
    uint64_t size() const noexcept;

  protected:
    using Queue = IndexQueue<Capacity>;
    using BufferIndex = typename Queue::value_t;

    // remark: actually m_freeIndices do not have to be in a queue, it could be another
    // multi-push multi-pop capable lockfree container (e.g. a stack or a list)
    Queue m_freeIndices;

    // required to be a queue for LockFreeQueue to exhibit FIFO behaviour
    Queue m_usedIndices;

    Buffer<ElementType, Capacity, BufferIndex> m_buffer;

    std::atomic<uint64_t> m_size{0u};

    // template is needed to distinguish between lvalue and rvalue T references
    // (universal reference type deduction)
    template <typename T>
    void writeBufferAt(const BufferIndex&, T&&);

    // needed to avoid code duplication (via universal reference type deduction)
    template <typename T>
    iox::cxx::optional<ElementType> pushImpl(T&& value) noexcept;

    cxx::optional<ElementType> readBufferAt(const BufferIndex&);
};
} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/lockfree_queue/lockfree_queue.inl"

#endif // IOX_UTILS_CONCURRENT_LOCKFREE_QUEUE_HPP