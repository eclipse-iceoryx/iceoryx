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

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/stat.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <assert.h>
#include <limits>

namespace iox
{
namespace posix
{
SharedMemory::SharedMemory(const char* name,
                           const AccessMode accessMode,
                           const OwnerShip ownerShip,
                           const mode_t permissions,
                           const uint64_t size) noexcept
    : m_ownerShip(ownerShip)
    , m_permissions(permissions)
    , m_size(size)
{
    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    if (name == nullptr || strlen(name) == 0U)
    {
        std::cerr << "No shared memory name specified!" << std::endl;
        m_isInitialized = false;
        return;
    }
    else if (name[0] != '/')
    {
        std::cerr << "Shared memory name must start with a leading slash!" << std::endl;
        m_isInitialized = false;
        return;
    }

    if (strlen(name) >= NAME_SIZE)
    {
        std::clog << "Shared memory name is too long! '" << name << "' will be truncated at position " << NAME_SIZE - 1U
                  << "!" << std::endl;
    }

    /// @note GCC drops here a warning that the destination char buffer length is equal to the max length to copy.
    /// This can potentially lead to a char array without null-terminator. We add the null-terminator afterwards.
    strncpy(m_name, name, NAME_SIZE);
    m_name[NAME_SIZE - 1U] = '\0';
    m_oflags |= (accessMode == AccessMode::readOnly) ? O_RDONLY : O_RDWR;
    m_oflags |= (ownerShip == OwnerShip::mine) ? O_CREAT | O_EXCL : 0;

    m_isInitialized = open();
}

SharedMemory::~SharedMemory() noexcept
{
    destroy();
}

void SharedMemory::destroy() noexcept
{
    if (m_isInitialized)
    {
        close();
        unlink();
    }
    reset();
}

void SharedMemory::reset() noexcept
{
    m_isInitialized = false;
    m_name[0] = '\0';
    m_oflags = 0;
    m_permissions = mode_t();
    m_size = 0U;
    m_handle = -1;
}

SharedMemory::SharedMemory(SharedMemory&& rhs) noexcept
{
    *this = std::move(rhs);
}

SharedMemory& SharedMemory::operator=(SharedMemory&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();

        m_isInitialized = std::move(rhs.m_isInitialized);
        strncpy(m_name, rhs.m_name, NAME_SIZE);
        m_ownerShip = std::move(rhs.m_ownerShip);
        m_oflags = std::move(rhs.m_oflags);
        m_permissions = std::move(rhs.m_permissions);
        m_handle = std::move(rhs.m_handle);

        rhs.reset();
    }
    return *this;
}

int32_t SharedMemory::getHandle() const noexcept
{
    return m_handle;
}

bool SharedMemory::open() noexcept
{
    cxx::Expects(static_cast<int64_t>(m_size) <= std::numeric_limits<int64_t>::max());

    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0U);

    // if we create the shm, cleanup old resources
    if (m_oflags & O_CREAT)
    {
        auto shmUnlinkCall =
            cxx::makeSmartC(shm_unlink, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {ENOENT}, m_name);
        if (!shmUnlinkCall.hasErrors() && shmUnlinkCall.getErrNum() != ENOENT)
        {
            std::cout << "SharedMemory still there, doing an unlink of " << m_name << std::endl;
        }
    }

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
        auto l_truncateCall = cxx::makeSmartC(
            ftruncate, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handle, static_cast<int64_t>(m_size));
        if (l_truncateCall.hasErrors())
        {
            if (l_truncateCall.getErrNum() == ENOMEM)
            {
                char errormsg[] = "\033[0;1;97;41mFatal error:\033[m the available memory is insufficient. Cannot "
                                  "allocate mempools in shared "
                                  "memory. Please make sure that enough memory is available. For this, consider also "
                                  "the memory which is "
                                  "required for the [/iceoryx_mgmt] segment. Please refer to share/doc/iceoryx/FAQ.md "
                                  "in your release delivery.";

                std::cerr << errormsg << std::endl;
                return false;
            }
            else
            {
                std::cerr << "Unable to truncate SharedMemory (ftruncate failed) : " << l_truncateCall.getErrorString()
                          << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool SharedMemory::unlink() noexcept
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

bool SharedMemory::close() noexcept
{
    if (m_isInitialized && m_handle != -1)
    {
        auto closeCall =
            cxx::makeSmartC(closePlatformFileHandle, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handle);
        m_handle = -1;
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
