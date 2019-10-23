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

#include <cstdint>
#include <sys/stat.h>

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"

namespace iox
{
namespace posix
{
using byte_t = uint8_t;

class SharedMemoryObject
{
  public:
    static cxx::optional<SharedMemoryObject> create(const char* f_name,
                                                    const uint64_t f_memorySizeInBytes,
                                                    const AccessMode f_accessMode,
                                                    const OwnerShip f_ownerShip,
                                                    const void* f_baseAddressHint,
                                                    const mode_t f_permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
                                                                                 | S_IROTH | S_IWOTH);
    SharedMemoryObject(const SharedMemoryObject&) = delete;
    SharedMemoryObject& operator=(const SharedMemoryObject&) = delete;
    SharedMemoryObject(SharedMemoryObject&&) = default;
    SharedMemoryObject& operator=(SharedMemoryObject&&) = default;
    ~SharedMemoryObject();

    void* allocate(const uint64_t f_size, const uint64_t f_alignment = Allocator::MEMORY_ALIGNMENT);
    void finalizeAllocation();

    Allocator* getAllocator();
    void* getBaseAddress() const;

    uint64_t getSizeInBytes() const;
    int getFileHandle() const;

    friend class cxx::optional<SharedMemoryObject>;

  private:
    SharedMemoryObject(const char* f_name,
                       const uint64_t f_memorySizeInBytes,
                       const AccessMode f_accessMode,
                       const OwnerShip f_ownerShip,
                       const void* f_baseAddressHint,
                       const mode_t f_permissions);

    bool isInitialized() const;

  private:
    SharedMemory m_sharedMemory;
    MemoryMap m_memoryMap;
    Allocator m_allocator;
    uint64_t m_memorySizeInBytes;

    bool m_isInitialized;
};
} // namespace posix
} // namespace iox

