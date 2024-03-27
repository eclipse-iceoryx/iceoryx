// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_platform/stdlib.hpp"

#ifndef IOX_PLATFORM_OVERRIDE_STDLIB_ALL

#include <cerrno>
#include <cstring>
#include <mutex>

namespace
{
std::mutex& env_mutex()
{
    static std::mutex mtx;
    return mtx;
}
} // namespace

#ifndef IOX_PLATFORM_OVERRIDE_STDLIB_GETENV_S

int iox_getenv_s(size_t* actual_size_with_null, char* buffer, size_t buffer_capacity, const char* name)
{
    std::lock_guard<std::mutex> lock(env_mutex());

    if (name == nullptr)
    {
        return EINVAL;
    }

    if (buffer == nullptr && buffer_capacity != 0)
    {
        return EINVAL;
    }

    auto value = getenv(name);

    if (value == nullptr)
    {
        if (actual_size_with_null != nullptr)
        {
            *actual_size_with_null = 0;
        }
        if (buffer != nullptr && buffer_capacity > 0)
        {
            buffer[0] = 0;
        }
    }
    else
    {
        constexpr size_t NULL_TERMINATOR_SIZE{1};
        auto env_var_size_with_null = strlen(value) + NULL_TERMINATOR_SIZE;
        if (actual_size_with_null != nullptr)
        {
            *actual_size_with_null = env_var_size_with_null;
        }

        if (env_var_size_with_null > buffer_capacity)
        {
            if (buffer != nullptr && buffer_capacity > 0)
            {
                buffer[0] = 0;
            }
            return ERANGE;
        }

        if (buffer != nullptr && buffer_capacity > 0)
        {
            strncpy(buffer, value, env_var_size_with_null);
            buffer[env_var_size_with_null - NULL_TERMINATOR_SIZE] = 0;
        }
    }

    return 0;
}

#endif // IOX_PLATFORM_OVERRIDE_STDLIB_GETENV_S


#ifndef IOX_PLATFORM_OVERRIDE_STDLIB_SETENV

int iox_setenv(const char* name, const char* value, int overwrite)
{
    std::lock_guard<std::mutex> lock(env_mutex());

    if (name == nullptr)
    {
        errno = EINVAL;
        return -1;
    }

    if (value == nullptr)
    {
        errno = EINVAL;
        return -1;
    }

    return setenv(name, value, overwrite);
}

#endif // IOX_PLATFORM_OVERRIDE_STDLIB_SETENV


#ifndef IOX_PLATFORM_OVERRIDE_STDLIB_UNSETENV

int iox_unsetenv(const char* name)
{
    std::lock_guard<std::mutex> lock(env_mutex());

    if (name == nullptr)
    {
        errno = EINVAL;
        return -1;
    }

    return unsetenv(name);
}

#endif // IOX_PLATFORM_OVERRIDE_STDLIB_UNSETENV


#endif // IOX_PLATFORM_OVERRIDE_STDLIB_ALL
