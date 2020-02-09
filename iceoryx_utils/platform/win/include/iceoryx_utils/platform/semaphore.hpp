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

#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/time.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/win32-error.hpp"
#include "iceoryx_utils/platform/windows.hpp"


#include <cstdlib>
#include <sddl.h>
#include <stdio.h>
#include <time.h>
#include <type_traits>

#define SEM_FAILED 0

struct sem_t
{
    HANDLE handle;
};
static constexpr LONG MAX_SEMAPHORE_VALUE = LONG_MAX;
static constexpr int MAX_SEMAPHORE_NAME_LENGTH = 128;

int sem_getvalue(sem_t* sem, int* sval);
int sem_post(sem_t* sem);
int sem_wait(sem_t* sem);
int sem_trywait(sem_t* sem);
int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout);
int sem_close(sem_t* sem);
int sem_destroy(sem_t* sem);
int sem_init(sem_t* sem, int pshared, unsigned int value);
sem_t* sem_open(const char* name, int oflag, ...);
int sem_unlink(const char* name);
