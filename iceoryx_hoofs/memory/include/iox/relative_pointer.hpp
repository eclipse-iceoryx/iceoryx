// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_RELATIVE_POINTER_HPP
#define IOX_HOOFS_MEMORY_RELATIVE_POINTER_HPP

#include "iox/detail/pointer_repository.hpp"
#include "iox/newtype.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
struct segment_id_t : public NewType<segment_id_t,
                                     uint64_t,
                                     newtype::DefaultConstructable,
                                     newtype::CopyConstructable,
                                     newtype::Convertable,
                                     newtype::ConstructByValueCopy,
                                     newtype::MoveConstructable>
{
    using ThisType::ThisType;
};

using segment_id_underlying_t = typename segment_id_t::value_type;

/// @brief pointer class to use when pointer and pointee are located in different shared memory segments
/// We can have the following scenario:
/// Pointer p is stored in segment S1 and points to object X of type T in segment S2.
///
/// Shared Memory   S1:  p              S2:  X
///                      |___________________^
/// App1            a1   b1             c1   d1
/// App2            a2   b2             c2   d2
///
/// Now it is no longer true in general that both segments will be offset by the same difference in App2 and therefore
/// relocatable pointers are no longer sufficient.
/// Relative pointers solve this problem by incorporating the information from where they need to measure differences
/// (i.e. relative to the given address). This requires an additional registration mechanism to be used by all
/// applications where the start addresses and the size of all segments to be used are registered. Since these start
/// address may differ between applications, each segment is identified by a unique id, which can be provided upon
/// registration by the first application. In the figure, this means that the starting addresses of both segments(a1, a2
/// and c1, c2) would have to be registered in both applications.
/// Once this registration is done, relative pointers can be constructed from raw pointers similar to relocatable
/// pointers.
/// @note It should be noted that relocating a memory segment will invalidate relative pointers, i.e. relative pointers
/// are NOT relocatable. This is because the registration mechanism cannot be automatically informed about the copy of a
/// whole segment, such a segment would have to be registered on its own (and the original segment deregistered).
template <typename T>
class RelativePointer final
{
  public:
    using ptr_t = T*;
    using offset_t = std::uint64_t;

    /// @brief Default constructs a RelativePointer as a logical nullptr
    RelativePointer() noexcept = default;

    ~RelativePointer() noexcept = default;

    /// @brief Constructs a RelativePointer pointing to the same pointee as ptr in a segment identified by id
    /// @param[in] ptr The pointer whose pointee shall be the same for this
    /// @param[in] id Is the unique id of the segment
    RelativePointer(ptr_t const ptr, const segment_id_t id) noexcept;

    /// @brief Constructs a RelativePointer from a given offset and segment id
    /// @param[in] offset Is the offset
    /// @param[in] id Is the unique id of the segment
    RelativePointer(const offset_t offset, const segment_id_t id) noexcept;

    /// @brief Constructs a RelativePointer pointing to the same pointee as ptr
    /// @param[in] ptr The pointer whose pointee shall be the same for this
    explicit RelativePointer(ptr_t const ptr) noexcept;

    RelativePointer(const RelativePointer& other) noexcept = default;
    RelativePointer(RelativePointer&& other) noexcept;
    RelativePointer& operator=(const RelativePointer& other) noexcept;
    // AXIVION Next Line AutosarC++19_03-A3.1.6 : False positive, this is not a trivial accessor/ mutator function but the move c'tor
    RelativePointer& operator=(RelativePointer&& other) noexcept;

    /// @brief Assigns the RelativePointer to point to the same pointee as ptr
    /// @param[in] ptr The pointer whose pointee shall be the same for this
    /// @return Reference to self
    RelativePointer& operator=(ptr_t const ptr) noexcept;

    /// @brief Dereferencing operator which returns a reference to the underlying object
    /// @tparam U a template parameter to enable the dereferencing operator only for non-void T
    /// @return A reference to the underlying object
    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const noexcept;

    /// @brief Access to the underlying object. If the RelativePointer does not point to anything the
    /// application terminates.
    /// @return A pointer to the underlying object
    T* operator->() const noexcept;

    /// @brief Access the underlying object.
    /// @return A pointer to the underlying object
    T* get() const noexcept;

    /// @brief Converts the RelativePointer to bool
    /// @return Bool which contains true if the RelativePointer contains a pointer
    // AXIVION Next Line AutosarC++19_03-A13.5.3 : Explicitly wanted to mimic the behaviour of raw pointers
    explicit operator bool() const noexcept;

    /// @brief Returns the id which identifies the segment
    /// @return The id which identifies the segment
    segment_id_underlying_t getId() const noexcept;

    /// @brief Returns the offset
    /// @return The offset of the RelativePointer object
    offset_t getOffset() const noexcept;

    /// @brief Get the base pointer associated with this' id
    /// @return The registered base pointer of the RelativePointer object
    T* getBasePtr() const noexcept;

    /// @brief Tries to registers a memory segment at ptr with size to a new id
    /// @param[in] ptr Starting address of the segment to be registered
    /// @param[in] size Is the size of the segment, defaults to size 0 if argument is not provided
    /// @return segment_id to which the pointer was registered, wrapped in an optional
    static optional<segment_id_underlying_t> registerPtr(ptr_t const ptr, const uint64_t size = 0U) noexcept;

