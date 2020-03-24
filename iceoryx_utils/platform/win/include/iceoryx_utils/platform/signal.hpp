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

#include <signal.h>

#define SIGEV_THREAD 0
#define SIGBUS 1
#define SIGHUP 2

using sigset_t = int;
using siginfo_t = int;

union sigval
{
    int sival_int;
    void* sival_ptr;
};

struct sigevent
{
    int sigev_notify;
    int sigev_signo;
    union sigval sigev_value;

    void (*sigev_notify_function)(union sigval);
    void* sigev_notify_attributes;
    pid_t sigev_notify_thread_id;
};

struct sigaction
{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t*, void*);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};


inline int sigemptyset(sigset_t* set)
{
    return 0;
}

inline int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)
{
    return 0;
}

inline int kill(pid_t pid, int sig)
{
    return 0;
}
