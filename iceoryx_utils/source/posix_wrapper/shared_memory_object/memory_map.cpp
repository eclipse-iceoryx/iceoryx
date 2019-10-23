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
                                           const int f_fileDescriptor,
                                           const AccessMode f_accessMode,
                                           const int f_flags,
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
                     const int f_fileDescriptor,
                     const AccessMode f_accessMode,
                     const int f_flags,
                     const off_t f_offset)
    : m_length(f_length)
{
    int l_memoryProtection;
    switch (f_accessMode)
    {
    case AccessMode::readOnly:
        l_memoryProtection = PROT_READ;
        break;
    case AccessMode::readWrite:
        l_memoryProtection = PROT_READ | PROT_WRITE;
        break;
    }
    auto mmapCall = cxx::makeSmartC(mmap,
                                    cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                    {MAP_FAILED},
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
}


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
