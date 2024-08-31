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
#ifndef IOX_HOOFS_WIN_PLATFORM_TYPES_HPP
#define IOX_HOOFS_WIN_PLATFORM_TYPES_HPP

#include <sys/types.h>

using iox_gid_t = int;
using iox_uid_t = int;
#if defined(_MSC_VER)
using mode_t = int;
using iox_ssize_t = int;
using pid_t = int;
#elif defined(__MINGW32__) || defined(__MINGW64__)
using iox_ssize_t = ssize_t;
#endif
using nlink_t = int;
using blksize_t = int;
using blkcnt_t = int;
// using off_t = int;

// mode_t umask(mode_t mask)
//{
//    return mode_t();
//}

#endif // IOX_HOOFS_WIN_PLATFORM_TYPES_HPP
