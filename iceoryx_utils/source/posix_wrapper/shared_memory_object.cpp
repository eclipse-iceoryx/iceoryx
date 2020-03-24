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
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

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
    cxx::optional<SharedMemoryObject> returnValue;
    returnValue.emplace(f_name, f_memorySizeInBytes, f_accessMode, f_ownerShip, f_baseAddressHint, f_permissions);

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
    : m_memorySizeInBytes(cxx::align(f_memorySizeInBytes, Allocator::MEMORY_ALIGNMENT))
    , m_sharedMemory(f_name, f_accessMode, f_ownerShip, f_permissions, m_memorySizeInBytes)
{
    if (!m_sharedMemory.isInitialized())
    {
        std::cerr << "Unable to create SharedMemoryObject since we could not acquire a SharedMemory resource"
                  << std::endl;
        m_isInitialized = false;
        return;
    }

    m_memoryMap = MemoryMap::create(
        f_baseAddressHint, m_memorySizeInBytes, m_sharedMemory.getHandle(), f_accessMode, MAP_SHARED, 0);

    if (!m_memoryMap.has_value())
    {
        std::cerr << "Unable to create SharedMemoryObject since we could not map the memory into the application"
                  << std::endl;
        m_isInitialized = false;
        return;
    }
    m_allocator.emplace(m_memoryMap->getBaseAddress(), m_memorySizeInBytes);
    m_isInitialized = true;

    if (f_ownerShip == OwnerShip::mine && m_isInitialized)
    {
        std::clog << "Reserving " << m_memorySizeInBytes << " bytes in the shared memory [" << f_name << "]"
                  << std::endl;
        memset(m_memoryMap->getBaseAddress(), 0, m_memorySizeInBytes);
        std::clog << "[ Reserving shared memory successful ] " << std::endl;
    }
}

void* SharedMemoryObject::allocate(const uint64_t f_size, const uint64_t f_alignment)
{
    return m_allocator->allocate(f_size, f_alignment);
}

void SharedMemoryObject::finalizeAllocation()
{
    m_allocator->finalizeAllocation();
}

bool SharedMemoryObject::isInitialized() const
{
    return m_isInitialized;
}

Allocator* SharedMemoryObject::getAllocator()
{
    return &*m_allocator;
}

void* SharedMemoryObject::getBaseAddress() const
{
    return m_memoryMap->getBaseAddress();
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
