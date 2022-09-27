// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/relocatable_pointer/base_relative_pointer.hpp"

namespace iox
{
namespace rp
{
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
BaseRelativePointer::BaseRelativePointer(const ptr_t ptr, const id_t id) noexcept
    : m_id(id)
    , m_offset(computeOffset(ptr))
{
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
BaseRelativePointer::BaseRelativePointer(const offset_t offset, const id_t id) noexcept
    : m_id(id)
    , m_offset(offset)
{
}

BaseRelativePointer::BaseRelativePointer(ptr_t ptr) noexcept
    : m_id(searchId(ptr))
    , m_offset(computeOffset(ptr))
{
}

BaseRelativePointer::BaseRelativePointer(BaseRelativePointer&& other) noexcept
    : m_id(other.m_id)
    , m_offset(other.m_offset)
{
    /// @note invalidating other would be an option but not required
}

BaseRelativePointer& BaseRelativePointer::operator=(const BaseRelativePointer& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
    }
    return *this;
}

BaseRelativePointer& BaseRelativePointer::operator=(void* ptr) noexcept
{
    m_id = searchId(ptr);
    m_offset = computeOffset(ptr);

    return *this;
}

BaseRelativePointer& BaseRelativePointer::operator=(BaseRelativePointer&& other) noexcept
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_offset = other.m_offset;
    }
    /// @note invalidating other would be an option but not required
    return *this;
}

BaseRelativePointer::ptr_t BaseRelativePointer::get() const noexcept
{
    /// @note we need to compute it each time since the application
    /// from where it's called might have changed (i.e. the lookup result is different)
    return computeRawPtr();
}

BaseRelativePointer::id_underlying_t BaseRelativePointer::getId() const noexcept
{
    return m_id;
}

BaseRelativePointer::offset_t BaseRelativePointer::getOffset() const noexcept
{
    return m_offset;
}

BaseRelativePointer::ptr_t BaseRelativePointer::getBasePtr() const noexcept
{
    return getBasePtr(id_t{m_id});
}

cxx::optional<BaseRelativePointer::id_underlying_t> BaseRelativePointer::registerPtr(const ptr_t ptr,
                                                                                     uint64_t size) noexcept
{
    return getRepository().registerPtr(ptr, size);
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool BaseRelativePointer::registerPtr(const id_t id, const ptr_t ptr, uint64_t size) noexcept
{
    return getRepository().registerPtrWithId(static_cast<id_underlying_t>(id), ptr, size);
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool BaseRelativePointer::unregisterPtr(const id_t id) noexcept
{
    return getRepository().unregisterPtr(static_cast<id_underlying_t>(id));
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
BaseRelativePointer::ptr_t BaseRelativePointer::getBasePtr(const id_t id) noexcept
{
    return getRepository().getBasePtr(static_cast<id_underlying_t>(id));
}

void BaseRelativePointer::unregisterAll() noexcept
{
    getRepository().unregisterAll();
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
BaseRelativePointer::offset_t BaseRelativePointer::getOffset(const id_t id, const_ptr_t ptr) noexcept
{
    if (static_cast<id_underlying_t>(id) == NULL_POINTER_ID)
    {
        return NULL_POINTER_OFFSET;
    }
    auto* basePtr = getBasePtr(id);
    // NOLINTJUSTIFICATION Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
}

// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
BaseRelativePointer::ptr_t BaseRelativePointer::getPtr(const id_t id, const offset_t offset) noexcept
{
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }
    auto* basePtr = getBasePtr(id);
    // NOLINTJUSTIFICATION Cast needed for pointer arithmetic
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
}

BaseRelativePointer::id_underlying_t BaseRelativePointer::searchId(ptr_t ptr) noexcept
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_ID;
    }
    return getRepository().searchId(ptr);
}

PointerRepository<BaseRelativePointer::id_underlying_t, BaseRelativePointer::ptr_t>&
BaseRelativePointer::getRepository() noexcept
{
    static PointerRepository<id_underlying_t, ptr_t> repository;
    return repository;
}

BaseRelativePointer::offset_t BaseRelativePointer::computeOffset(ptr_t ptr) const noexcept
{
    return getOffset(id_t{m_id}, ptr);
}

BaseRelativePointer::ptr_t BaseRelativePointer::computeRawPtr() const noexcept
{
    return getPtr(id_t{m_id}, m_offset);
}
} // namespace rp
} // namespace iox
