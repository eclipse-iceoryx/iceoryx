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

    RelativePointer(const_ptr_t ptr, const id_t id) noexcept
        : m_id(id)
        , m_offset(computeOffset(ptr))
    {
    }

    RelativePointer(const offset_t offset, const id_t id) noexcept
        : m_id(id)
        , m_offset(offset)
    {
    }

    RelativePointer(const ptr_t ptr = nullptr) noexcept
        : m_id(searchId(ptr))
        , m_offset(computeOffset(ptr))
    {
    }

    RelativePointer(const RelativePointer& other) noexcept
        : m_id(other.m_id)
        , m_offset(other.m_offset)
    {
    }

    RelativePointer(RelativePointer&& other) noexcept
        : m_id(other.m_id)
        , m_offset(other.m_offset)
    {
        /// @note invalidating other would be an option but not required
    }

    RelativePointer& operator=(const RelativePointer& other) noexcept
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_offset = other.m_offset;
        }
        return *this;
    }

    RelativePointer& operator=(void* ptr) noexcept
    {
        m_id = searchId(ptr);
        m_offset = computeOffset(ptr);

        return *this;
    }

    RelativePointer& operator=(RelativePointer&& other) noexcept
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_offset = other.m_offset;
        }
        /// @note invalidating other would be an option but not required
        return *this;
    }

    ptr_t get() const noexcept
    {
        /// @note we need to compute it each time since the application
        /// from where it's called might have changed (i.e. the lookup result is different)
        return computeRawPtr();
    }

    id_t getId() const noexcept
    {
        return m_id;
    }

    offset_t getOffset() const noexcept
    {
        return m_offset;
    }

    ptr_t getBasePtr() const noexcept
    {
        return getBasePtr(m_id);
    }

    //*********************************id operations********************************************

    /// @brief registers a memory segment at ptr with size of a new id
    /// @return id id it was registered to
    static id_t registerPtr(const ptr_t ptr, const uint64_t size = 0U) noexcept
    {
        return getRepository().registerPtr(ptr, size);
    }

    /// @brief registers a memory segment at ptr with size of given id
    /// @return true if successful (id not occupied), false otherwise
    static bool registerPtr(const id_t id, const ptr_t ptr, uint64_t size = 0U) noexcept
    {
        return getRepository().registerPtr(id, ptr, size);
    }

    /// @brief unregister ptr with given id
    /// @return true if successful (ptr was registered with this id before), false otherwise
    static bool unregisterPtr(const id_t id) noexcept
    {
        return getRepository().unregisterPtr(id);
    }

    /// @brief get the base ptr associated with the given id
    /// @return ptr registered at the given id, nullptr if none was registered
    static ptr_t getBasePtr(const id_t id) noexcept
    {
        return getRepository().getBasePtr(id);
    }

    /// @brief unregister all ptr id pairs (leads to initial state)
    static void unregisterAll() noexcept
    {
        getRepository().unregisterAll();
    }

    /// @brief get the offset from id and ptr
    /// @return offset
    static offset_t getOffset(const id_t id, const_ptr_t ptr) noexcept
    {
        if (id == NULL_POINTER_ID)
        {
            return NULL_POINTER_OFFSET;
        }
        auto basePtr = getBasePtr(id);
        return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
    }


    /// @brief get the pointer from id and offset ("inverse" to getOffset)
    /// @return ptr
    static ptr_t getPtr(const id_t id, const offset_t offset) noexcept
    {
        if (offset == NULL_POINTER_OFFSET)
        {
            return nullptr;
        }
        auto basePtr = getBasePtr(id);
        return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
    }

    static id_t searchId(const ptr_t ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return NULL_POINTER_ID;
        }
        return getRepository().searchId(ptr);
    }

    static bool isValid(const id_t id) noexcept
    {
        return getRepository().isValid(id);
    }

    static PointerRepository<id_t, ptr_t>& getRepository() noexcept
    {
        static PointerRepository<id_t, ptr_t> repository;
        return repository;
    }

    //*****************************************************************************************

    offset_t computeOffset(const_ptr_t ptr) const noexcept
    {
        return getOffset(m_id, ptr);
    }

    ptr_t computeRawPtr() const noexcept
    {
        return getPtr(m_id, m_offset);
    }

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
    relative_ptr(const_ptr_t ptr, const id_t id) noexcept
        : RelativePointer(ptr, id)
    {
    }

    relative_ptr(const offset_t offset, const id_t id) noexcept
        : RelativePointer(offset, id)
    {
    }

    relative_ptr(const ptr_t ptr = nullptr) noexcept
        : RelativePointer(ptr)
    {
    }


    relative_ptr(const RelativePointer& other) noexcept
        : RelativePointer(other)
    {
    }

    relative_ptr& operator=(const RelativePointer& other) noexcept
    {
        RelativePointer::operator=(other);

        return *this;
    }

    relative_ptr& operator=(const ptr_t ptr) noexcept
    {
        m_id = searchId(ptr);
        m_offset = computeOffset(ptr);

        return *this;
    }

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, U&>::type operator*() noexcept
    {
        return *get();
    }

    T* operator->() noexcept
    {
        return get();
    }

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const noexcept
    {
        return *get();
    }

    T* operator->() const noexcept
    {
        return get();
    }

    T* get() const noexcept
    {
        return static_cast<T*>(computeRawPtr());
    }

    operator T*() const noexcept
    {
        return get();
    }

    bool operator==(T* const ptr) const noexcept
    {
        return ptr == get();
    }

    bool operator!=(T* const ptr) const noexcept
    {
        return ptr != get();
    }
};

} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
