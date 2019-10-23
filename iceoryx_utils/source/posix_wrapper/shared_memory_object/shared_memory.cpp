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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace iox
{
namespace posix
{
cxx::optional<SharedMemory> SharedMemory::create(const char* f_name,
                                                 const AccessMode f_accessMode,
                                                 const OwnerShip f_ownerShip,
                                                 const mode_t f_permissions,
                                                 const uint64_t f_size)
{
    cxx::optional<SharedMemory> l_sharedMemory;
    l_sharedMemory.emplace(f_name, f_accessMode, f_ownerShip, f_permissions, f_size);

    if (l_sharedMemory->isInitialized())
    {
        return l_sharedMemory;
    }
    else
    {
        return cxx::nullopt_t();
    }
}

SharedMemory::SharedMemory(const char* f_name,
                           const AccessMode f_accessMode,
                           const OwnerShip f_ownerShip,
                           const mode_t f_permissions,
                           const uint64_t f_size)
    : m_ownerShip(f_ownerShip)
    , m_permissions(f_permissions)
    , m_size(f_size)
{
    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    if (f_name == nullptr || strlen(f_name) == 0)
    {
        std::cerr << "No shared memory name specified!" << std::endl;
        std::terminate();
    }
    else if (f_name[0] != '/')
    {
        std::cerr << "Shared memory name must start with a leading slash!" << std::endl;
        std::terminate();
    }

    if (strlen(f_name) >= NAME_SIZE)
    {
        std::clog << "Shared memory name is too long! '" << f_name << "' will be truncated at position "
                  << NAME_SIZE - 1 << "!" << std::endl;
    }

    strncpy(m_name, f_name, NAME_SIZE);
    m_oflags |= (f_accessMode == AccessMode::readOnly) ? O_RDONLY : O_RDWR;
    m_oflags |= (f_ownerShip == OwnerShip::mine) ? O_CREAT : 0;

    m_isInitialized = open();
}

SharedMemory::~SharedMemory()
{
    if (m_isInitialized)
    {
        close();
        unlink();
    }
}

SharedMemory::SharedMemory(SharedMemory&& rhs)
{
    *this = std::move(rhs);
}

SharedMemory& SharedMemory::operator=(SharedMemory&& rhs)
{
    if (this != &rhs)
    {
        m_isInitialized = std::move(rhs.m_isInitialized);
        strncpy(m_name, rhs.m_name, NAME_SIZE);
        m_ownerShip = std::move(rhs.m_ownerShip);
        m_oflags = std::move(rhs.m_oflags);
        m_permissions = std::move(rhs.m_permissions);
        m_handle = std::move(rhs.m_handle);

        rhs.m_isInitialized = false;
    }
    return *this;
}

bool SharedMemory::isInitialized() const
{
    return m_isInitialized;
}

int SharedMemory::getHandle() const
{
    return m_handle;
}

bool SharedMemory::open()
{
    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0);

    auto l_shmOpenCall =
        cxx::makeSmartC(shm_open, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_name, m_oflags, m_permissions);

    umask(umaskSaved);

    if (l_shmOpenCall.hasErrors())
    {
        std::cerr << "Unable to initialize SharedMemory (shm_open failed) : " << l_shmOpenCall.getErrorString()
                  << std::endl;
        return false;
    }

    m_handle = l_shmOpenCall.getReturnValue();

    if (m_ownerShip == OwnerShip::mine)
    {
        auto l_truncateCall =
            cxx::makeSmartC(ftruncate, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handle, m_size);
        if (l_truncateCall.hasErrors())
        {
            std::cerr << "Unable to truncate SharedMemory (ftruncate failed) : " << l_truncateCall.getErrorString()
                      << std::endl;
            return false;
        }
    }

    return true;
}

bool SharedMemory::unlink()
{
    if (m_isInitialized && m_ownerShip == OwnerShip::mine)
    {
        auto unlinkCall = cxx::makeSmartC(shm_unlink, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_name);
        if (unlinkCall.hasErrors())
        {
            std::cerr << "Unable to unlink SharedMemory (shm_unlink failed) : " << unlinkCall.getErrorString()
                      << std::endl;
            return false;
        }
    }
    return true;
}

bool SharedMemory::close()
{
    if (m_isInitialized)
    {
        auto closeCall = cxx::makeSmartC(::close, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handle);
        if (closeCall.hasErrors())
        {
            std::cerr << "Unable to close SharedMemory filedescriptor (close failed) : " << closeCall.getErrorString()
                      << std::endl;
            return false;
        }
    }
    return true;
}
} // namespace posix
} // namespace iox
