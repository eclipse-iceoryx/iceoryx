// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP

#include "iceoryx_hoofs/cxx/filesystem.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/design_pattern/creation.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_hoofs/platform/stat.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
using byte_t = uint8_t;

enum class SharedMemoryObjectError
{
    SHARED_MEMORY_CREATION_FAILED,
    MAPPING_SHARED_MEMORY_FAILED,
};

class SharedMemoryObject : public DesignPattern::Creation<SharedMemoryObject, SharedMemoryObjectError>
{
  public:
    static constexpr void* NO_ADDRESS_HINT = nullptr;
    SharedMemoryObject(const SharedMemoryObject&) = delete;
    SharedMemoryObject& operator=(const SharedMemoryObject&) = delete;
    SharedMemoryObject(SharedMemoryObject&&) noexcept = default;
    SharedMemoryObject& operator=(SharedMemoryObject&&) noexcept = default;
    ~SharedMemoryObject() noexcept = default;

    void* allocate(const uint64_t size, const uint64_t alignment) noexcept;
    void finalizeAllocation() noexcept;

    Allocator* getAllocator() noexcept;
    const void* getBaseAddress() const noexcept;
    void* getBaseAddress() noexcept;

    uint64_t getSizeInBytes() const noexcept;
    int getFileHandle() const noexcept;
    bool hasOwnership() const noexcept;

    friend class DesignPattern::Creation<SharedMemoryObject, SharedMemoryObjectError>;

  private:
    SharedMemoryObject(const SharedMemory::Name_t& name,
                       const uint64_t memorySizeInBytes,
                       const AccessMode accessMode,
                       const OpenMode openMode,
                       const cxx::optional<const void*>& baseAddressHint = cxx::nullopt,
                       const cxx::perms permissions = cxx::perms::owner_read | cxx::perms::owner_write
                                                      | cxx::perms::group_read | cxx::perms::group_write) noexcept;

    bool isInitialized() const noexcept;

  private:
    uint64_t m_memorySizeInBytes;
    cxx::optional<SharedMemory> m_sharedMemory;
    cxx::optional<MemoryMap> m_memoryMap;
    cxx::optional<Allocator> m_allocator;

    bool m_isInitialized;
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP
