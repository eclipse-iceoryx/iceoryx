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

#include "iox/detail/posix_memory_map.hpp"
#include "iox/filesystem.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

#include <bitset>

namespace iox
{
namespace detail
{
expected<PosixMemoryMap, PosixMemoryMapError> PosixMemoryMapBuilder::create() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3, CertC++-EXP55 : Incompatibility with POSIX definition of mmap
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) low-level memory management
    auto result = IOX_POSIX_CALL(mmap)(const_cast<void*>(m_baseAddressHint),
                                       static_cast<size_t>(m_length),
                                       convertToProtFlags(m_accessMode),
                                       static_cast<int32_t>(m_flags),
                                       m_fileDescriptor,
                                       m_offset)

                      // NOLINTJUSTIFICATION cast required, type of error MAP_FAILED defined by POSIX to be void*
                      // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
                      .failureReturnValue(MAP_FAILED)
                      // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
                      .evaluate();

    if (result)
    {
        return ok(PosixMemoryMap(result.value().value, m_length));
    }

    constexpr uint64_t FLAGS_BIT_SIZE = 32U;
    IOX_LOG(ERROR,
            "Unable to map memory with the following properties [ baseAddressHint = "
                << iox::log::hex(m_baseAddressHint) << ", length = " << m_length
                << ", fileDescriptor = " << m_fileDescriptor << ", access mode = " << asStringLiteral(m_accessMode)
                << ", flags = " << std::bitset<FLAGS_BIT_SIZE>(static_cast<uint32_t>(m_flags)).to_string()
                << ", offset = " << iox::log::hex(m_offset) << " ]");
    return err(PosixMemoryMap::errnoToEnum(result.error().errnum));
}

PosixMemoryMap::PosixMemoryMap(void* const baseAddress, const uint64_t length) noexcept
    : m_baseAddress(baseAddress)
    , m_length(length)
{
}


// NOLINTJUSTIFICATION the function size results from the error handling and the expanded log macro
// NOLINTNEXTLINE(readability-function-size)
PosixMemoryMapError PosixMemoryMap::errnoToEnum(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
        IOX_LOG(ERROR,
                "One or more of the following failures happened:\n"
                    << "  1. The file descriptor belongs to a non-regular file.\n"
                    << "  2. The file descriptor is not opened for reading.\n"
                    << "  3. MAP_SHARED is requested and PROT_WRITE is set but the file descriptor is not opened for "
                       "writing.\n"
                    << "  4. PROT_WRITE is set but the file descriptor is set to append-only.");
        return PosixMemoryMapError::ACCESS_FAILED;
    case EAGAIN:
        IOX_LOG(ERROR, "Either too much memory has been locked or the file is already locked.");
        return PosixMemoryMapError::UNABLE_TO_LOCK;
    case EBADF:
        IOX_LOG(ERROR, "Invalid file descriptor provided.");
        return PosixMemoryMapError::INVALID_FILE_DESCRIPTOR;
    case EEXIST:
        IOX_LOG(ERROR, "The mapped range that is requested is overlapping with an already mapped memory range.");
        return PosixMemoryMapError::MAP_OVERLAP;
    case EINVAL:
        IOX_LOG(ERROR,
                "One or more of the following failures happened:\n"
                    << "  1. The address, length or the offset is not aligned on a page boundary.\n"
                    << "  2. The provided length is 0.\n"
                    << "  3. One of the flags of MAP_PRIVATE, MAP_SHARED or MAP_SHARED_VALIDATE is missing.");
        return PosixMemoryMapError::INVALID_PARAMETERS;
    case ENFILE:
        IOX_LOG(ERROR, "System limit of maximum open files reached");
        return PosixMemoryMapError::OPEN_FILES_SYSTEM_LIMIT_EXCEEDED;
    case ENODEV:
        IOX_LOG(ERROR, "Memory mappings are not supported by the underlying filesystem.");
        return PosixMemoryMapError::FILESYSTEM_DOES_NOT_SUPPORT_MEMORY_MAPPING;
    case ENOMEM:
        IOX_LOG(
            ERROR,
            "One or more of the following failures happened:\n"
                << "  1. Not enough memory available.\n"
                << "  2. The maximum supported number of mappings is exceeded.\n"
                << "  3. Partial unmapping of an already mapped memory region dividing it into two parts.\n"
                << "  4. The processes maximum size of data segments is exceeded.\n"
                << "  5. The sum of the number of pages used for length and the pages used for offset would overflow "
                   "and unsigned long. (only 32-bit architecture)");
        return PosixMemoryMapError::NOT_ENOUGH_MEMORY_AVAILABLE;
    case EOVERFLOW:
        IOX_LOG(ERROR, "The sum of the number of pages and offset are overflowing. (only 32-bit architecture)");
        return PosixMemoryMapError::OVERFLOWING_PARAMETERS;
    case EPERM:
        IOX_LOG(ERROR,
                "One or more of the following failures happened:\n"
                    << "  1. Mapping a memory region with PROT_EXEC which belongs to a filesystem that has no-exec.\n"
                    << "  2. The corresponding file is sealed.");
        return PosixMemoryMapError::PERMISSION_FAILURE;
    case ETXTBSY:
        IOX_LOG(ERROR, "The memory region was set up with MAP_DENYWRITE but write access was requested.");
        return PosixMemoryMapError::NO_WRITE_PERMISSION;
    default:
        IOX_LOG(ERROR, "This should never happened. An unknown error occurred!\n");
        return PosixMemoryMapError::UNKNOWN_ERROR;
    };
}

PosixMemoryMap::PosixMemoryMap(PosixMemoryMap&& rhs) noexcept
{
    *this = std::move(rhs);
}

PosixMemoryMap& PosixMemoryMap::operator=(PosixMemoryMap&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (!destroy())
        {
            IOX_LOG(ERROR, "move assignment failed to unmap mapped memory");
        }

        m_baseAddress = rhs.m_baseAddress;
        m_length = rhs.m_length;

        rhs.m_baseAddress = nullptr;
        rhs.m_length = 0U;
    }
    return *this;
}

PosixMemoryMap::~PosixMemoryMap() noexcept
{
    if (!destroy())
    {
        IOX_LOG(ERROR, "destructor failed to unmap mapped memory");
    }
}

const void* PosixMemoryMap::getBaseAddress() const noexcept
{
    return m_baseAddress;
}

void* PosixMemoryMap::getBaseAddress() noexcept
{
    return m_baseAddress;
}

bool PosixMemoryMap::destroy() noexcept
{
    if (m_baseAddress != nullptr)
    {
        auto unmapResult =
            IOX_POSIX_CALL(munmap)(m_baseAddress, static_cast<size_t>(m_length)).failureReturnValue(-1).evaluate();
        m_baseAddress = nullptr;
        m_length = 0U;

        if (unmapResult.has_error())
        {
            errnoToEnum(unmapResult.error().errnum);
            IOX_LOG(ERROR,
                    "unable to unmap mapped memory [ address = " << iox::log::hex(m_baseAddress)
                                                                 << ", size = " << m_length << " ]");
            return false;
        }
    }

    return true;
}

} // namespace detail
} // namespace iox
