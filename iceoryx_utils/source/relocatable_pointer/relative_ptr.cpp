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

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
RelativePointer::RelativePointer(ptr_t ptr, id_t id) noexcept
    : m_id(id)
    , m_offset(computeOffset(ptr))
{
}

RelativePointer::RelativePointer(offset_t offset, id_t id) noexcept
    : m_id(id)
    , m_offset(offset)
{
}

RelativePointer::RelativePointer(ptr_t ptr) noexcept
    : m_id(searchId(ptr))
    , m_offset(computeOffset(ptr))
{
}

RelativePointer::RelativePointer(const RelativePointer& other) noexcept
    : m_id(other.m_id)
    , m_offset(other.m_offset)
{
}

RelativePointer::RelativePointer(RelativePointer&& other) noexcept
    : m_id(other.m_id)
    , m_offset(other.m_offset)
{
    /// @note invalidating other would be an option but not required
}

RelativePointer& RelativePointer::operator=(const RelativePointer& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
    }
    return *this;
}

RelativePointer& RelativePointer::operator=(void* ptr) noexcept
{
    m_id = searchId(ptr);
    m_offset = computeOffset(ptr);

    return *this;
}

RelativePointer& RelativePointer::operator=(RelativePointer&& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
    }
    /// @note invalidating other would be an option but not required
    return *this;
}

RelativePointer::ptr_t RelativePointer::get() const noexcept
{
    /// @note we need to compute it each time since the application
    /// from where it's called might have changed (i.e. the lookup result is different)
    return computeRawPtr();
}

RelativePointer::id_t RelativePointer::getId() const noexcept
{
    return m_id;
}

RelativePointer::offset_t RelativePointer::getOffset() const noexcept
{
    return m_offset;
}

RelativePointer::ptr_t RelativePointer::getBasePtr() const noexcept
{
    return getBasePtr(m_id);
}

//*********************************id operations********************************************

/// @brief registers a memory segment at ptr with size of a new id
/// @return id id it was registered to
RelativePointer::id_t RelativePointer::registerPtr(const ptr_t ptr, uint64_t size) noexcept
{
    return getRepository().registerPtr(ptr, size);
}

/// @brief registers a memory segment at ptr with size of given id
/// @return true if successful (id not occupied), false otherwise
bool RelativePointer::registerPtr(const id_t id, const ptr_t ptr, uint64_t size) noexcept
{
    return getRepository().registerPtr(id, ptr, size);
}

/// @brief unregister ptr with given id
/// @return true if successful (ptr was registered with this id before), false otherwise
bool RelativePointer::unregisterPtr(const id_t id) noexcept
{
    return getRepository().unregisterPtr(id);
}

/// @brief get the base ptr associated with the given id
/// @return ptr registered at the given id, nullptr if none was registered
RelativePointer::ptr_t RelativePointer::getBasePtr(const id_t id) noexcept
{
    return getRepository().getBasePtr(id);
}

/// @brief unregister all ptr id pairs (leads to initial state)
void RelativePointer::unregisterAll() noexcept
{
    getRepository().unregisterAll();
}

/// @brief get the offset from id and ptr
/// @return offset
RelativePointer::offset_t RelativePointer::getOffset(const id_t id, const_ptr_t ptr) noexcept
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
RelativePointer::ptr_t RelativePointer::getPtr(const id_t id, const offset_t offset) noexcept
{
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }
    auto basePtr = getBasePtr(id);
    return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
}

RelativePointer::id_t RelativePointer::searchId(ptr_t ptr) noexcept
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_ID;
    }
    return getRepository().searchId(ptr);
}

bool RelativePointer::isValid(id_t id) noexcept
{
    return getRepository().isValid(id);
}

PointerRepository<RelativePointer::id_t, RelativePointer::ptr_t>& RelativePointer::getRepository() noexcept
{
    static PointerRepository<id_t, ptr_t> repository;
    return repository;
}

//*****************************************************************************************

RelativePointer::offset_t RelativePointer::computeOffset(ptr_t ptr) const noexcept
{
    return getOffset(m_id, ptr);
}

RelativePointer::ptr_t RelativePointer::computeRawPtr() const noexcept
{
    return getPtr(m_id, m_offset);
}
} // namespace iox
