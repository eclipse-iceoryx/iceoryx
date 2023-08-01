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

#ifndef IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
#define IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP

#include "iceoryx_hoofs/concurrent/lockfree_queue.hpp"
#include "iox/type_traits.hpp"
#include "iox/vector.hpp"

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

    ResizeableLockFreeQueue() noexcept = default;
    ~ResizeableLockFreeQueue() noexcept = default;

    // deleted for now, can be implemented later if needed
    // note: concurrent copying or moving in lockfree fashion is nontrivial
    ResizeableLockFreeQueue(const ResizeableLockFreeQueue&) = delete;
    ResizeableLockFreeQueue(ResizeableLockFreeQueue&&) = delete;
    ResizeableLockFreeQueue& operator=(const ResizeableLockFreeQueue&) = delete;
    ResizeableLockFreeQueue& operator=(ResizeableLockFreeQueue&&) = delete;

    explicit ResizeableLockFreeQueue(const uint64_t initialCapacity) noexcept;

    /// @brief returns the maximum capacity of the queue
    /// @return the maximum capacity
    static constexpr uint64_t maxCapacity() noexcept;

    using Base::empty;
    using Base::pop;
    using Base::size;
    using Base::tryPush;

    /// @brief returns the current capacity of the queue
    /// @return the current capacity
    /// @note threadsafe, lockfree
    uint64_t capacity() const noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param[in] value to be inserted is copied into the queue
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::optional<ElementType> push(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param[in] value to be inserted is moved into the queue if possible
    /// @return removed value if an overflow occured, empty optional otherwise
    /// @note threadsafe, lockfree
    iox::optional<ElementType> push(ElementType&& value) noexcept;

    // overloads to set the capacity
    // 1) The most general one allows providing a removeHandler to specify remove behavior.
    //    This could e.g. be to store them in a container.
    // 2) The second overload discards removed elements.

    /// @note setCapacity is lockfree, but if an application crashes during setCapacity it
    ///       currently may prevent other applications from setting the capacity (they will not block though).
    ///       This is not a problem if for example there is only one application calling setCapacity or
    ///       setCapacity is only called from vital applications (which if they crash will lead to system shutdown)
    ///       and there is only one (non-vital, i.e. allowed to crash) application reading the data via pop.
    ///       The reader application may also call setCapacity, since if it crashes there is no one reading
    ///       the data and the capacity can be considered meaningless.

    /// @brief      Set the capacity to some value.
    /// @param[in]  newCapacity capacity to be set
    /// @param[in]  removeHandler is a function taking an element which specifies
    ///             what to do with removed elements should the need for removal arise.
    /// @return     true if the capacity was successfully set, false otherwise
    template <typename Function, typename = typename std::enable_if<is_invocable<Function, ElementType>::value>::type>
    bool setCapacity(const uint64_t newCapacity, Function&& removeHandler) noexcept;

    /// @brief Set the capacity to a new capacity between 0 and MaxCapacity, if the capacity is reduced
    /// it may be necessary to remove the least recent elements which are then discarded.
    /// @param[in] newCapacity new capacity to be set, if it is larger than MaxCapacity the call fails
    /// @return true setting if the new capacity was successful, false otherwise (newCapacity > MaxCapacity)
    /// @note threadsafe, lockfree but multiple concurrent calls may have no effect
    bool setCapacity(const uint64_t newCapacity) noexcept;

  private:
    using BufferIndex = uint64_t;
    std::atomic<uint64_t> m_capacity{MaxCapacity};
    // must be operator= otherwise it is undefined, see https://en.cppreference.com/w/cpp/atomic/ATOMIC_FLAG_INIT
    std::atomic_flag m_resizeInProgress = ATOMIC_FLAG_INIT;
    iox::vector<BufferIndex, MaxCapacity> m_unusedIndices;

    /// @brief      Increase the capacity by some value.
    /// @param[in]  toIncrease value by which the capacity is to be increased
    /// @return     value by which the capacity was actually increased
    /// @note       If incrementing cannot be carried out (because the MaxCapacity was reached),
    ///             this value will be smaller than toIncrease.
    uint64_t increaseCapacity(const uint64_t toIncrease) noexcept;

    /// @brief      Decrease the capacity by some value.
    /// @param[in]  toDecrease value by which the capacity is to be decreased
    /// @param[in]  removedHandler is a function  which specifies what to do with removed elements
    ///             (e.g. store in a container or discard it).
    /// @return     value by which the capacity was actually decreased.
    /// @note       If decrementing cannot be carried out (because the capacity is already 0),
    ///             this value will be smaller than toDecrease.
    template <typename Function>
    uint64_t decreaseCapacity(const uint64_t toDecrease, Function&& removeHandler) noexcept;

    /// @brief       Try to get a used index if available.
    /// @param[out]  index index obtained in the successful case
    /// @return      true if an index was obtained, false otherwise
    bool tryGetUsedIndex(BufferIndex& index) noexcept;

    template <typename T>
    iox::optional<ElementType> pushImpl(T&& value) noexcept;
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_hoofs/internal/concurrent/lockfree_queue/resizeable_lockfree_queue.inl"

#endif // IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
