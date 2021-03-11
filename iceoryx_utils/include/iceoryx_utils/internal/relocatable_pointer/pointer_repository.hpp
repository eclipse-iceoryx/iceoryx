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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_POINTER_REPOSITORY_HPP
#define IOX_UTILS_RELOCATABLE_POINTER_POINTER_REPOSITORY_HPP

#include "iceoryx_utils/cxx/vector.hpp"
#include <iostream>

#include <assert.h>

namespace iox
{
/// @brief Allows registration of memory segments with their start pointers and size.
/// This class is used to resolve relative pointers in the corresponding address space of the application.
/// Up to CAPACITY segments can be registered with MIN_ID = 1 to MAX_ID = CAPACITY - 1
/// id 0 is reserved and allows relative pointers to behave like normal pointers
/// (which is equivalent to measure the offset relative to 0).
template <typename id_t, typename ptr_t, uint64_t CAPACITY = 10000U>
class PointerRepository
{
  private:
    struct Info
    {
        ptr_t basePtr{nullptr};
        ptr_t endPtr{nullptr};
    };

    /// @note 0 is a special purpose id and reserved
    /// id 0 is reserved to interpret the offset just as a raw pointer,
    /// i.e. its corresponding base ptr is 0
    static constexpr size_t MIN_ID = 1U;
    static constexpr size_t MAX_ID = CAPACITY - 1U;

  public:
    static constexpr id_t INVALID_ID = std::numeric_limits<id_t>::max();

    PointerRepository() noexcept
        : m_info(CAPACITY)
    {
    }

    bool registerPtr(const id_t id, const ptr_t ptr, const uint64_t size) noexcept
    {
        if (id > MAX_ID)
        {
            return false;
        }
        if (m_info[id].basePtr == nullptr)
        {
            m_info[id].basePtr = ptr;
            m_info[id].endPtr = reinterpret_cast<ptr_t>(reinterpret_cast<uintptr_t>(ptr) + size - 1U);
            if (id > m_maxRegistered)
            {
                m_maxRegistered = id;
            }
            return true;
        }
        return false;
    }

    id_t registerPtr(const ptr_t ptr, const uint64_t size = 0U) noexcept
    {
        for (id_t id = 1U; id <= MAX_ID; ++id)
        {
            if (m_info[id].basePtr == nullptr)
            {
                m_info[id].basePtr = ptr;
                m_info[id].endPtr = reinterpret_cast<ptr_t>(reinterpret_cast<uintptr_t>(ptr) + size - 1U);
                if (id > m_maxRegistered)
                {
                    m_maxRegistered = id;
                }
                return id;
            }
        }

        return INVALID_ID;
    }

    bool unregisterPtr(const id_t id) noexcept
    {
        if (id <= MAX_ID && id >= MIN_ID)
        {
            if (m_info[id].basePtr != nullptr)
            {
                m_info[id].basePtr = nullptr;

                /// @note do not search for next lower registered index but we could do it here
                return true;
            }
        }

        return false;
    }

    void unregisterAll() noexcept
    {
        for (auto& info : m_info)
        {
            info.basePtr = nullptr;
        }
        m_maxRegistered = 0U;
    }

    ptr_t getBasePtr(const id_t id) const noexcept
    {
        if (id <= MAX_ID && id >= MIN_ID)
        {
            return m_info[id].basePtr;
        }

        /// @note for id 0 nullptr is returned, meaning we will later interpret a relative pointer
        /// by casting the offset into a pointer (i.e. we measure relative to 0)

        /// @note we cannot distinguish between not registered and nullptr registered, but we do not need to
        return nullptr;
    }

    id_t searchId(const ptr_t ptr) const noexcept
    {
        for (id_t id = 1U; id <= m_maxRegistered; ++id)
        {
            // return first id where the ptr is in the corresponding interval
            if (ptr >= m_info[id].basePtr && ptr <= m_info[id].endPtr)
            {
                return id;
            }
        }
        /// @note implicitly interpret the pointer as a regular pointer if not found
        /// by setting id to 0
        /// rationale: test cases work without registered shared memory and require
        /// this at the moment to avoid fundamental changes
        return 0U;
        // return INVALID_ID;
    }

    bool isValid(const id_t id) const noexcept
    {
        return id != INVALID_ID;
    }

    void print() const noexcept
    {
        for (id_t id = 0U; id < m_info.size(); ++id)
        {
            auto ptr = m_info[id].basePtr;
            if (ptr != nullptr)
            {
                std::cout << id << " ---> " << ptr << std::endl;
            }
        }
    }

  private:
    /// @todo: if required protect vector against concurrent modification
    // whether this is required depends on the use case, we currently do not need it
    // we control the ids, so if they are consecutive we only need a vector/array to get the address
    // this variable exists once per application using relative pointers,
    // and each needs to initialize it via register calls above

    iox::cxx::vector<Info, CAPACITY> m_info;
    uint64_t m_maxRegistered{0U};
};

} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_POINTER_REPOSITORY_HPP
