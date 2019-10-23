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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"

#include "iceoryx_utils/cxx/optional.hpp"

#include <cstdint>
#include <sys/mman.h>

namespace iox
{
namespace posix
{
class SharedMemoryObject;

class MemoryMap
{
  public:
    cxx::optional<MemoryMap> static create(const void* f_baseAddressHint,
                                           const uint64_t f_length,
                                           const int f_fileDescriptor,
                                           const AccessMode f_accessMode = AccessMode::readWrite,
                                           const int f_flags = MAP_SHARED,
                                           const off_t f_offset = 0);

    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator=(const MemoryMap&) = delete;
    MemoryMap(MemoryMap&& rhs);
    MemoryMap& operator=(MemoryMap&& rhs);

    ~MemoryMap();
    void* getBaseAddress() const;

    friend class posix::SharedMemoryObject;
    friend class cxx::optional<MemoryMap>;

  private:
    MemoryMap(const void* f_baseAddressHint,
              const uint64_t f_length,
              const int f_fileDescriptor,
              const AccessMode f_accessMode,
              const int f_flags = MAP_SHARED,
              const off_t f_offset = 0);
    bool isInitialized() const;

    bool m_isInitialized;
    void* m_baseAddress;
    uint64_t m_length;
};
} // namespace posix
} // namespace iox

