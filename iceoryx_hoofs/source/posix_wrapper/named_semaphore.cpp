// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/named_semaphore.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
cxx::expected<SemaphoreError>
NamedSemaphoreBuilder::create(cxx::optional<NamedSemaphore>& uninitializedSemaphore) noexcept
{
    if (!cxx::isValidFilePath(m_name))
    {
        return cxx::error<SemaphoreError>(SemaphoreError::INVALID_NAME);
    }

    if (m_openMode == OpenMode::PURGE_AND_CREATE)
    {
        auto result = posixCall(iox_sem_unlink)(m_name.c_str()).failureReturnValue(-1).ignoreErrnos(ENOENT).evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case EACCES:
                LogError() << "You don't have permission to remove to remove the semaphore \"" << m_name << "\"";
                return cxx::error<SemaphoreError>(SemaphoreError::PERMISSION_DENIED);
            default:
                LogError() << "This should never happen. An unknown error occurred.";
                return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
            }
        }
    }

    auto result = posixCall(iox_sem_open_ext)(m_name.c_str(),
                                              convertToOflags(m_openMode),
                                              static_cast<mode_t>(m_permissions),
                                              static_cast<unsigned int>(m_initialValue));
}

iox_sem_t* NamedSemaphore::getHandle() noexcept
{
    return m_handle;
}


} // namespace posix
} // namespace iox
