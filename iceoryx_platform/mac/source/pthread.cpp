// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_platform/pthread.hpp"

#include <map>
#include <mutex>
#include <string>
#include <utility>

/// @note Since pthread_setname_np and pthread_getname_np are missing in MacOS, their functionality is simulated via a
/// map of thread handle and thread name.
static std::map<iox_pthread_t, std::string> handleNameMap;
static std::mutex handleNameMapMutex;

int iox_pthread_setname_np(iox_pthread_t thread, const char* name)
{
    std::lock_guard<std::mutex> lock(handleNameMapMutex);
    const auto result = handleNameMap.insert(std::make_pair(thread, name));
    if (!result.second)
    {
        /// @todo iox-#1365 return error code once std::thread is replaced; not possible now because iox_pthread_join is
        /// not called which erases the map entry
        return 0;
    }
    return 0;
}

int iox_pthread_getname_np(iox_pthread_t thread, char* name, size_t len)
{
    std::lock_guard<std::mutex> lock(handleNameMapMutex);
    const auto result = handleNameMap.find(thread);
    if (result == handleNameMap.end())
    {
        return -1;
    }
    strncpy(name, result->second.c_str(), std::min(len, result->second.size() + 1));
    return 0;
}

int iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    return pthread_create(thread, attr, start_routine, arg);
}

int iox_pthread_join(iox_pthread_t thread, void** retval)
{
    {
        std::lock_guard<std::mutex> lock(handleNameMapMutex);
        handleNameMap.erase(thread);
    }
    return pthread_join(thread, retval);
}

iox_pthread_t iox_pthread_self()
{
    return pthread_self();
}

int pthread_mutexattr_setrobust(pthread_mutexattr_t*, int)
{
    return 0;
}
