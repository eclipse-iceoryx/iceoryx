// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/memory/port_pool_memory_block.hpp"

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"

namespace iox
{
namespace roudi
{
PortPoolMemoryBlock::~PortPoolMemoryBlock() noexcept
{
    destroy();
}

uint64_t PortPoolMemoryBlock::size() const noexcept
{
    return sizeof(PortPoolData);
}

uint64_t PortPoolMemoryBlock::alignment() const noexcept
{
    return alignof(PortPoolData);
}

void PortPoolMemoryBlock::onMemoryAvailable(cxx::not_null<void*> memory) noexcept
{
    m_portPoolData = new (memory) PortPoolData;
}

void PortPoolMemoryBlock::destroy() noexcept
{
    if (m_portPoolData)
    {
        m_portPoolData->~PortPoolData();
        m_portPoolData = nullptr;
    }
}

cxx::optional<PortPoolData*> PortPoolMemoryBlock::portPool() const noexcept
{
    return m_portPoolData ? cxx::make_optional<PortPoolData*>(m_portPoolData) : cxx::nullopt_t();
}

} // namespace roudi
} // namespace iox
