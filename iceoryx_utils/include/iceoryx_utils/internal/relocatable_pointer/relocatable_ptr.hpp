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

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
///@todo restructure and split into multiple files along with an implementation cpp for non-template code
class RelocatablePointer
{
  public:
    using offset_t = std::ptrdiff_t;

    RelocatablePointer()
    {
    }

    explicit RelocatablePointer(const void* ptr)
        : m_offset(computeOffset(ptr))
    {
    }

    RelocatablePointer(const RelocatablePointer& other)
        : m_offset(computeOffset(other.computeRawPtr()))
    {
    }

    RelocatablePointer(RelocatablePointer&& other)
        : m_offset(computeOffset(other.computeRawPtr()))
    {
        // could set other to null but there is no advantage
        // in moving RelocatablePointers since they are lightweight and in principle other is allowed to
        // still be functional (you just cannot rely on it)
    }

    RelocatablePointer& operator=(const RelocatablePointer& other)
    {
        if (this != &other)
        {
            m_offset = computeOffset(other.computeRawPtr());
        }
        return *this;
    }

    RelocatablePointer& operator=(const void* rawPtr)
    {
        m_offset = computeOffset(rawPtr);
        return *this;
    }

    RelocatablePointer& operator=(RelocatablePointer&& other)
    {
        m_offset = computeOffset(other.computeRawPtr());
        return *this;
    }

    const void* operator*() const
    {
        return computeRawPtr();
    }

    operator bool() const
    {
        return m_offset != NULL_POINTER_OFFSET;
    }

    bool operator!() const
    {
        return m_offset == NULL_POINTER_OFFSET;
    }

    void* get()
    {
        return computeRawPtr();
    }

    offset_t getOffset()
    {
        return m_offset;
    }

    void print() const
    {
        std::cout << "&m_offset = " << reinterpret_cast<offset_t>(&m_offset) << std::endl;
        std::cout << "m_offset = " << m_offset << std::endl;
        std::cout << "raw = " << this->operator*() << std::endl;
    }

  protected:
    offset_t m_offset{NULL_POINTER_OFFSET};

    static constexpr offset_t NULL_POINTER_OFFSET = std::numeric_limits<offset_t>::max();

    inline offset_t computeOffset(const void* ptr) const
    {
        //@ todo: find most efficient way to do this and check the valid range (signed/unsigned issues)
        // this implies that the absolute difference cannot be larger than 2^63 which is probably true in any shared
        // memory we use
        // otherwise we would need to use unsigned for differences and use one extra bit from somewhere else to indicate
        // the sign

        // this suffices if both addresses are not too far apart, e.g. when they point to data in a sufficiently "small"
        // shared memory
        //(if the shared memory is small, the difference does never underflow)

        // todo: better first cast to unsigned, then cast to signed later (extends range where it)
        return reinterpret_cast<offset_t>(&m_offset) - reinterpret_cast<offset_t>(ptr);
    }

    inline void* computeRawPtr() const
    {
        if (m_offset == NULL_POINTER_OFFSET)
            return nullptr;

        //@ todo: find most efficient way to do this (see above)
        return reinterpret_cast<void*>(reinterpret_cast<offset_t>(&m_offset) - m_offset);
    }
};

// typed version so we can use operator->
template <typename T>
class relocatable_ptr : public RelocatablePointer
{
  public:
    relocatable_ptr()
        : RelocatablePointer()
    {
    }

    relocatable_ptr(const T* ptr)
        : RelocatablePointer(ptr)
    {
    }

    relocatable_ptr(const RelocatablePointer& other)
    {
        m_offset = computeOffset(other.computeRawPtr());
    }

    relocatable_ptr(T* rawPtr)
    {
        m_offset = computeOffset(rawPtr);
    }

    relocatable_ptr& operator=(const RelocatablePointer& other)
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

    T& operator[](uint64_t index)
    {
        auto ptr = static_cast<T*>(computeRawPtr());
        return *(ptr + index);
    }
};

} // namespace iox
