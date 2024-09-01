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
#ifndef IOX_HOOFS_WIN_PLATFORM_UNISTD_HPP
#define IOX_HOOFS_WIN_PLATFORM_UNISTD_HPP

#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/windows.hpp"

#include <io.h>
#include <process.h>
#include <stdio.h>
#include <vector>

#define IOX_SEEK_SET SEEK_SET
#define IOX_SC_PAGESIZE 1
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

using iox_off_t = long;

#define IOX_F_OK 0
#define IOX_X_OK 1
#define IOX_W_OK 2
#define IOX_R_OK 4

int iox_ftruncate(int fildes, off_t length);
long iox_sysconf(int name);
int iox_close(int fd);
int iox_ext_close(int fd);
int iox_fchown(int fd, iox_uid_t owner, iox_gid_t group);
int iox_access(const char* pathname, int mode);
int iox_unlink(const char* pathname);
iox_off_t iox_lseek(int fd, iox_off_t offset, int whence);
iox_ssize_t iox_read(int fd, void* buf, size_t count);
iox_ssize_t iox_write(int fd, const void* buf, size_t count);
iox_gid_t iox_getgid(void);
iox_uid_t iox_geteuid(void);


#endif // IOX_HOOFS_WIN_PLATFORM_UNISTD_HPP
