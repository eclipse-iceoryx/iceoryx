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
#ifndef IOX_HOOFS_UNIX_PLATFORM_STAT_HPP
#define IOX_HOOFS_UNIX_PLATFORM_STAT_HPP

#include <sys/stat.h>

using iox_stat = struct stat;
using iox_mode_t = mode_t;

inline int iox_fstat(int fildes, iox_stat* buf)
{
    return fstat(fildes, buf);
}

inline int iox_fchmod(int fildes, iox_mode_t mode)
{
    return fchmod(fildes, mode);
}

#endif // IOX_HOOFS_UNIX_PLATFORM_STAT_HPP
