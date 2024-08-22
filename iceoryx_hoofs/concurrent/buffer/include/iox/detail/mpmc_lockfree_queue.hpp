// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_HPP
#define IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_HPP

#include "iox/atomic.hpp"
#include "iox/detail/mpmc_lockfree_queue/mpmc_index_queue.hpp"
#include "iox/optional.hpp"
#include "iox/uninitialized_array.hpp"

namespace iox
{
namespace concurrent
{
/// @brief implements a lock free queue (i.e. container with FIFO order) of elements of type T
/// with a fixed Capacity
template <typename ElementType, uint64_t Capacity>
class MpmcLockFreeQueue
{
  public:
    using element_t = ElementType;

    /// @brief creates and initalizes an empty MpmcLockFreeQueue
    MpmcLockFreeQueue() noexcept;

    ~MpmcLockFreeQueue() noexcept = default;

    // remark: a thread-safe and lockfree implementation of copy seems impossible
    // but unsafe copying (i.e. where synchronization is up to the user) would be possible
    // can be implemented when it is needed
    MpmcLockFreeQueue(const MpmcLockFreeQueue&) = delete;
    MpmcLockFreeQueue(MpmcLockFreeQueue&&) = delete;
    MpmcLockFreeQueue& operator=(const MpmcLockFreeQueue&) = delete;
    MpmcLockFreeQueue& operator=(MpmcLockFreeQueue&&) = delete;

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
    iox::optional<ElementType> push(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted is moved into the queue if possible
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::optional<ElementType> push(ElementType&& value) noexcept;

    /// @brief tries to remove value in FIFO order
    /// @return value if removal was successful, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::optional<ElementType> pop() noexcept;

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
    using Queue = MpmcIndexQueue<Capacity>;

    // remark: actually m_freeIndices do not have to be in a queue, it could be another
    // multi-push multi-pop capable lockfree container (e.g. a stack or a list)
    Queue m_freeIndices;

    // required to be a queue for MpmcLockFreeQueue to exhibit FIFO behaviour
    Queue m_usedIndices;

    UninitializedArray<ElementType, Capacity> m_buffer;

    Atomic<uint64_t> m_size{0U};

    // template is needed to distinguish between lvalue and rvalue T references
    // (universal reference type deduction)
    template <typename T>
    void writeBufferAt(const uint64_t& index, T&& value) noexcept;

    // needed to avoid code duplication (via universal reference type deduction)
    template <typename T>
    iox::optional<ElementType> pushImpl(T&& value) noexcept;

    optional<ElementType> readBufferAt(const uint64_t& index) noexcept;
};
} // namespace concurrent
} // namespace iox

#include "iox/detail/mpmc_lockfree_queue/mpmc_lockfree_queue.inl"

#endif // IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_HPP
