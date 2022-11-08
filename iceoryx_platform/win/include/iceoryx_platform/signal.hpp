// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_SIGNAL_HPP
#define IOX_HOOFS_WIN_PLATFORM_SIGNAL_HPP

#include "iceoryx_platform/types.hpp"

#include <signal.h>

#define SIGEV_THREAD 0
#define SIGBUS SIGSEGV // SIGBUS/SIGSEGV signal serious program erros, SIGSEGV is best alternative
#define SIGHUP SIGTERM // SIGHUP signaled when controlling terminal is closed, SIGTERM is best alternative
#define SIGKILL 9

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


int sigemptyset(sigset_t* set);
int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);
int kill(pid_t pid, int sig);

#endif // IOX_HOOFS_WIN_PLATFORM_SIGNAL_HPP
