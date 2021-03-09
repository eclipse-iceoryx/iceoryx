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

#include "iceoryx_utils/posix_wrapper/file_lock.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

#include <sys/file.h>

namespace iox
{
namespace posix
{
FileLock::FileLock(FileName_t name) noexcept
{
    cxx::string<200> fullPath = PATH_PREFIX + name;
    constexpr int OFlags = O_CREAT | O_RDWR;

    auto openCall =
        cxx::makeSmartC(open, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, fullPath.c_str(), OFlags);

    if (openCall.hasErrors())
    {
        m_isInitialized = false;
    }
    else
    {
        m_fd = openCall.getReturnValue();

        auto lockCall =
            cxx::makeSmartC(flock, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_fd, LOCK_EX | LOCK_NB);

        if (!lockCall.hasErrors())
        {
            m_isInitialized = true;
        }
        else
        {
            if (lockCall.getErrNum() == EWOULDBLOCK)
            {
                m_errorValue = FileLockError::LOCKED_BY_OTHER_PROCESS;
            }
        }
    }
}

FileLock::~FileLock() noexcept
{}

} // namespace posix
} // namespace iox