    /// @brief Tries to register a memory segment with a given size starting at ptr to a given id
    /// @param[in] id Is the id of the segment
    /// @param[in] ptr Starting address of the segment to be registered
    /// @param[in] size Is the size of the segment
    /// @return True if successful (id not occupied), false otherwise
    static bool registerPtrWithId(const segment_id_t id, ptr_t const ptr, const uint64_t size = 0U) noexcept;

    /// @brief Unregisters ptr with given id
    /// @param[in] id Is the id of the segment
    /// @return True if successful (ptr was registered with this id before), false otherwise
    static bool unregisterPtr(const segment_id_t id) noexcept;

    /// @brief Get the base ptr associated with the given id
    /// @param[in] id Is the id of the segment
    /// @return The pointer registered at the given id, nullptr if none was registered
    static T* getBasePtr(const segment_id_t id) noexcept;

    /// @brief Unregisters all ptr id pairs leading to initial state. This affects all pointer both typed and untyped.
    static void unregisterAll() noexcept;

    /// @brief Get the offset from id and ptr
    /// @param[in] id Is the id of the segment and is used to get the base pointer
    /// @param[in] ptr Is the pointer whose offset should be calculated
    /// @return The offset of the passed pointer
    static offset_t getOffset(const segment_id_t id, ptr_t const ptr) noexcept;

    /// @brief Get the pointer from id and offset ("inverse" to getOffset)
    /// @param[in] id Is the id of the segment and is used to get the base pointer
    /// @param[in] offset Is the offset for which the pointer should be calculated
    /// @return The pointer from id and offset
    static T* getPtr(const segment_id_t id, const offset_t offset) noexcept;

    /// @brief Get the id for a given ptr
    /// @param[in] ptr The pointer whose corresponding id is searched for
    /// @return segment_id to which the pointer was registered to
    static segment_id_underlying_t searchId(ptr_t const ptr) noexcept;

    /// @brief Get the offset from the start address of the segment and ptr
    /// @param[in] ptr Is the pointer whose offset should be calculated
    /// @return The offset of the passed pointer
    offset_t computeOffset(ptr_t const ptr) const noexcept;

    /// @brief Get the pointer from stored id and offset
    /// @return The pointer for stored id and offset
    T* computeRawPtr() const noexcept;

    static constexpr segment_id_underlying_t NULL_POINTER_ID{std::numeric_limits<segment_id_underlying_t>::max()};
    static constexpr offset_t NULL_POINTER_OFFSET{std::numeric_limits<offset_t>::max()};

  private:
    segment_id_underlying_t m_id{NULL_POINTER_ID};
    offset_t m_offset{NULL_POINTER_OFFSET};
};

using UntypedRelativePointer = RelativePointer<void>;

/// @brief returns the pointer repository storing untyped pointers
/// @return the static pointer repository
PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t>& getRepository() noexcept;

/// @brief Checks if lhs and rhs point to the same memory
/// @param[in] lhs Relative Pointer whose pointer is compared
/// @param[in] rhs Is the raw pointer which is compared
/// @return True if both point to the same memory, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
template <typename T>
bool operator==(const RelativePointer<T> lhs, const T* const rhs) noexcept;

/// @brief Checks if lhs and rhs point to the same memory
/// @param[in] lhs Is the raw pointer which is compared
/// @param[in] rhs Relative Pointer whose pointer is compared
/// @return True if both point to the same memory, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
template <typename T>
bool operator==(const T* const lhs, const RelativePointer<T> rhs) noexcept;

/// @brief Checks if rhs is a nullptr
/// @param[in] lhs nullptr
/// @param[in] rhs Relative Pointer whose pointer is compared
/// @return True if the rhs points to nothing, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
template <typename T>
bool operator==(std::nullptr_t, const RelativePointer<T> rhs) noexcept;

/// @brief Checks if lhs is a nullptr
/// @param[in] lhs Relative Pointer whose pointer is compared
/// @param[in] rhs nullptr
/// @return True if the lhs points to nothing, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
template <typename T>
bool operator==(const RelativePointer<T> lhs, std::nullptr_t rhs) noexcept;

/// @brief Checks if lhs and rhs don't point to the same memory
/// @param[in] lhs Relative Pointer whose pointer is compared
/// @param[in] rhs Is the raw pointer which is compared
/// @return True if both don't point to the same memory, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
template <typename T>
bool operator!=(const RelativePointer<T> lhs, const T* const rhs) noexcept;

/// @brief Checks if lhs and rhs don't point to the same memory
/// @param[in] lhs Is the raw pointer which is compared
/// @param[in] rhs Relative Pointer whose pointer is compared
/// @return True if both don't point to the same memory, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to raw pointers
template <typename T>
bool operator!=(const T* const lhs, const RelativePointer<T> rhs) noexcept;

/// @brief Checks if rhs is not a nullptr
/// @param[in] lhs nullptr
/// @param[in] rhs Relative Pointer whose pointer is compared
/// @return True if the rhs does not point to nothing, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
template <typename T>
bool operator!=(std::nullptr_t, const RelativePointer<T> rhs) noexcept;

/// @brief Checks if lhs is not a nullptr
/// @param[in] lhs Relative Pointer whose pointer is compared
/// @param[in] rhs nullptr
/// @return True if the lhs does not point to nothing, otherwise false
// AXIVION Next Construct AutosarC++19_03-A13.5.5 : The RelativePointer shall explicitly be comparable to nullptrs
template <typename T>
bool operator!=(const RelativePointer<T> lhs, std::nullptr_t rhs) noexcept;

} // namespace iox

#include "iox/detail/relative_pointer.inl"

#endif // IOX_HOOFS_MEMORY_RELATIVE_POINTER_HPP
