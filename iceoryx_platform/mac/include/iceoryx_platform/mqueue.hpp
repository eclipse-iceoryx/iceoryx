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
#ifndef IOX_HOOFS_MAC_PLATFORM_MQUEUE_HPP
#define IOX_HOOFS_MAC_PLATFORM_MQUEUE_HPP

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

inline int mq_send(mqd_t, const char*, size_t, unsigned int)
{
    return 0;
}

inline int mq_timedsend(mqd_t, const char*, size_t, unsigned int, const struct timespec*)
{
    return 0;
}

inline int mq_close(mqd_t)
{
    return 0;
}

inline ssize_t mq_receive(mqd_t, char*, size_t, unsigned int*)
{
    return 0;
}

inline ssize_t mq_timedreceive(mqd_t, char*, size_t, unsigned int*, const struct timespec*)
{
    return 0;
}

inline int mq_unlink(const char*)
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

#endif // IOX_HOOFS_MAC_PLATFORM_MQUEUE_HPP
