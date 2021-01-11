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

    RelativePointer(ptr_t ptr, id_t id)
        : m_id(id)
        , m_offset(computeOffset(ptr))
    {
    }

    RelativePointer(offset_t offset, id_t id)
        : m_id(id)
        , m_offset(offset)
    {
    }

    RelativePointer(ptr_t ptr = nullptr)
        : m_id(searchId(ptr))
        , m_offset(computeOffset(ptr))
    {
    }

    RelativePointer(const RelativePointer& other)
        : m_id(other.m_id)
        , m_offset(other.m_offset)
    {
    }

    RelativePointer(RelativePointer&& other)
        : m_id(other.m_id)
        , m_offset(other.m_offset)
    {
        // invalidating other would be an option but not required
    }

    RelativePointer& operator=(const RelativePointer& other)
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_offset = other.m_offset;
        }
        return *this;
    }

    RelativePointer& operator=(void* ptr)
    {
        m_id = searchId(ptr);
        m_offset = computeOffset(ptr);

        return *this;
    }

    RelativePointer& operator=(RelativePointer&& other)
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_offset = other.m_offset;
        }
        // invalidating other would be an option but not required
        return *this;
    }

    ptr_t get() const
    {
        // note that we need to compute it each time since the application
        // from where its called might have changed (i.e. the lookup result is different)
        return computeRawPtr();
    }

    id_t getId() const
    {
        return m_id;
    }

    offset_t getOffset() const
    {
        return m_offset;
    }

    ptr_t getBasePtr() const
    {
        return getBasePtr(m_id);
    }

    //*********************************id operations********************************************

    ///@brief registers a memory segment at ptr with size of a new id
    ///@return id id it was registered to
    static id_t registerPtr(const ptr_t ptr, uint64_t size = 0)
    {
        return getRepository().registerPtr(ptr, size);
    }

    ///@brief registers a memory segment at ptr with size of given id
    ///@return true if successful (id not occupied), false otherwise
    static bool registerPtr(const id_t id, const ptr_t ptr, uint64_t size = 0)
    {
        return getRepository().registerPtr(id, ptr, size);
    }

    ///@brief unregister ptr with given id
    ///@return true if successful (ptr was registered with this id before), false otherwise
    static bool unregisterPtr(const id_t id)
    {
        return getRepository().unregisterPtr(id);
    }

    ///@brief get the base ptr associated with the given id
    ///@return ptr registered at the given id, nullptr if none was registered
    static ptr_t getBasePtr(const id_t id)
    {
        return getRepository().getBasePtr(id);
    }

    ///@brief unregister all ptr id pairs (leads to initial state)
    static void unregisterAll()
    {
        getRepository().unregisterAll();
    }

    ///@brief get the offset from id and ptr
    ///@return offset
    static offset_t getOffset(const id_t id, const_ptr_t ptr)
    {
        if (id == NULL_POINTER_ID)
        {
            return NULL_POINTER_OFFSET;
        }
        auto basePtr = getBasePtr(id);
        return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
    }


    ///@brief get the pointer from id and offset ("inverse" to getOffset)
    ///@return ptr
    static ptr_t getPtr(const id_t id, const offset_t offset)
    {
        if (offset == NULL_POINTER_OFFSET)
        {
            return nullptr;
        }
        auto basePtr = getBasePtr(id);
        return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
    }

    static id_t searchId(ptr_t ptr)
    {
        if (ptr == nullptr)
        {
            return NULL_POINTER_ID;
        }
        return getRepository().searchId(ptr);
    }

    static bool isValid(id_t id)
    {
        return getRepository().isValid(id);
    }

    static PointerRepository<id_t, ptr_t>& getRepository() noexcept
    {
        static PointerRepository<id_t, ptr_t> repository;
        return repository;
    }

    //*****************************************************************************************

    offset_t computeOffset(ptr_t ptr)
    {
        return getOffset(m_id, ptr);
    }

    ptr_t computeRawPtr() const
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
    relative_ptr(ptr_t ptr, id_t id)
        : RelativePointer(ptr, id)
    {
    }

    relative_ptr(offset_t offset, id_t id)
        : RelativePointer(offset, id)
    {
    }

    relative_ptr(ptr_t ptr = nullptr)
        : RelativePointer(ptr)
    {
    }


    relative_ptr(const RelativePointer& other)
        : RelativePointer(other)
    {
    }

    relative_ptr& operator=(const RelativePointer& other)
    {
        RelativePointer::operator=(other);

        return *this;
    }

    relative_ptr& operator=(ptr_t ptr)
    {
        m_id = searchId(ptr);
        m_offset = computeOffset(ptr);

        return *this;
    }

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, U&>::type operator*()
    {
        return *get();
    }

    T* operator->()
    {
        return get();
    }

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }

    T* get() const
    {
        return static_cast<T*>(computeRawPtr());
    }

    operator T*() const
    {
        return get();
    }

    bool operator==(T* const ptr) const
    {
        return ptr == get();
    }

    bool operator!=(T* const ptr) const
    {
        return ptr != get();
    }
};

} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
