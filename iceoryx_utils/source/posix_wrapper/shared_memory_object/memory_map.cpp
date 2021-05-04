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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <bitset>

namespace iox
{
namespace posix
{
MemoryMap::MemoryMap(const void* baseAddressHint,
                     const uint64_t length,
                     const int32_t fileDescriptor,
                     const AccessMode accessMode,
                     const int32_t flags,
                     const off_t offset) noexcept
    : m_length(length)
{
    int32_t l_memoryProtection{PROT_NONE};
    switch (accessMode)
    {
    case AccessMode::READ_ONLY:
        l_memoryProtection = PROT_READ;
        break;
    case AccessMode::READ_WRITE:
        l_memoryProtection = PROT_READ | PROT_WRITE;
        break;
    }
    // PRQA S 3066 1 # incompatibility with POSIX definition of mmap

    auto mmapCall = cxx::makeSmartC(static_cast<void* (*)(void*, size_t, int, int, int, off_t)>(mmap), // PRQA S 3066
                                    cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                    // we have to perform reinterpret cast since mmap returns MAP_FAILED on error which
                                    // is defined as (void*) -1; see man mmap for that definition
                                    {reinterpret_cast<void*>(MAP_FAILED)},
                                    {},
                                    const_cast<void*>(baseAddressHint),
                                    m_length,
                                    l_memoryProtection,
                                    flags,
                                    fileDescriptor,
                                    offset);

    if (mmapCall.hasErrors())
    {
        std::cerr << "Unable to map memory with the following properties [ baseAddressHint = " << std::hex
                  << baseAddressHint << ", length = " << std::dec << m_length << ", fileDescriptor = " << fileDescriptor
                  << ", access mode = " << ACCESS_MODE_STRING[static_cast<uint64_t>(accessMode)]
                  << ", flags = " << std::bitset<32>(static_cast<uint32_t>(flags)) << ", offset = " << std::hex
                  << offset << std::dec << " ]" << std::endl;
        m_errorValue = errnoToEnum(mmapCall.getErrNum());
        m_isInitialized = false;
        m_baseAddress = nullptr;
        m_length = 0U;
    }
    else
    {
        m_isInitialized = true;
        m_baseAddress = mmapCall.getReturnValue();
    }
}

MemoryMapError MemoryMap::errnoToEnum(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
        std::cerr
            << "One or more of the following failures happened:\n"
            << "  1. The file descriptor belongs to a non-regular file.\n"
            << "  2. The file descriptor is not opened for reading.\n"
            << "  3. MAP_SHARED is requested and PROT_WRITE is set but the file descriptor is not opened for writing.\n"
            << "  4. PROT_WRITE is set but the file descriptor is set to append-only." << std::endl;
        return MemoryMapError::ACCESS_FAILED;
    case EAGAIN:
        std::cerr << "Either too much memory has been locked or the file is already locked." << std::endl;
        return MemoryMapError::UNABLE_TO_LOCK;
    case EBADF:
        std::cerr << "Invalid file descriptor provided." << std::endl;
        return MemoryMapError::INVALID_FILE_DESCRIPTOR;
    case EEXIST:
        std::cerr << "The mapped range that is requested is overlapping with an already mapped memory range."
                  << std::endl;
        return MemoryMapError::MAP_OVERLAP;
    case EINVAL:
        std::cerr << "One or more of the following failures happened:\n"
                  << "  1. The address, length or the offset is not aligned on a page boundary.\n"
                  << "  2. The provided length is 0.\n"
                  << "  3. One of the flags of MAP_PRIVATE, MAP_SHARED or MAP_SHARED_VALIDATE is missing." << std::endl;
        return MemoryMapError::INVALID_PARAMETERS;
    case ENFILE:
        std::cerr << "System limit of maximum open files reached" << std::endl;
        return MemoryMapError::OPEN_FILES_SYSTEM_LIMIT_EXCEEDED;
    case ENODEV:
        std::cerr << "Memory mappings are not supported by the underlying filesystem." << std::endl;
        return MemoryMapError::FILESYSTEM_DOES_NOT_SUPPORT_MEMORY_MAPPING;
    case ENOMEM:
        std::cerr << "One or more of the following failures happened:\n"
                  << "  1. Not enough memory available.\n"
                  << "  2. The maximum supported number of mappings is exceeded.\n"
                  << "  3. Partial unmapping of an already mapped memory region dividing it into two parts.\n"
                  << "  4. The processes maximum size of data segments is exceeded.\n"
                  << "  5. The sum of the number of pages used for length and the pages used for offset would overflow "
                     "and unsigned long. (only 32-bit architecture)"
                  << std::endl;
        return MemoryMapError::NOT_ENOUGH_MEMORY_AVAILABLE;
    case EOVERFLOW:
        std::cerr << "The sum of the number of pages and offset are overflowing. (only 32-bit architecture)"
                  << std::endl;
        return MemoryMapError::OVERFLOWING_PARAMETERS;
    case EPERM:
        std::cerr << "One or more of the following failures happened:\n"
                  << "  1. Mapping a memory region with PROT_EXEC which belongs to a filesystem that has no-exec.\n"
                  << "  2. The corresponding file is sealed." << std::endl;
        return MemoryMapError::PERMISSION_FAILURE;
    case ETXTBSY:
        std::cerr << "The memory region was set up with MAP_DENYWRITE but write access was requested." << std::endl;
        return MemoryMapError::NO_WRITE_PERMISSION;
    default:
        std::cerr << "This should never happened. An unknown error occurred!\n";
        return MemoryMapError::UNKNOWN_ERROR;
    };
}

MemoryMap::MemoryMap(MemoryMap&& rhs) noexcept
{
    *this = std::move(rhs);
}

MemoryMap& MemoryMap::operator=(MemoryMap&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (!destroy())
        {
            std::cerr << "move assignment failed to unmap mapped memory" << std::endl;
        }

        CreationPattern_t::operator=(std::move(rhs));

        m_baseAddress = std::move(rhs.m_baseAddress);
        m_length = std::move(rhs.m_length);

        rhs.m_baseAddress = nullptr;
        rhs.m_length = 0U;
    }
    return *this;
}

MemoryMap::~MemoryMap()
{
    if (!destroy())
    {
        std::cerr << "destructor failed to unmap mapped memory" << std::endl;
    }
}

bool MemoryMap::isInitialized() const noexcept
{
    return m_isInitialized;
}

void* MemoryMap::getBaseAddress() const noexcept
{
    return m_baseAddress;
}

bool MemoryMap::destroy() noexcept
{
    if (m_isInitialized)
    {
        m_isInitialized = false;
        auto unmapResult =
            cxx::makeSmartC(munmap, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_baseAddress, m_length);
        if (unmapResult.hasErrors())
        {
            m_errorValue = errnoToEnum(unmapResult.getErrNum());
            std::cerr << "unable to unmap mapped memory [ address = " << std::hex << m_baseAddress
                      << ", size = " << std::dec << m_length << " ]" << std::endl;
            return false;
        }
    }

    return true;
}

} // namespace posix
} // namespace iox
