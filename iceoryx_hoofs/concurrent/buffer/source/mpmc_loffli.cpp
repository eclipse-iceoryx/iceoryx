// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/detail/mpmc_loffli.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iox/assertions.hpp"

namespace iox
{
namespace concurrent
{
void MpmcLoFFLi::init(not_null<Index_t*> freeIndicesMemory, const uint32_t capacity) noexcept
{
    IOX_ENFORCE(capacity > 0, "A capacity of 0 is not supported!");
    constexpr uint32_t INTERNALLY_RESERVED_INDICES{1U};
    IOX_ENFORCE(capacity < (std::numeric_limits<Index_t>::max() - INTERNALLY_RESERVED_INDICES),
                "Requested capacity exceeds limits!");

    m_nextFreeIndex = freeIndicesMemory;
    m_size = capacity;
    m_invalidIndex = m_size + 1;

    if (m_nextFreeIndex != nullptr)
    {
        for (uint32_t i = 0; i < m_size + 1; i++)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) upper limit of index is set by m_size
            m_nextFreeIndex.get()[i] = i + 1;
        }
    }
}

bool MpmcLoFFLi::pop(Index_t& index) noexcept
{
    Node oldHead = m_head.load(std::memory_order_acquire);
    Node newHead = oldHead;

    do
    {
        // we are empty if next points to an element with index of Size
        if (oldHead.indexToNextFreeIndex >= m_size || !m_nextFreeIndex)
        {
            return false;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) upper limit of index set by m_size
        newHead.indexToNextFreeIndex = m_nextFreeIndex.get()[oldHead.indexToNextFreeIndex];
        newHead.abaCounter += 1;
    } while (!m_head.compare_exchange_weak(oldHead, newHead, std::memory_order_acq_rel, std::memory_order_acquire));

    /// comes from outside, is not shared and therefore no synchronization is needed
    index = oldHead.indexToNextFreeIndex;
    /// What if interrupted here an another thread guesses the index and calls push?
    /// @brief murphy case: m_nextFreeIndex does not require any synchronization since it
    ///         either is used by the same thread in push or it is given to another
    ///         thread which performs the cleanup and during this process a synchronization
    ///         is required
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    m_nextFreeIndex.get()[index] = m_invalidIndex;

    /// we need to synchronize m_nextFreeIndex with push so that we can perform a validation
    /// check right before push to avoid double free's;
    /// a simple fence without explicit atomic store should be sufficient since the index needs to be transferred to
    /// another thread and the most simple method would be a relaxed store of the index in the 'pop' thread and a
    /// relaxed load of the same atomic in the 'push' thread which would be the atomic in the fence to fence
    /// synchronization; other mechanism would involve stronger synchronizations and implicitly also synchronize
    /// m_nextFreeIndex
    std::atomic_thread_fence(std::memory_order_release);

    return true;
}

bool MpmcLoFFLi::push(const Index_t index) noexcept
{
    /// we synchronize with m_nextFreeIndex in pop to perform the validity check
    std::atomic_thread_fence(std::memory_order_acquire);

    /// we want to avoid double free's therefore we check if the index was acquired
    /// in pop and the push argument "index" is valid
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) index is limited by capacity
    if (index >= m_size || !m_nextFreeIndex || m_nextFreeIndex.get()[index] != m_invalidIndex)
    {
        return false;
    }

    Node oldHead = m_head.load(std::memory_order_acquire);
    Node newHead = oldHead;

    do
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) index is limited by capacity
        m_nextFreeIndex.get()[index] = oldHead.indexToNextFreeIndex;
        newHead.indexToNextFreeIndex = index;
        newHead.abaCounter += 1;
    } while (!m_head.compare_exchange_weak(oldHead, newHead, std::memory_order_acq_rel, std::memory_order_acquire));

    return true;
}

} // namespace concurrent
} // namespace iox
