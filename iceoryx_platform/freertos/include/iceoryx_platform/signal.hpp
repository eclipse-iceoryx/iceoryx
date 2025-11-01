// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_SIGNAL_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_SIGNAL_HPP

#include "iceoryx_platform/pthread.hpp"

#include "FreeRTOS.h"
#include "FreeRTOS_POSIX/signal.h"

#include <sys/types.h>

#define SIGHUP 1
#define SIGINT 2
#define SIGABRT 6
#define SIGBUS 7
#define SIGTERM 15

#define SIGKILL 9
#define SIGTERM 15

using siginfo_t = int;

struct sigaction
{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t*, void*);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

inline int sigaction(int, const struct sigaction*, struct sigaction*)
{
    configASSERT(false);
    return -1;
}

inline int sigemptyset(sigset_t*)
{
    configASSERT(false);
    return -1;
}

inline int kill(pid_t, int)
{
    configASSERT(false);
    return -1;
}

#endif // IOX_HOOFS_FREERTOS_PLATFORM_SIGNAL_HPP
