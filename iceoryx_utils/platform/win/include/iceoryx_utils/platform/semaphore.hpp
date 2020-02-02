// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_utils/platform/types.hpp"

#define SEM_FAILED 0

using sem_t = int;

inline int sem_getvalue(sem_t* sem, int* sval)
{
    return 0;
}

inline int sem_post(sem_t* sem)
{
    return 0;
}

inline int sem_wait(sem_t* sem)
{
    return 0;
}

inline int sem_trywait(sem_t* sem)
{
    return 0;
}

inline int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout)
{
    return 0;
}

inline int sem_close(sem_t* sem)
{
    return 0;
}

inline int sem_destroy(sem_t* sem)
{
    return 0;
}

inline int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    return 0;
}

// sem_t *
// sem_open( const char *name, int oflag )
//{
//}

inline sem_t* sem_open(const char* name, int oflag, mode_t mode, unsigned int value)
{
    return 0;
}

inline int sem_unlink(const char* name)
{
    return 0;
}
