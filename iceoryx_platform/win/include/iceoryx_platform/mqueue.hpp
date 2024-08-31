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
#ifndef IOX_HOOFS_WIN_PLATFORM_MQUEUE_HPP
#define IOX_HOOFS_WIN_PLATFORM_MQUEUE_HPP

#include "iceoryx_platform/types.hpp"

#include <sys/stat.h>

using mqd_t = int;

struct mq_attr
{
    long mq_flags;
    long mq_maxmsg;
    long mq_msgsize;
    long mq_curmsgs;
};

inline int mq_send(mqd_t mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    return 0;
}

inline int mq_timedsend(
    mqd_t mqdes, const char* msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec* abs_timeout)
{
    return 0;
}

inline int mq_close(mqd_t mqdes)
{
    return 0;
}

// mqd_t
// mq_open( const char *name, int oflag )
//{
//}

inline iox_ssize_t mq_receive(mqd_t mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio)
{
    return 0;
}

inline iox_ssize_t
mq_timedreceive(mqd_t mqdes, char* msg_ptr, size_t msg_len, unsigned int* msg_prio, const struct timespec* abs_timeout)
{
    return 0;
}

inline int mq_unlink(const char* name)
{
    return 0;
}

inline mqd_t iox_mq_open2(const char*, int)
{
    return 0;
}

inline mqd_t iox_mq_open4(const char*, int, mode_t, struct mq_attr*)
{
    return 0;
}

#endif // IOX_HOOFS_WIN_PLATFORM_MQUEUE_HPP
