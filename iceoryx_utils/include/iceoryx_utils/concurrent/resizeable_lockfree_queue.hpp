// Copyright (c) 2020 Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
#define IOX_UTILS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP

#include "iceoryx_utils/concurrent/lockfree_queue.hpp"
#include "iceoryx_utils/cxx/type_traits.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <atomic>

namespace iox
{
namespace concurrent
{
/// @brief implements a lock free queue (i.e. container with FIFO order) of elements of type T
/// with a maximum capacity MaxCapacity.
/// The capacity can be defined to be anything between 0 and MaxCapacity at construction time
/// or later at runtime using setCapacity. This is even possible while concurrent push and pop
/// operations are executed, i.e. the queue does not have to be empty.
/// Only one thread will succeed setting its desired capacity if there are more
/// threads trying to change the capacity at the same time (it is unpredictable which thread).

// Remark: We use protected inheritance to make the base class methods inaccessible for the user.
// We cannot use virtual functions since we need to use this class in shared memory.
// Some of the methods need to be rewritten specifically for this class, others simply redirect
// the call to the base class.
//
// Since supporting the resize (setCapacity) functionality has an impact on the runtime even
// if the feature is not used, we provide a queue wihout resize functionality in an additional
// base class that can be used separately.
template <typename ElementType, uint64_t MaxCapacity>
class ResizeableLockFreeQueue : protected LockFreeQueue<ElementType, MaxCapacity>
{
  private:
    using Base = LockFreeQueue<ElementType, MaxCapacity>;

  public:
    using element_t = ElementType;
    static constexpr uint64_t MAX_CAPACITY = MaxCapacity;

    ResizeableLockFreeQueue() = default;
    ~ResizeableLockFreeQueue() = default;

    // deleted for now, can be implemented later if needed
    // note: concurrent copying or moving in lockfree fashion is nontrivial
    ResizeableLockFreeQueue(const ResizeableLockFreeQueue&) = delete;
    ResizeableLockFreeQueue(ResizeableLockFreeQueue&&) = delete;
    ResizeableLockFreeQueue& operator=(const ResizeableLockFreeQueue&) = delete;
    ResizeableLockFreeQueue& operator=(ResizeableLockFreeQueue&&) = delete;

    ResizeableLockFreeQueue(uint64_t initialCapacity) noexcept;

    /// @brief returns the maximum capacity of the queue
    /// @return the maximum capacity
    static constexpr uint64_t maxCapacity() noexcept;

    /// @brief tries to insert value in FIFO order, copies the value internally
    /// @param[in] value to be inserted
    /// @return true if insertion was successful (i.e. queue was not full during push), false otherwise
    /// @note threadsafe, lockfree
    /// bool tryPush(ElementType&& value) noexcept;
    /// bool tryPush(const ElementType& value) noexcept;
    using Base::tryPush;

    /// @brief tries to remove value in FIFO order
    /// @return value if removal was successful, empty optional otherwise
    /// @note threadsafe, lockfree
    /// iox::cxx::optional<ElementType> pop() noexcept;
    using Base::pop;

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// @note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    ///  (but it was at some point during the call)
    /// @note threadsafe, lockfree
    /// bool empty() const noexcept;
    using Base::empty;

    /// @brief get the number of stored elements in the queue
    /// @return number of stored elements in the queue
    /// @note that this will not be perfectly in sync with the actual number of contained elements
    /// during concurrent operation but will always be at most capacity
    /// @note threadsafe, lockfree
    /// uint64_t size() const noexcept;
    using Base::size;

