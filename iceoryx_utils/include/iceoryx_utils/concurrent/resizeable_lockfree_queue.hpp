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

#pragma once

#include "iceoryx_utils/concurrent/lockfree_queue.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <atomic>

namespace iox
{
namespace concurrent
{
// Design Goal: have both a fixed size queue and a  resizable queue to be chosen when needed.
//(i.e. no unnecessary "one size fits all" solution)

// we could also use composition (more boilerplate for redirection),
// but if the code requires (major) interface changes this may be needed
// for now we prefer inheritance here since we (mainly) extend functionality
// We do not want to use virtual functions to be able to use the class in shared memory.
template <typename ElementType, uint64_t Capacity>
class ResizeableLockFreeQueue : public LockFreeQueue<ElementType, Capacity>
{
  private:
    using Base = LockFreeQueue<ElementType, Capacity>;

  public:
    static constexpr uint64_t MAX_CAPACITY = Capacity;

    ResizeableLockFreeQueue() = default;

    ResizeableLockFreeQueue(uint64_t initialCapacity) noexcept;

    static constexpr uint64_t maxCapacity() noexcept;

    //********************************************************************************
    // reimplement the differing parts of the interface (shadowing is intentional)
    // note that this has to be done since the logic to detect a full queue is different
    // we also cannot use virtual functions since we want to use the queue in shared memory
    //********************************************************************************

    uint64_t capacity() const noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted is copied into the queue
    /// @return removed value if an overflow occured, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<ElementType> push(const ElementType& value) noexcept;

    /// @brief inserts value in FIFO order, always succeeds by removing the oldest value
    /// when the queue is detected to be full (overflow)
    /// @param value to be inserted is moved into the queue if possible
    /// @return removed value if an overflow occured, empty optional otherwise
    /// threadsafe, lockfree
    iox::cxx::optional<ElementType> push(ElementType&& value) noexcept;

    //********************************************************************************
    // extend interface with the resize functionaility
    //********************************************************************************

    /// @brief Set the capacity to a new Capacity between 0 and Capacity, if the capacity is reduced
    /// it may be necessary to remove the least recent elements.
    /// @param[in] newCapacity new capacity to be set, if it is larger than Capacity the call fails
    /// @param[out] removedElements container were potentially removed elements can be stored.
    /// @return true setting if the new capacity was successful, false otherwise (newCapacity > Capacity)
    template <typename ContainerType = iox::cxx::vector<ElementType, Capacity>>
    bool setCapacity(uint64_t newCapacity, ContainerType& removedElements) noexcept;

    /// @brief Set the capacity to a new Capacity between 0 and Capacity, if the capacity is reduced
    /// it may be necessary to remove the least recent elements which are then discarded.
    /// @param[in] newCapacity new capacity to be set, if it is larger than Capacity the call fails
    /// @return true setting if the new capacity was successful, false otherwise (newCapacity > Capacity)
    bool setCapacity(uint64_t newCapacity) noexcept;

  private:
    using Queue = typename iox::concurrent::LockFreeQueue<ElementType, Capacity>;
    using BufferIndex = typename Queue::BufferIndex;

    std::atomic<uint64_t> m_capacity{Capacity};

    // we also sync m_capacity with this
    std::atomic_flag m_resizeInProgress{false};

    // The vector is protected by the atomic flag, but this also means dying during a resize
    // will prevent further resizes.
    // I.e. resize is lockfree, but not in a useful and robust way as it assumes that a concurrent resize will
    // always eventually complete (which is true when the application does not die and the relevant thread is scheduled
    // eventually. The latter is the case for any OS and mandatory for a Realtime OS.

    iox::cxx::vector<BufferIndex, Capacity> m_unusedIndices;

    /// @brief      Increase the capacity by some value.
    /// @param[in]  toIncrease value by which the capacity is to be increased
    /// @return     value by which the capacity was actually increased
    /// @note       If incrementing cannot be carried out (because the Capacity was reached),
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

    /// @brief      Set the capacity to some value.
    /// @param[in]  newCapacity capacity to be set
    /// @param[in]  removedHandler is a function taking an index which specifies
    ///             what to do with removed element should the need for removal arise.
    /// @return     true if the capacity was successfully set, false otherwise
    template <typename Function>
    bool setCapacityImpl(uint64_t newCapacity, Function&& removeHandler) noexcept;

    /// @brief      try to get a used index
    /// @note the underlying strategy can change later, there are several reasonable alternatives
    bool tryGetUsedIndex(BufferIndex& index) noexcept;

    template <typename T>
    iox::cxx::optional<ElementType> pushImpl(T&& value) noexcept;
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/lockfree_queue/resizeable_lockfree_queue.inl"
