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

#include <bitset>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace iox
{
namespace posix
{
SharedMemoryObject::SharedMemoryObject(const char* name,
                                       const uint64_t memorySizeInBytes,
                                       const AccessMode accessMode,
                                       const OwnerShip ownerShip,
                                       const void* baseAddressHint,
                                       const mode_t permissions)
    : m_memorySizeInBytes(cxx::align(memorySizeInBytes, Allocator::MEMORY_ALIGNMENT))
    , m_sharedMemory(name, accessMode, ownerShip, permissions, m_memorySizeInBytes)
{
    m_isInitialized = true;

    if (!m_sharedMemory.isInitialized())
    {
        std::cerr << "Unable to create SharedMemoryObject since we could not acquire a SharedMemory resource"
                  << std::endl;
        m_isInitialized = false;
        m_errorValue = SharedMemoryObjectError::SHARED_MEMORY_CREATION_FAILED;
    }

    if (m_isInitialized)
    {
        MemoryMap::create(baseAddressHint, m_memorySizeInBytes, m_sharedMemory.getHandle(), accessMode, MAP_SHARED, 0)
            .and_then([this](auto& memoryMap) { m_memoryMap.emplace(std::move(memoryMap)); })
            .or_else([this](auto) {
                std::cerr << "Failed to map created shared memory into process!" << std::endl;
                m_isInitialized = false;
                m_errorValue = SharedMemoryObjectError::MAPPING_SHARED_MEMORY_FAILED;
            });
    }

    if (m_isInitialized == false)
    {
        std::cerr << "Unable to create shared memory with the following properties [ name = " << name
                  << ", sizeInBytes = " << memorySizeInBytes
                  << ", access mode = " << ACCESS_MODE_STRING[static_cast<uint64_t>(accessMode)]
                  << ", ownership = " << OWNERSHIP_STRING[static_cast<uint64_t>(ownerShip)]
                  << ", baseAddressHint = " << std::hex << baseAddressHint
                  << ", permissions = " << std::bitset<sizeof(mode_t)>(permissions) << " ]" << std::endl;
        return;
    }

    m_allocator.emplace(m_memoryMap->getBaseAddress(), m_memorySizeInBytes);

    if (ownerShip == OwnerShip::mine && m_isInitialized)
    {
        std::clog << "Reserving " << m_memorySizeInBytes << " bytes in the shared memory [" << name << "]" << std::endl;
        memset(m_memoryMap->getBaseAddress(), 0, m_memorySizeInBytes);
        std::clog << "[ Reserving shared memory successful ] " << std::endl;
    }
}

void* SharedMemoryObject::allocate(const uint64_t size, const uint64_t alignment)
{
    return m_allocator->allocate(size, alignment);
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

int32_t SharedMemoryObject::getFileHandle() const
{
    return m_sharedMemory.getHandle();
}

} // namespace posix
} // namespace iox
