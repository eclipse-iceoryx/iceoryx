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

#pragma once

#include "pointer_repository.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
class RelativePointer
{
  public:
    using id_t = size_t;
    using ptr_t = void*;
    using offset_t = std::ptrdiff_t;

    RelativePointer(ptr_t ptr = nullptr, id_t id = 0)
        : m_id(id)
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

    RelativePointer& operator=(void* rawPtr)
    {
        m_id = NULL_POINTER_ID;
        m_offset = computeOffset(rawPtr);

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

    operator bool() const
    {
        return this->get() != nullptr;
    }

    bool operator!() const
    {
        return this->get() == nullptr;
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

    //***************************************debug***********************************************

    void setId(id_t id)
    {
        m_id = id;
    }

    void print() const
    {
        std::cout << "m_id = " << m_id << std::endl;
        std::cout << "m_offset = " << m_offset << std::endl;
        std::cout << "base = " << getBasePtr() << std::endl;
        std::cout << "raw = " << get() << std::endl;
    }

    static void printRepository()
    {
        s_repository.print();
    }

    //*********************************id operations********************************************

    id_t registerPtr(const ptr_t ptr)
    {
        return s_repository.registerPtr(ptr);
    }

    static bool registerPtr(const id_t id, const ptr_t ptr)
    {
        return s_repository.registerPtr(id, ptr);
    }

    static bool unregisterPtr(const id_t id)
    {
        return s_repository.unregisterPtr(id);
    }

    static ptr_t getBasePtr(const id_t id)
    {
        return s_repository.getBasePtr(id);
    }

    static void unregisterAll()
    {
        s_repository.unregisterAll();
    }
    //*****************************************************************************************

  protected:
    static constexpr id_t NULL_POINTER_ID = std::numeric_limits<id_t>::max();
    static constexpr offset_t NULL_POINTER_OFFSET = std::numeric_limits<offset_t>::max();

    id_t m_id{NULL_POINTER_ID};
    offset_t m_offset{NULL_POINTER_OFFSET};

    inline offset_t computeOffset(ptr_t ptr)
    {
        if (m_id == NULL_POINTER_ID)
        {
            return NULL_POINTER_OFFSET;
        }
        auto basePtr = getBasePtr(m_id);
        return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
    }

    inline ptr_t computeRawPtr() const
    {
        // note: ideally this function needs to be fast, i.e. not much overhead
        if (m_offset == NULL_POINTER_OFFSET)
        {
            return nullptr;
        }
        auto basePtr = getBasePtr(m_id);
        ///@todo: check types, ranges and casts
        return reinterpret_cast<ptr_t>(m_offset + reinterpret_cast<offset_t>(basePtr));
    }

    static PointerRepository<id_t, ptr_t> s_repository;
};

PointerRepository<RelativePointer::id_t, RelativePointer::ptr_t> RelativePointer::s_repository;

template <typename T>
class relative_ptr : public RelativePointer
{
  public:
    ///@todo: size check: whether we are in the segment, if not logical nullptr pendant or assertion
    relative_ptr(ptr_t ptr = nullptr, id_t id = 0)
        : RelativePointer(ptr, id)
    {
    }

    relative_ptr(const RelativePointer& other)
    {
        m_offset = computeOffset(other.computeRawPtr());
        print();
    }

    relative_ptr(void* rawPtr)
    {
        m_offset = computeOffset(rawPtr);
        print();
    }

    relative_ptr& operator=(const RelativePointer& other)
    {
        m_offset = computeOffset(other.computeRawPtr());

        return *this;
    }

    T& operator*()
    {
        return *(static_cast<T*>(computeRawPtr()));
    }

    T* operator->()
    {
        return static_cast<T*>(computeRawPtr());
    }

    const T& operator*() const
    {
        return *(static_cast<T*>(computeRawPtr()));
    }

    const T* operator->() const
    {
        return static_cast<T*>(computeRawPtr());
    }

    //*********************************debug****************

    void print() const
    {
        RelativePointer::print();
        std::cout << "value " << operator*() << std::endl;
    }
};
} // namespace iox
