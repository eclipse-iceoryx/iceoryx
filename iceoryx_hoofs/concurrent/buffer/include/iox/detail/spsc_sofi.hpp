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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_HPP
#define IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_HPP

#include "iceoryx_platform/platform_correction.hpp"
#include "iox/type_traits.hpp"
#include "iox/uninitialized_array.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <utility>

namespace iox
{
namespace concurrent
{
/// @brief Thread safe (without locks) single producer and single consumer queue with a safe
/// overflowing behavior
/// @note When SoFi is full and a sender tries to push, the data at the current read pos will be
/// returned. This behavior mimics a FiFo queue but prevents data loss when pushing into
/// a full SoFi.
/// SoFi is especially designed to provide fixed capacity storage.
/// SoFi only allocates memory when created, capacity can be adjusted explicitly.
/// @example
/// It's an expected behavior that when push/pop are called concurrently and SoFi is full, 2
/// elements can be removed
/// 0: Initial situation:
///    |--1--|--2--|
/// 1. Thread 1 pushes a new element. Since it is an overflowing situation, the overwritten value is
/// removed and returned to the caller
///    |--3--|--2--|
/// 2. Right before push() returns, pop() detects that an element is about to be removed, and remove
/// the next element
///    |--3--|----|
/// @param[in] ValueType        DataType to be stored, must be trivially copyable
/// @param[in] CapacityValue    Capacity of the SpscSofi
template <class ValueType, uint64_t CapacityValue>
class SpscSofi
{
    // We need to make sure that the copy operation doesn't have any logic
    static_assert(std::is_trivially_copyable<ValueType>::value,
                  "SpscSofi can handle only trivially copyable data types");
    /// @brief Check if Atomic integer is lockfree on platform
    /// ATOMIC_INT_LOCK_FREE = 2 - is always lockfree
    /// ATOMIC_INT_LOCK_FREE = 1 - is sometimes lockfree
    /// ATOMIC_INT_LOCK_FREE = 0 - is never lockfree
    static_assert(2 <= ATOMIC_INT_LOCK_FREE, "SpscSofi is not able to run lock free on this data type");

    // A limitation of the current implementation (first write and then read in the push() method) is
    // that the internal capacity needs to be larger than the capacity desired by the user to fulfill
    // the contract that the SoFi should always be able to store at least CapacityValue elements.
    // ========================================================================
    // Consider the following scenario when there is no "capacity add-on":
    // 1. CapacityValue = 2
    //    |--1--|--2--|
    //    ^
    //    w=2, r=0
    // 2. We want to push a new element:
    //     - we first write at index 2 % size
    //     - we detect that the queue if full so we want to read at r=0
    //     - we cannot read anymore the value since we've just overwritten it
    // ========================================================================
    // With "capacity add-on"
    // 1. CapacityValue = 2, InternalCapacity = 3
    //    |--1--|--2--|----|
    //    ^           ^
    //    r=0        w=2
    // 2. We want to push a new element:
    //     - we first write at index 2 % size
    //     - we detect that the queue if full so we want to read at r=0
    //     - we set value_out to the value read
    //   |--1--|--2--|--3--|
    //   ^     ^
    //   w=3  r=1
    // ========================================================================
    static constexpr uint32_t INTERNAL_CAPACITY_ADDON = 1;

    /// @brief Internal capacity of the queue at creation
    static constexpr uint32_t INTERNAL_SPSC_SOFI_CAPACITY = CapacityValue + INTERNAL_CAPACITY_ADDON;

  public:
    /// @brief default constructor which constructs an empty sofi
    SpscSofi() noexcept = default;

    /// @brief push an element into sofi. if sofi is full the oldest data will be
    ///         returned and the pushed element is stored in its place instead.
    /// @param[in] value_in value which should be stored
    /// @param[out] value_out if sofi is overflowing  the value of the overridden value
    ///                      is stored here
    /// @note restricted thread safe: can only be called from one thread
    /// @return return true if push was successful else false.
    /// @code
    /// 1. sofi is empty    |-----|-----|
    /// 2. push an element  |--A--|-----|
    /// 3. push an element  |--A--|--B--|
    /// 5. sofi is full
    /// 6. push an element  |--C--|--B--| -> value_out is set to 'A'
    bool push(const ValueType& valueIn, ValueType& valueOut) noexcept;

    /// @brief pop the oldest element
    /// @param[out] valueOut storage of the pop'ed value
    /// @concurrent restricted thread safe: can only be called from one thread
    /// @return false if SpscSofi is empty, otherwise true
    bool pop(ValueType& valueOut) noexcept;

    /// @brief returns true if SpscSofi is empty, otherwise false
    /// @note the use of this function is limited in the concurrency case. if you
    ///         call this and in another thread pop is called the result can be out
    ///         of date as soon as you require it
    /// @concurrent unrestricted thread safe (the result might already be outdated when used)
    bool empty() const noexcept;

    /// @brief resizes SpscSofi
    /// @param[in] newSize valid values are 0 < newSize < CapacityValue
    /// @pre it is important that no pop or push calls occur during
    ///         this call
    /// @concurrent not thread safe
    bool setCapacity(const uint64_t newSize) noexcept;

    /// @brief returns the capacity of SpscSofi
    /// @concurrent unrestricted thread safe
    uint64_t capacity() const noexcept;

    /// @brief returns the current size of SpscSofi
    /// @concurrent unrestricted thread safe (the result might already be outdated when used)
    uint64_t size() const noexcept;

  private:
    std::pair<uint64_t, uint64_t> getReadWritePositions() const noexcept;

    UninitializedArray<ValueType, INTERNAL_SPSC_SOFI_CAPACITY> m_data;
    uint64_t m_size = INTERNAL_SPSC_SOFI_CAPACITY;

    std::atomic<uint64_t> m_readPosition{0};
    std::atomic<uint64_t> m_writePosition{0};
};

} // namespace concurrent
} // namespace iox

#include "iox/detail/spsc_sofi.inl"

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_HPP
