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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include <iostream>

namespace iox
{
namespace posix
{
constexpr uint64_t Allocator::MEMORY_ALIGNMENT;
Allocator::Allocator(const void* f_startAddress, const uint64_t f_length)
    : m_startAddress(const_cast<byte_t*>(static_cast<const byte_t*>(f_startAddress)))
    , m_length(f_length)
{
    /// @todo memset to set memory and to avoid the usage of unavailable memory
}

void* Allocator::allocate(const uint64_t f_size, const uint64_t f_alignment)
{
    cxx::Expects(f_size > 0);

    if (m_allocationFinalized)
    {
        std::cerr << "allocate() call after finalizeAllocation()! You are not allowed to acquire shared memory chunks "
                     "anymore!"
                  << std::endl;
        std::terminate();
    }

    uintptr_t l_currentAddress = reinterpret_cast<uintptr_t>(m_startAddress) + m_currentPosition;
    uintptr_t l_alignedPosition = cxx::align(l_currentAddress, f_alignment);
    l_alignedPosition -= reinterpret_cast<uintptr_t>(m_startAddress);

    byte_t* l_returnValue = nullptr;

    if (m_length >= l_alignedPosition + f_size)
    {
        l_returnValue = m_startAddress + l_alignedPosition;
        m_currentPosition = l_alignedPosition + f_size;
    }
    else
    {
        std::cerr << "Trying to allocate additional " << f_size << " bytes in the shared memory of capacity "
                  << m_length << " when there are already " << l_alignedPosition << " aligned bytes in use."
                  << std::endl;
        std::cerr << "Only " << m_length - l_alignedPosition << " bytes left." << std::endl;
        std::terminate();
    }

    return static_cast<void*>(l_returnValue);
}

void Allocator::finalizeAllocation()
{
    m_allocationFinalized = true;
}

} // namespace posix
} // namespace iox
