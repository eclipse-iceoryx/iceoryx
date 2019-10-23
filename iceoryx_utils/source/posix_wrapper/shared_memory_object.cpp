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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>


#include "iceoryx_utils/cxx/smart_c.hpp"

namespace iox
{
namespace posix
{
cxx::optional<SharedMemoryObject> SharedMemoryObject::create(const char* f_name,
                                                             const uint64_t f_memorySizeInBytes,
                                                             const AccessMode f_accessMode,
                                                             const OwnerShip f_ownerShip,
                                                             const void* f_baseAddressHint,
                                                             const mode_t f_permissions)
{
    auto l_alignment = Allocator::MEMORY_ALIGNMENT;
    uint64_t l_alignedMemorySize = (f_memorySizeInBytes % l_alignment == 0)
                                       ? f_memorySizeInBytes
                                       : (f_memorySizeInBytes / l_alignment + 1) * l_alignment;
    cxx::optional<SharedMemoryObject> returnValue;
    returnValue.emplace(f_name, l_alignedMemorySize, f_accessMode, f_ownerShip, f_baseAddressHint, f_permissions);

    if (returnValue->isInitialized())
    {
        return returnValue;
    }
    else
    {
        return cxx::nullopt_t();
    }
}

SharedMemoryObject::SharedMemoryObject(const char* f_name,
                                       const uint64_t f_memorySizeInBytes,
                                       const AccessMode f_accessMode,
                                       const OwnerShip f_ownerShip,
                                       const void* f_baseAddressHint,
                                       const mode_t f_permissions)
    : m_sharedMemory(f_name, f_accessMode, f_ownerShip, f_permissions, f_memorySizeInBytes)
    , m_memoryMap(f_baseAddressHint, f_memorySizeInBytes, m_sharedMemory.getHandle(), f_accessMode, MAP_SHARED, 0)
    , m_allocator(m_memoryMap.getBaseAddress(), f_memorySizeInBytes)
    , m_memorySizeInBytes(f_memorySizeInBytes)
    , m_isInitialized(m_sharedMemory.isInitialized() && m_memoryMap.isInitialized())
{
    if (!m_isInitialized)
    {
        if (!m_sharedMemory.isInitialized())
        {
            std::cerr << "Unable to create SharedMemoryObject since we could not acquire a SharedMemory resource"
                      << std::endl;
        }
        else if (!m_memoryMap.isInitialized())
        {
            std::cerr << "Unable to create SharedMemoryObject since we could not map the memory into the application"
                      << std::endl;
        }
    }

    if (f_ownerShip == OwnerShip::mine && m_isInitialized)
    {
        std::clog << "Reserving " << f_memorySizeInBytes << " bytes in the shared memory [" << f_name << "]"
                  << std::endl;
        memset(m_memoryMap.getBaseAddress(), 0, f_memorySizeInBytes);
        std::clog << "[ Reserving shared memory successful ] " << std::endl;
    }
}

SharedMemoryObject::~SharedMemoryObject()
{
}


void* SharedMemoryObject::allocate(const uint64_t f_size, const uint64_t f_alignment)
{
    return m_allocator.allocate(f_size, f_alignment);
}

void SharedMemoryObject::finalizeAllocation()
{
    m_allocator.finalizeAllocation();
}

bool SharedMemoryObject::isInitialized() const
{
    return m_isInitialized;
}

Allocator* SharedMemoryObject::getAllocator()
{
    return &m_allocator;
}

void* SharedMemoryObject::getBaseAddress() const
{
    return m_memoryMap.getBaseAddress();
}

uint64_t SharedMemoryObject::getSizeInBytes() const
{
    return m_memorySizeInBytes;
}

int SharedMemoryObject::getFileHandle() const
{
    return m_sharedMemory.getHandle();
}

} // namespace posix
} // namespace iox