    /// @brief returns the current capacity of the queue
    /// @return the current capacity
    /// @note threadsafe, lockfree
    uint64_t capacity() const noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param[in] value to be inserted is copied into the queue
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::cxx::optional<ElementType> push(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param[in] value to be inserted is moved into the queue if possible
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::cxx::optional<ElementType> push(ElementType&& value) noexcept;

    // multiple overloads to set the capacity
    // 1) The most general one allows providing a removeHandler to specify remove behavior.
    // 2) The overload where a container is provided can be used if the removed elements are to be stored.
    //    The container must satisfy certain requirements, such as providing push_back.
    // 3) The final overload discards removed elements.

    /// @brief      Set the capacity to some value.
    /// @param[in]  newCapacity capacity to be set
    /// @param[in]  removeHandler is a function taking an element which specifies
    ///             what to do with removed elements should the need for removal arise.
    /// @return     true if the capacity was successfully set, false otherwise
    template <typename Function,
              typename = typename std::enable_if<cxx::is_invocable<Function, ElementType>::value>::type>
    bool setCapacity(uint64_t newCapacity, Function&& removeHandler) noexcept;

    /// @brief Set the capacity to a new capacity between 0 and MaxCapacity, if the capacity is reduced
    /// it may be necessary to remove the least recent elements.
    /// @param[in] newCapacity new capacity to be set, if it is larger than MaxCapacity the call fails
    /// @param[out] removedElements container were potentially removed elements can be stored.
    /// @return true setting if the new capacity was successful, false otherwise (newCapacity > MaxCapacity)
    /// @note threadsafe, lockfree but multiple concurrent calls may have no effect
    template <typename ContainerType = iox::cxx::vector<ElementType, MaxCapacity>,
              typename = typename std::enable_if<!cxx::is_invocable<ContainerType, ElementType>::value>::type>
    bool setCapacity(uint64_t newCapacity, ContainerType& removedElements) noexcept;

    /// @brief Set the capacity to a new capacity between 0 and MaxCapacity, if the capacity is reduced
    /// it may be necessary to remove the least recent elements which are then discarded.
    /// @param[in] newCapacity new capacity to be set, if it is larger than MaxCapacity the call fails
    /// @return true setting if the new capacity was successful, false otherwise (newCapacity > MaxCapacity)
    /// @note threadsafe, lockfree but multiple concurrent calls may have no effect
    bool setCapacity(uint64_t newCapacity) noexcept;

  private:
    using BufferIndex = typename Base::BufferIndex;
    std::atomic<uint64_t> m_capacity{MaxCapacity};

    // we also sync m_capacity with this flag
    std::atomic_flag m_resizeInProgress{false};

    // Remark: The vector m_unusedIndices is protected by the atomic flag, but this also means dying during a resize
    // will prevent further resizes (This is not a problem for the use case were only the dying receiver itself requests
    // the resize.)
    // I.e. resize is lockfree, but not in a useful and robust way as it assumes that a concurrent resize will
    // always eventually complete (which is true when the application does not die and the relevant thread is scheduled
    // eventually. The latter is the case for any OS and mandatory for a Realtime OS.

    iox::cxx::vector<BufferIndex, MaxCapacity> m_unusedIndices;

    /// @brief      Increase the capacity by some value.
    /// @param[in]  toIncrease value by which the capacity is to be increased
    /// @return     value by which the capacity was actually increased
    /// @note       If incrementing cannot be carried out (because the MaxCapacity was reached),
    ///             this value will be smaller than toIncrease.
    uint64_t increaseCapacity(uint64_t toIncrease) noexcept;

    /// @brief      Decrease the capacity by some value.
    /// @param[in]  toDecrease value by which the capacity is to be decreased
    /// @param[in]  removedHandler is a function taking an index which specifies
    ///             what to do with removed element at this index (e.g. store in a container or discard it).
    /// @return     value by which the capacity was actually decreased.
    /// @note       If decrementing cannot be carried out (because the capacity is already 0),
    ///             this value will be smaller than toDecrease.
    template <typename Function>
    uint64_t decreaseCapacity(uint64_t toDecrease, Function&& removeHandler) noexcept;

    /// @brief      try to get a used index
    /// @note the underlying strategy can change later, there are several reasonable alternatives
    bool tryGetUsedIndex(BufferIndex& index) noexcept;

    template <typename T>
    iox::cxx::optional<ElementType> pushImpl(T&& value) noexcept;
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/lockfree_queue/resizeable_lockfree_queue.inl"

#endif // IOX_UTILS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
