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

#include "iceoryx_utils/cxx/vector.hpp"
#include <iostream>

namespace iox
{
//@todo: extend the PointerRepository to use also segment size information
// to forbid creation of pointers outside of a valid segment
template <typename id_t, typename ptr_t, size_t SIZE = 10000>
class PointerRepository
{
  public:
    static constexpr id_t INVALID_ID = std::numeric_limits<id_t>::max();

    PointerRepository()
    {
        // we need more convenient ctors for vector ... like vector(size, initialValue)
        for (size_t i = 0; i < SIZE; ++i)
        {
            m_ptrs.emplace_back(nullptr);
        }
    }

    bool registerPtr(id_t id, ptr_t ptr)
    {
        auto s = m_ptrs.size();
        if (s <= id)
        {
            return false;
        }

        if (m_ptrs[id] == nullptr)
        {
            m_ptrs[id] = ptr;
            return true;
        }
        return false;
    }

    id_t registerPtr(const ptr_t ptr)
    {
        for (id_t id = 1; id < SIZE; ++id)
        {
            if (m_ptrs[id] == nullptr)
            {
                m_ptrs[id] = ptr;
                return id;
            }
        }

        return INVALID_ID;
    }

    bool unregisterPtr(id_t id)
    {
        if (m_ptrs.size() > id)
        {
            if (m_ptrs[id] != nullptr)
            {
                m_ptrs[id] = nullptr;
                return true;
            }
        }

        return false;
    }

    void unregisterAll()
    {
        for (auto& ptr : m_ptrs)
        {
            ptr = nullptr;
        }
    }

    ptr_t getBasePtr(id_t id)
    {
        if (m_ptrs.size() > id)
        {
            return m_ptrs[id];
        }

        return nullptr; // we cannot distinguish between not registered and nullptr registered, but we do not need to
    }

    void print()
    {
        for (id_t id = 0; id < m_ptrs.size(); ++id)
        {
            auto ptr = m_ptrs[id];
            if (ptr != nullptr)
            {
                std::cout << id << " ---> " << ptr << std::endl;
            }
        }
    }

  private:
    ///@ todo: if required protect vector against concurrent modification
    // whether this is required depends on the use case, we currently do not need it
    // we control the ids, so if they are consecutive we only need a vector/array to get the address
    // this variable exists once per application using Relative Pointers,
    // and each needs to initialize it via register calls above

    iox::cxx::vector<ptr_t, SIZE> m_ptrs;
};

} // namespace iox
