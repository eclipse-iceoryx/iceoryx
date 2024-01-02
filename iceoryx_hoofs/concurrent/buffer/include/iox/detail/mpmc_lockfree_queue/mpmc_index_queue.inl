// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_MPMC_INDEX_QUEUE_INL
#define IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_MPMC_INDEX_QUEUE_INL

#include "iox/detail/mpmc_lockfree_queue/mpmc_index_queue.hpp"

namespace iox
{
namespace concurrent
{
template <uint64_t Capacity, typename ValueType>
inline MpmcIndexQueue<Capacity, ValueType>::MpmcIndexQueue(ConstructEmpty_t) noexcept
    : m_readPosition(Index(Capacity))
    , m_writePosition(Index(Capacity))
{
    for (uint64_t i = 0U; i < Capacity; ++i)
    {
        m_cells[i].store(Index(0U), std::memory_order_relaxed);
    }
}

template <uint64_t Capacity, typename ValueType>
inline MpmcIndexQueue<Capacity, ValueType>::MpmcIndexQueue(ConstructFull_t) noexcept
    : m_readPosition(Index(0U))
    , m_writePosition(Index(Capacity))
{
    for (uint64_t i = 0U; i < Capacity; ++i)
    {
        m_cells[i].store(Index(i), std::memory_order_relaxed);
    }
}

template <uint64_t Capacity, typename ValueType>
inline constexpr uint64_t MpmcIndexQueue<Capacity, ValueType>::capacity() const noexcept
{
    return Capacity;
}

template <uint64_t Capacity, typename ValueType>
inline void MpmcIndexQueue<Capacity, ValueType>::push(const ValueType index) noexcept
{
    // we need the CAS loop here since we may fail due to concurrent push operations
    // note that we are always able to succeed to publish since we have
    // enough capacity for all unique indices used

    // case analyis
    // (1) loaded value is exactly one cycle behind:
    //     value is from the last cycle
    //     we can try to publish
    // (2) loaded value has the same cycle:
    //     some other push has published but not updated the write position
    //     help updating the write position
    // (3) loaded value is more than one cycle behind:
    //     this should only happen due to wrap around when push is interrupted for a long time
    //     reload write position and try again
    //     note that a complete wraparound can lead to a false detection of 1) (ABA problem)
    //     but this is very unlikely with e.g. a 64 bit value type
    // (4) loaded value is some cycle ahead:
    //     write position is outdated, there must have been other pushes concurrently
    //     reload write position and try again

    constexpr bool NotPublished = true;

    auto writePosition = m_writePosition.load(std::memory_order_relaxed);
    do
    {
        auto oldValue = loadvalueAt(writePosition, std::memory_order_relaxed);

        auto cellIsFree = oldValue.isOneCycleBehind(writePosition);

        if (cellIsFree)
        {
            // case (1)
            Index newValue(index, writePosition.getCycle());

            // if publish fails, another thread has published before us
            bool published = m_cells[writePosition.getIndex()].compare_exchange_weak(
                oldValue, newValue, std::memory_order_relaxed, std::memory_order_relaxed);

            if (published)
            {
                break;
            }
        }

        // even if we are not able to publish, we check whether some other push has already updated the writePosition
        // before trying again to publish
        auto writePositionRequiresUpdate = oldValue.getCycle() == writePosition.getCycle();

        if (writePositionRequiresUpdate)
        {
            // case (2)
            // the writePosition was not updated yet by another push but the value was already written
            // help with the update
            // (note that we do not care if it fails, then a retry or another push will handle it)

            Index newWritePosition(writePosition + 1U);
            m_writePosition.compare_exchange_strong(
                writePosition, newWritePosition, std::memory_order_relaxed, std::memory_order_relaxed);
        }
        else
        {
            // case (3) and (4)
            // note: we do not update with CAS here, the CAS is bound to fail anyway
            // (since our value of writePosition is not up to date so needs to be loaded again)
            writePosition = m_writePosition.load(std::memory_order_relaxed);
        }

    } while (NotPublished);

    Index newWritePosition(writePosition + 1U);

    // if this compare-exchange fails it is no problem, this only delays the update of m_writePosition
    // for other pushes which are able to do them on their own (if writePositionRequiresUpdate above is true)
    // no one else except popIfFull requires this update:
    // In this case it is also ok: the push is only complete once this update of m_writePosition was executed,
    // and the queue (logically) cannot be full until this happens.
    m_writePosition.compare_exchange_strong(
        writePosition, newWritePosition, std::memory_order_relaxed, std::memory_order_relaxed);
}

template <uint64_t Capacity, typename ValueType>
inline bool MpmcIndexQueue<Capacity, ValueType>::pop(ValueType& index) noexcept
{
    // we need the CAS loop here since we may fail due to concurrent pop operations
    // we leave when we detect an empty queue, otherwise we retry the pop operation

    // case analyis
    // (1) loaded value has the same cycle:
    //     value was not popped before
    //     try to get ownership
    // (2) loaded value is exactly one cycle behind:
    //     value is from the last cycle which means the queue is empty
    //     return false
    // (3) loaded value is more than one cycle behind:
    //     this should only happen due to wrap around when push is interrupted for a long time
    //     reload read position and try again
    // (4) loaded value is some cycle ahead:
    //     read position is outdated, there must have been pushes concurrently
    //     reload read position and try again

    bool ownershipGained = false;
    Index value;
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    do
    {
        value = loadvalueAt(readPosition, std::memory_order_relaxed);

        // we only dequeue if value and readPosition are in the same cycle
        auto cellIsValidToRead = readPosition.getCycle() == value.getCycle();

        if (cellIsValidToRead)
        {
            // case (1)
            Index newReadPosition(readPosition + 1U);
            ownershipGained = m_readPosition.compare_exchange_weak(
                readPosition, newReadPosition, std::memory_order_relaxed, std::memory_order_relaxed);
        }
        else
        {
            // readPosition is ahead by one cycle, queue was empty at value load
            auto isEmpty = value.isOneCycleBehind(readPosition);

            if (isEmpty)
            {
                // case (2)
                return false;
            }

            // case (3) and (4) requires loading readPosition again
            readPosition = m_readPosition.load(std::memory_order_relaxed);
        }

        // readPosition is outdated, retry operation

    } while (!ownershipGained); // we leave if we gain ownership of readPosition

    index = value.getIndex();
    return true;
}

template <uint64_t Capacity, typename ValueType>
inline bool MpmcIndexQueue<Capacity, ValueType>::popIfFull(ValueType& index) noexcept
{
    // we do NOT need a CAS loop here since if we detect that the queue is not full
    // someone else popped an element and we do not retry to check whether it was filled AGAIN
    // concurrently (which will usually not be the case and then we would return false anyway)
    // if it is filled again we can (and will) retry popIfFull from the call site

    // the queue is full if and only if write position and read position are the same but read position is
    // one cycle behind write position
    // unfortunately it seems impossible in this design to check this condition without loading
    // write posiion and read position (which causes more contention)

    const auto writePosition = m_writePosition.load(std::memory_order_relaxed);
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    const auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

    auto isFull = writePosition.getIndex() == readPosition.getIndex() && readPosition.isOneCycleBehind(writePosition);

    if (isFull)
    {
        Index newReadPosition(readPosition + 1U);
        auto ownershipGained = m_readPosition.compare_exchange_strong(
            readPosition, newReadPosition, std::memory_order_relaxed, std::memory_order_relaxed);

        if (ownershipGained)
        {
            index = value.getIndex();
            return true;
        }
    }

    // otherwise someone else has dequeued an index and the queue was not full at the start of this popIfFull
    return false;
}

template <uint64_t Capacity, typename ValueType>
inline bool MpmcIndexQueue<Capacity, ValueType>::popIfSizeIsAtLeast(const uint64_t minSize, ValueType& index) noexcept
{
    if (minSize == 0)
    {
        return pop(index);
    }
    // which to load first should make no difference for correctness
    // but for performance it might
    // note that without sync mechanisms (such as seq_cst), reordering is possible
    const auto writePosition = m_writePosition.load(std::memory_order_relaxed);
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);

