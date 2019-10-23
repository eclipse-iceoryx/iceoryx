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

#include "iceoryx_utils/internal/concurrent/locked_loffli.hpp"

#include <cassert>

namespace iox
{
namespace concurrent
{
void LockedLoFFLi::init(cxx::not_null<uint32_t*> f_freeIndicesMemory, const uint32_t f_size)
{
    cxx::Expects(m_accessMutex.has_value());
    cxx::Expects(f_size > 0);
    cxx::Expects(f_size <= UINT32_MAX - 2U);


    m_freeIndices = f_freeIndicesMemory;
    m_size = f_size;
    m_invalidIndex = m_size + 1;
    if (m_freeIndices != nullptr)
    {
        for (uint32_t i = 0; i < m_size + 1; i++)
        {
            m_freeIndices[i] = i + 1;
        }
    }
}

bool LockedLoFFLi::pop(uint32_t& index)
{
    std::lock_guard<posix::mutex> lock(*m_accessMutex);

    // we are empty if next points to an element with index of Size
    if (m_head >= m_size)
    {
        return false;
    }

    index = m_head;
    m_head = m_freeIndices[m_head];
    m_freeIndices[index] = m_invalidIndex;

    return true;
}

bool LockedLoFFLi::push(const uint32_t index)
{
    std::lock_guard<posix::mutex> lock(*m_accessMutex);

    if (index >= m_size || m_freeIndices[index] != m_invalidIndex)
    {
        return false;
    }

    m_freeIndices[index] = m_head;
    m_head = index;

    return true;
}

} // namespace concurrent
} // namespace iox
