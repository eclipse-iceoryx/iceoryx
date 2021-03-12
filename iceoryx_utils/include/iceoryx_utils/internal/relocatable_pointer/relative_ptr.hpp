// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
#define IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP

#include "pointer_repository.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
class RelativePointer
{
  public:
    using id_t = uint64_t;
    using ptr_t = void*;
    using const_ptr_t = const void* const;
    using offset_t = std::uintptr_t;

    RelativePointer(ptr_t ptr, id_t id) noexcept;

    RelativePointer(offset_t offset, id_t id) noexcept;

    RelativePointer(ptr_t ptr = nullptr) noexcept;

    RelativePointer(const RelativePointer& other) noexcept;

    RelativePointer(RelativePointer&& other) noexcept;

    RelativePointer& operator=(const RelativePointer& other) noexcept;

    RelativePointer& operator=(void* ptr) noexcept;

    RelativePointer& operator=(RelativePointer&& other) noexcept;

    ptr_t get() const noexcept;

    id_t getId() const noexcept;

    offset_t getOffset() const noexcept;

    ptr_t getBasePtr() const noexcept;

    //*********************************id operations********************************************

    /// @brief registers a memory segment at ptr with size of a new id
    /// @return id id it was registered to
    static id_t registerPtr(const ptr_t ptr, uint64_t size = 0U) noexcept;

    /// @brief registers a memory segment at ptr with size of given id
    /// @return true if successful (id not occupied), false otherwise
    static bool registerPtr(const id_t id, const ptr_t ptr, uint64_t size = 0U) noexcept;

    /// @brief unregister ptr with given id
    /// @return true if successful (ptr was registered with this id before), false otherwise
    static bool unregisterPtr(const id_t id) noexcept;

    /// @brief get the base ptr associated with the given id
    /// @return ptr registered at the given id, nullptr if none was registered
    static ptr_t getBasePtr(const id_t id) noexcept;

    /// @brief unregister all ptr id pairs (leads to initial state)
    static void unregisterAll() noexcept;

    /// @brief get the offset from id and ptr
    /// @return offset
    static offset_t getOffset(const id_t id, const_ptr_t ptr) noexcept;

    /// @brief get the pointer from id and offset ("inverse" to getOffset)
    /// @return ptr
    static ptr_t getPtr(const id_t id, const offset_t offset) noexcept;

    static id_t searchId(ptr_t ptr) noexcept;

    static bool isValid(id_t id) noexcept;

    static PointerRepository<id_t, ptr_t>& getRepository() noexcept;

    //*****************************************************************************************

    offset_t computeOffset(ptr_t ptr) const noexcept;

    ptr_t computeRawPtr() const noexcept;

    static constexpr id_t NULL_POINTER_ID = std::numeric_limits<id_t>::max();
    static constexpr offset_t NULL_POINTER_OFFSET = std::numeric_limits<offset_t>::max();

  protected:
    id_t m_id{NULL_POINTER_ID};
    offset_t m_offset{NULL_POINTER_OFFSET};
};

template <typename T>
class relative_ptr : public RelativePointer
{
  public:
    relative_ptr(ptr_t ptr, id_t id) noexcept;

    relative_ptr(offset_t offset, id_t id) noexcept;

    relative_ptr(ptr_t ptr = nullptr) noexcept;

    relative_ptr(const RelativePointer& other) noexcept;

    relative_ptr& operator=(const RelativePointer& other) noexcept;

    relative_ptr& operator=(ptr_t ptr) noexcept;

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, U&>::type operator*() noexcept;

    T* operator->() noexcept;

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const noexcept;

    T* operator->() const noexcept;

    T* get() const noexcept;

    operator T*() const noexcept;

    bool operator==(T* const ptr) const noexcept;

    bool operator!=(T* const ptr) const noexcept;
};

} // namespace iox

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.inl"

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
