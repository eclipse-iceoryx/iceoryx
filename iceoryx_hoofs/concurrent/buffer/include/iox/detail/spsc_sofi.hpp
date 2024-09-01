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
#include "iox/atomic.hpp"
#include "iox/type_traits.hpp"
#include "iox/uninitialized_array.hpp"

#include <cstdint>
#include <cstring>
#include <utility>

namespace iox
{
namespace concurrent
{
/// @brief Thread safe lock-free single producer and single consumer queue with a safe
/// overflowing behavior
/// @note When SpscSoFi is full and a sender tries to push, the data at the current read pos will be
/// returned. This behavior mimics a FiFo queue but prevents resource leaks when pushing into
/// a full SpscSoFi.
/// SpscSoFi is especially designed to provide fixed capacity storage.
/// It's an expected behavior that when push/pop are called concurrently and SpscSoFi is full, as
/// many elements as specified with 'CapacityValue' can be removed
/// @param[in] ValueType        DataType to be stored, must be trivially copyable
/// @param[in] CapacityValue    Capacity of the SpscSofi
template <class ValueType, uint64_t CapacityValue>
class SpscSofi
{
    static_assert(std::is_trivially_copyable<ValueType>::value,
                  "SpscSofi can only handle trivially copyable data types since 'memcpy' is used internally");
    /// @brief Check if Atomic integer is lockfree on platform
    /// ATOMIC_INT_LOCK_FREE = 2 - is always lockfree
    /// ATOMIC_INT_LOCK_FREE = 1 - is sometimes lockfree
    /// ATOMIC_INT_LOCK_FREE = 0 - is never lockfree
    static_assert(2 <= ATOMIC_INT_LOCK_FREE, "SpscSofi is not able to run lock free on this data type");

    // To ensure a consumer gets at least the amount of capacity of data when a queue is full, an additional free
    // slot (add-on) is required.
    // ========================================================================
    // Consider the following scenario when there is no "capacity add-on":
    // 1. CapacityValue = 2
    //    |--A--|--B--|
    //    ^
    //    w=2, r=0
    // 2. The producer thread pushes a new element
    // 3. Increment the read position (this effectively reduces the capacity and is the reason the internal capacity
    // needs to be larger;
    //    |--A--|--B--|
    //    ^     ^
    //    w=2  r=1
    // 4. The producer thread is suspended, the consumer thread pops a value
    //    |--A--|-----|
    //    ^
    //    w=2, r=2
    // 5. The consumer tries to pop another value but the queue looks empty as
    //    write position == read position: the consumer cannot pop
    //    out CAPACITY amount of samples even though the queue was full
    // ========================================================================
    // With "capacity add-on"
    // 1. CapacityValue = 2, InternalCapacity = 3
    //    |--A--|--B--|----|
    //    ^           ^
    //    r=0        w=2
    // 2. The producer threads pushes a new element
    // 3. First write the element at index 2 % capacity and increment the write index
    //    |--A--|--B--|--C--|
    //    ^
    //   w=3, r=0,
    // 4. Then increment the read position and return the overflowing 'A'
    //   |-----|--B--|--C--|
    //   ^     ^
    //   w=3  r=1
    // 5. The producer thread is suspended, the consumer thread pops a value
    //   |--A--|-----|--C--|
    //   ^           ^
    //   w=3        r=2
    // 6. The consumer thread pops another value
    //   |--A--|-----|-----|
    //   ^
    //   w=3, r=3
    // 7. Now, write position == read position so we cannot pop another element: the queue looks empty. We managed to
    // pop CapacityValue elements
    // ========================================================================
    static constexpr uint32_t INTERNAL_CAPACITY_ADDON = 1;

    /// @brief Internal capacity of the queue at creation
    static constexpr uint32_t INTERNAL_SPSC_SOFI_CAPACITY = CapacityValue + INTERNAL_CAPACITY_ADDON;

  public:
    /// @brief default constructor which constructs an empty SpscSofi
    SpscSofi() noexcept = default;

    /// @brief push an element into SpscSofi. if SpscSofi is full the oldest data will be
    ///         returned and the pushed element is stored in its place instead.
    /// @param[in] value_in value which should be stored
    /// @param[out] value_out if SpscSofi is overflowing  the value of the overridden value
    ///                      is stored here
    /// @note restricted thread safe: can only be called from one thread. The authorization to push into the
    /// SpscSofi can be transferred to another thread if appropriate synchronization mechanisms are used.
    /// @return return true if push was successful else false.
    /// @remarks
    /// 1. SpscSofi is empty    |-----|-----|
    /// 2. push an element  |--A--|-----|
    /// 3. push an element  |--A--|--B--|
    /// 5. SpscSofi is full
    /// 6. push an element  |--C--|--B--| -> value_out is set to 'A'
    bool push(const ValueType& valueIn, ValueType& valueOut) noexcept;

    /// @brief pop the oldest element
    /// @param[out] valueOut storage of the pop'ed value
    /// @concurrent restricted thread safe: can only be called from one thread. The authorization to pop from the
    /// SpscSofi can be transferred to another thread if appropriate synchronization mechanisms are used.
    /// @return false if SpscSofi is empty, otherwise true
    bool pop(ValueType& valueOut) noexcept;

    /// @brief returns true if SpscSofi is empty, otherwise false
    /// @note the use of this function is limited in the concurrency case. if you
    ///         call this and in another thread pop is called the result can be out
    ///         of date as soon as you require it
    /// @concurrent unrestricted thread safe (the result might already be outdated when used). Expected to be called
    /// from either the producer or the consumer thread but not from a third thread
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
    /// @concurrent unrestricted thread safe (the result might already be outdated when used). Expected to be called
    /// from either the producer or the consumer thread but not from a third thread
    uint64_t size() const noexcept;

  private:
    std::pair<uint64_t, uint64_t> getReadWritePositions() const noexcept;

  private:
    UninitializedArray<ValueType, INTERNAL_SPSC_SOFI_CAPACITY> m_data;
    uint64_t m_size = INTERNAL_SPSC_SOFI_CAPACITY;
    Atomic<uint64_t> m_readPosition{0};
    Atomic<uint64_t> m_writePosition{0};
};

} // namespace concurrent
} // namespace iox

#include "iox/detail/spsc_sofi.inl"

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_HPP
