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
#include <sys/mman.h>

#include "iceoryx_utils/cxx/optional.hpp"

namespace iox
{
namespace posix
{
class SharedMemoryObject;
enum class AccessMode
{
    readOnly,
    readWrite
};
enum class OwnerShip
{
    mine,
    openExisting
};

class SharedMemory
{
  public:
    static cxx::optional<SharedMemory> create(const char* f_name,
                                              const AccessMode f_accessMode,
                                              const OwnerShip f_ownerShip,
                                              const mode_t f_permissions,
                                              const uint64_t f_size);
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&);
    SharedMemory& operator=(SharedMemory&&);
    ~SharedMemory();

    int getHandle() const;

    friend class posix::SharedMemoryObject;
    friend class cxx::optional<SharedMemory>;

  private:
    SharedMemory(const char* f_name,
                 const AccessMode f_accessMode,
                 const OwnerShip f_ownerShip,
                 const mode_t f_permissions,
                 const uint64_t f_size);

    bool open();
    bool unlink();
    bool close();
    bool isInitialized() const;

    static constexpr uint64_t NAME_SIZE = 128;

    bool m_isInitialized;
    char m_name[NAME_SIZE];
    OwnerShip m_ownerShip;
    int m_oflags = 0;
    mode_t m_permissions;
    uint64_t m_size;
    int m_handle{-1};
};
} // namespace posix
} // namespace iox

