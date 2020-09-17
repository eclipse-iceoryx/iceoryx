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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

namespace iox
{
namespace posix
{
cxx::optional<MemoryMap> MemoryMap::create(const void* f_baseAddressHint,
                                           const uint64_t f_length,
                                           const int32_t f_fileDescriptor,
                                           const AccessMode f_accessMode,
                                           const int32_t f_flags,
                                           const off_t f_offset)
{
    cxx::optional<MemoryMap> returnValue;
    returnValue.emplace(f_baseAddressHint, f_length, f_fileDescriptor, f_accessMode, f_flags, f_offset);

    if (returnValue->isInitialized())
    {
        return returnValue;
    }
    else
    {
        return cxx::nullopt_t();
    }
}

MemoryMap::MemoryMap(const void* f_baseAddressHint,
                     const uint64_t f_length,
                     const int32_t f_fileDescriptor,
                     const AccessMode f_accessMode,
                     const int32_t f_flags,
                     const off_t f_offset)
    : m_length(f_length)
{
    int32_t l_memoryProtection;
    switch (f_accessMode)
    {
    case AccessMode::readOnly:
        l_memoryProtection = PROT_READ;
        break;
    case AccessMode::readWrite:
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
                                    const_cast<void*>(f_baseAddressHint),
                                    f_length,
                                    l_memoryProtection,
                                    f_flags,
                                    f_fileDescriptor,
                                    f_offset);

    if (mmapCall.hasErrors())
    {
        std::cerr << "Failed to perform memory mapping : " << mmapCall.getErrorString() << std::endl;
        m_isInitialized = false;
        m_baseAddress = nullptr;
    }
    else
    {
        m_isInitialized = true;
        m_baseAddress = mmapCall.getReturnValue();
    }

    /// lock all memory pages in QNX only for better performance on acquiring memory on RouDi
    /// RouDi must be run as root! Otherwise
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
    int l_lockflags = MCL_CURRENT;

    auto mlockCall = cxx::makeSmartC(mlockall, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, l_lockflags);

    if (mlockCall.hasErrors())
    {
        std::cerr << "Failed to perform memory mapping : " << mmapCall.getErrorString() << std::endl;
        m_isLocked = false;
    }
    else
    {
        m_isLocked = true;
    }
#endif
} // namespace posix


MemoryMap::MemoryMap(MemoryMap&& rhs)
{
    *this = std::move(rhs);
}

MemoryMap& MemoryMap::operator=(MemoryMap&& rhs)
{
    if (this != &rhs)
    {
        m_isInitialized = std::move(rhs.m_isInitialized);
        m_baseAddress = std::move(rhs.m_baseAddress);
        m_length = std::move(rhs.m_length);

        rhs.m_isInitialized = false;
    }
    return *this;
}

MemoryMap::~MemoryMap()
{
    if (m_isInitialized)
    {
        cxx::makeSmartC(munmap, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_baseAddress, m_length);
    }
}

bool MemoryMap::isInitialized() const
{
    return m_isInitialized;
}

void* MemoryMap::getBaseAddress() const
{
    return m_baseAddress;
}
} // namespace posix
} // namespace iox
