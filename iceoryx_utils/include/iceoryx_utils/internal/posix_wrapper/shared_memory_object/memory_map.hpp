// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_MEMORY_MAP_HPP
#define IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_MEMORY_MAP_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_utils/platform/mman.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class SharedMemoryObject;

enum class MemoryMapError
{
    INVALID_STATE,
    ACCESS_FAILED,
    UNABLE_TO_LOCK,
    INVALID_FILE_DESCRIPTOR,
    MAP_OVERLAP,
    INVALID_PARAMETERS,
    OPEN_FILES_SYSTEM_LIMIT_EXCEEDED,
    FILESYSTEM_DOES_NOT_SUPPORT_MEMORY_MAPPING,
    NOT_ENOUGH_MEMORY_AVAILABLE,
    OVERFLOWING_PARAMETERS,
    PERMISSION_FAILURE,
    NO_WRITE_PERMISSION,
    UNKNOWN_ERROR
};

class MemoryMap : public DesignPattern::Creation<MemoryMap, MemoryMapError>
{
  public:
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator=(const MemoryMap&) = delete;
    MemoryMap(MemoryMap&& rhs) noexcept;
    MemoryMap& operator=(MemoryMap&& rhs) noexcept;

    ~MemoryMap();
    void* getBaseAddress() const noexcept;

    friend class DesignPattern::Creation<MemoryMap, MemoryMapError>;

  private:
    MemoryMap(const void* baseAddressHint,
              const uint64_t length,
              const int32_t fileDescriptor,
              const AccessMode accessMode,
              const int32_t flags = MAP_SHARED,
              const off_t offset = 0) noexcept;
    bool isInitialized() const noexcept;
    bool destroy() noexcept;
    MemoryMapError errnoToEnum(const int32_t errnum) const noexcept;

    void* m_baseAddress{nullptr};
    uint64_t m_length{0U};
};
} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_MEMORY_MAP_HPP