    // if readPosition + n = readPosition for some n>=0, the queue contains n elements
    // at this instant (!) but slightly later may contain more or less elements
    // while the m_readPosition and m_writePosition can grow during this operation,
    // we detect this for readPosition with compare_exchange and for writePosition it does not matter,
    // the queue will contain even more elements then ( > n)
    const int64_t delta = writePosition - readPosition;

    // delta < 0 can actually happen (atomic values may not be up to date, i.e. detect writePosition as smaller than
    // readPosition leading to negative delta)
    // since we cannot conclude that the queue is filled with requiredSize elements in this case we just return
    //
    // note that delta is signed and we cannot compare it to requiredSize (unsigned) when it is negative
    // without getting unexpected results (it will be converted to large positive numbers)
    if (delta < 0)
    {
        return false;
    }

    // delta is positive, therefore the conversion is fine (it surely fits into uint64_t)
    if (static_cast<uint64_t>(delta) >= minSize)
    {
        auto value = loadvalueAt(readPosition, std::memory_order_relaxed);
        Index newReadPosition(readPosition + 1U);
        auto ownershipGained = m_readPosition.compare_exchange_strong(
            readPosition, newReadPosition, std::memory_order_relaxed, std::memory_order_relaxed);
        if (ownershipGained)
        {
            index = value.getIndex();
            return true;
        }
    }

    return false;
}

template <uint64_t Capacity, typename ValueType>
inline optional<ValueType> MpmcIndexQueue<Capacity, ValueType>::pop() noexcept
{
    ValueType value;
    if (pop(value))
    {
        return value;
    }
    return nullopt;
}

template <uint64_t Capacity, typename ValueType>
inline optional<ValueType> MpmcIndexQueue<Capacity, ValueType>::popIfFull() noexcept
{
    ValueType value;
    if (popIfFull(value))
    {
        return value;
    }
    return nullopt;
}

template <uint64_t Capacity, typename ValueType>
inline optional<ValueType> MpmcIndexQueue<Capacity, ValueType>::popIfSizeIsAtLeast(const uint64_t size) noexcept
{
    ValueType value;
    if (popIfSizeIsAtLeast(size, value))
    {
        return value;
    }
    return nullopt;
}

template <uint64_t Capacity, typename ValueType>
inline bool MpmcIndexQueue<Capacity, ValueType>::empty() const noexcept
{
    const auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    const auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

    // if m_readPosition is ahead by one cycle compared to the value stored at head,
    // the queue was empty at the time of the loads above (but might not be anymore!)
    return value.isOneCycleBehind(readPosition);
}

template <uint64_t Capacity, typename ValueType>
typename MpmcIndexQueue<Capacity, ValueType>::Index inline MpmcIndexQueue<Capacity, ValueType>::loadvalueAt(
    const Index& position, const std::memory_order memoryOrder) const noexcept
{
    return m_cells[position.getIndex()].load(memoryOrder);
}

} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_MPMC_INDEX_QUEUE_INL
