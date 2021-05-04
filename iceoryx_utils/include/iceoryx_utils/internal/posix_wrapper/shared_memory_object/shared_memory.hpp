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
#ifndef IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
#define IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/platform/mman.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class SharedMemoryObject;
enum class AccessMode : uint64_t
{
    READ_ONLY = 0U,
    READ_WRITE = 1U
};
static constexpr const char* ACCESS_MODE_STRING[] = {"AccessMode::READ_ONLY", "AccessMode::READ_WRITE"};

enum class OwnerShip : uint64_t
{
    MINE = 0U,
    OPEN_EXISTING = 1U
};
static constexpr const char* OWNERSHIP_STRING[] = {"OwnerShip::MINE", "OwnerShip::OPEN_EXISTING"};

enum class SharedMemoryError
{
    INVALID_STATE,
    EMPTY_NAME,
    NAME_WITHOUT_LEADING_SLASH,
    INSUFFICIENT_PERMISSIONS,
    DOES_EXIST,
    PROCESS_LIMIT_OF_OPEN_FILES_REACHED,
    SYSTEM_LIMIT_OF_OPEN_FILES_REACHED,
    DOES_NOT_EXIST,
    NOT_ENOUGH_MEMORY_AVAILABLE,
    REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE,
    PATH_IS_A_DIRECTORY,
    TOO_MANY_SYMBOLIC_LINKS,
    NO_FILE_RESIZE_SUPPORT,
    NO_RESIZE_SUPPORT,
    INVALID_FILEDESCRIPTOR,
    UNKNOWN_ERROR
};

class SharedMemory : public DesignPattern::Creation<SharedMemory, SharedMemoryError>
{
  public:
    static constexpr uint64_t NAME_SIZE = 128U;
    using Name_t = cxx::string<NAME_SIZE>;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;
    ~SharedMemory() noexcept;

    int32_t getHandle() const noexcept;

    friend class DesignPattern::Creation<SharedMemory, SharedMemoryError>;

  private:
    SharedMemory(const Name_t& name,
                 const AccessMode accessMode,
                 const OwnerShip ownerShip,
                 const mode_t permissions,
                 const uint64_t size) noexcept;

    bool open(const int oflags, const mode_t permissions, const uint64_t size) noexcept;
    bool unlink() noexcept;
    bool close() noexcept;
    void destroy() noexcept;
    void reset() noexcept;

    SharedMemoryError errnoToEnum(const int32_t errnum) const noexcept;


    Name_t m_name;
    OwnerShip m_ownerShip;
    int m_handle{-1};
};
} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
