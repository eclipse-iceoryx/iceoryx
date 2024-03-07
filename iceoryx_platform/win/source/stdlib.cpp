// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include <mutex>

namespace
{
std::mutex& env_mutex()
{
    static std::mutex mtx;
    return mtx;
}
} // namespace

int iox_getenv_s(size_t* env_var_size_with_null, char* buffer, size_t buffer_capacity, const char* name)
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

    size_t actual_size_platform_adaption{0};
    size_t* actual_size_ptr =
        (env_var_size_with_null != nullptr) ? env_var_size_with_null : &actual_size_platform_adaption;

    char buffer_platform_adaption[1];
    char* buffer_ptr = (buffer != nullptr) ? buffer : buffer_platform_adaption;

    size_t capacity = (buffer != nullptr) ? buffer_capacity : 0;

    return getenv_s(actual_size_ptr, buffer_ptr, capacity, name);
}

int iox_setenv(const char* name, const char* value, int overwrite)
{
    std::lock_guard<std::mutex> lock(env_mutex());

    if (overwrite == 0 && getenv(name) != nullptr)
    {
        return 0;
    }
    return _putenv_s(name, value);
}

int iox_unsetenv(const char* name)
{
    std::lock_guard<std::mutex> lock(env_mutex());

    return _putenv_s(name, "");
}
