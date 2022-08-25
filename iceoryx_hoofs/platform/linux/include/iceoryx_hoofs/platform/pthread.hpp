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
#ifndef IOX_HOOFS_LINUX_PLATFORM_PTHREAD_HPP
#define IOX_HOOFS_LINUX_PLATFORM_PTHREAD_HPP

#include <pthread.h>

using iox_pthread_t = pthread_t;
using iox_pthread_attr_t = pthread_attr_t;

inline int iox_pthread_setname_np(iox_pthread_t thread, const char* name)
{
    return pthread_setname_np(thread, name);
}

inline int iox_pthread_getname_np(iox_pthread_t thread, char* name, size_t len)
{
    return pthread_getname_np(thread, name, len);
}

inline int
iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    return pthread_create(thread, attr, start_routine, arg);
}

inline int iox_pthread_join(iox_pthread_t thread, void** retval)
{
    return pthread_join(thread, retval);
}

inline iox_pthread_t iox_pthread_self()
{
    return pthread_self();
}

#endif // IOX_HOOFS_LINUX_PLATFORM_PTHREAD_HPP
