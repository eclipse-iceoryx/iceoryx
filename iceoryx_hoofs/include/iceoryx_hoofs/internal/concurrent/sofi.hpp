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
#ifndef IOX_HOOFS_CONCURRENT_SOFI_HPP
#define IOX_HOOFS_CONCURRENT_SOFI_HPP

#include "iceoryx_platform/platform_correction.hpp"
#include "iox/type_traits.hpp"
#include "iox/uninitialized_array.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>

namespace iox
{
namespace concurrent
{
/// @brief
/// Thread safe producer and consumer queue with a safe overflowing behavior.
/// SoFi is designed in a FIFO Manner but prevents data loss when pushing into
/// a full SoFi. When SoFi is full and a Sender tries to push, the data at the
/// current read position will be returned. SoFi is a Thread safe without using
/// locks. When the buffer is filled, new data is written starting at the
/// beginning of the buffer and overwriting the old.The SoFi is especially
/// designed to provide fixed capacity storage. When its capacity is exhausted,
/// newly inserted elements will cause elements either at the beginning
/// to be overwritten.The SoFi only allocates memory when
/// created , capacity can be is adjusted explicitly.
///
/// @param[in] ValueType        DataType to be stored, must be trivially copyable
/// @param[in] CapacityValue    Capacity of the SoFi
template <class ValueType, uint64_t CapacityValue>
class SoFi
{
    static_assert(std::is_trivially_copyable<ValueType>::value, "SoFi can handle only trivially copyable data types");
    /// @brief Check if Atomic integer is lockfree on platform
    /// ATOMIC_INT_LOCK_FREE = 2 - is always lockfree
    /// ATOMIC_INT_LOCK_FREE = 1 - is sometimes lockfree
    /// ATOMIC_INT_LOCK_FREE = 0 - is never lockfree
    static_assert(2 <= ATOMIC_INT_LOCK_FREE, "SoFi is not able to run lock free on this data type");

    /// @brief Internal size needs to be bigger than the size desirred by the user
    /// This is because of buffer empty detection and overflow handling
    static constexpr uint32_t INTERNAL_SIZE_ADD_ON = 1;

    /// @brief This is the resulting internal size on creation
    static constexpr uint32_t INTERNAL_SOFI_SIZE = CapacityValue + INTERNAL_SIZE_ADD_ON;

  public:
    /// @brief default constructor which constructs an empty sofi
    SoFi() noexcept = default;

    /// @brief pushs an element into sofi. if sofi is full the oldest data will be
    ///         returned and the pushed element is stored in its place instead.
    /// @param[in] valueIn value which should be stored
    /// @param[out] valueOut if sofi is overflowing  the value of the overridden value
    ///                      is stored here
    /// @concurrent restricted thread safe: single pop, single push no
    ///             push calls from multiple contexts
    /// @return return true if push was sucessfull else false.
    /// @code
    /// (initial situation, SOFI is FULL)
    ///     Start|-----A-------|
    ///                        |-----B-------|
    ///                                      |-----C-------|
    ///                                                    |-----D-------|
    ///
    ///
    /// (calling push with data ’E’)
    ///     Start|-----E-------|
    ///                        |-----A-------|
    ///                                      |-----B-------|
    ///                                                    |-----C-------|
    ///                                     (’D’ is returned as valueOut)
    ///
    /// ###################################################################
    ///
    /// (if SOFI is not FULL , calling push() add new data)
    ///     Start|-------------|
    ///                        |-------------|  ( Initial SOFI )
    ///  (push() Called two times)
    ///
    ///                                      |-------------|
    ///                                      (New Data)
    ///                                                     |-------------|
    ///                                                      (New Data)
    /// @endcode
    bool push(const ValueType& valueIn, ValueType& valueOut) noexcept;

    /// @brief pop the oldest element
    /// @param[out] valueOut storage of the pop'ed value
    /// @concurrent restricted thread safe: single pop, single push no
    ///             pop or popIf calls from multiple contexts
    /// @return false if sofi is empty, otherwise true
    bool pop(ValueType& valueOut) noexcept;

    /// @brief conditional pop call to provide an alternative for a peek
    ///         and pop approach. If the verificator returns true the
    ///         peeked element is returned.
    /// @param[out] valueOut storage of the pop'ed value
    /// @param[in] verificator callable of type bool(const ValueType& peekValue)
    ///             which takes the value which would be pop'ed as argument and returns
    ///             true if it should be pop'ed, otherwise false
    /// @code
    ///     int limit = 7128;
    ///     mysofi.popIf(value, [=](const ValueType & peek)
    ///         {
    ///             return peek < limit; // pop only when peek is smaller than limit
    ///         }
    ///     ); // pop's a value only if it is smaller than 9012
    /// @endcode
    /// @concurrent restricted thread safe: single pop, single push no
    ///             pop or popIf calls from multiple contexts
    /// @return false if sofi is empty or when verificator returns false, otherwise true
    template <typename Verificator_T>
    bool popIf(ValueType& valueOut, const Verificator_T& verificator) noexcept;

    /// @brief returns true if sofi is empty, otherwise false
    /// @note the use of this function is limited in the concurrency case. if you
    ///         call this and in another thread pop is called the result can be out
    ///         of date as soon as you require it
    /// @concurrent unrestricted thread safe
    bool empty() const noexcept;

    /// @brief resizes sofi
    /// @param[in] newSize valid values are 0 < newSize < CapacityValue
    /// @pre it is important that no pop or push calls occur during
    ///         this call
    /// @concurrent not thread safe
    bool setCapacity(const uint64_t newSize) noexcept;

    /// @brief returns the capacity of sofi
    /// @concurrent unrestricted thread safe
    uint64_t capacity() const noexcept;

    /// @brief returns the current size of sofi
    /// @concurrent unrestricted thread safe
    uint64_t size() const noexcept;

  private:
    UninitializedArray<ValueType, INTERNAL_SOFI_SIZE> m_data;
    uint64_t m_size = INTERNAL_SOFI_SIZE;

    /// @brief the write/read pointers are "atomic pointers" so that they are not
    /// reordered (read or written too late)
    std::atomic<uint64_t> m_readPosition{0};
    std::atomic<uint64_t> m_writePosition{0};
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_hoofs/internal/concurrent/sofi.inl"

#endif // IOX_HOOFS_CONCURRENT_SOFI_HPP
