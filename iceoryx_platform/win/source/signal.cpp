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

#include "iceoryx_platform/signal.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"

int sigemptyset(sigset_t* set)
{
    return 0;
}

int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)
{
    auto previousSignalHandler = Win32Call(signal, signum, act->sa_handler).value;
    if (oldact != nullptr)
    {
        oldact->sa_handler = previousSignalHandler;
    }
    return 0;
}

int kill(pid_t pid, int sig)
{
    return 0;
}
