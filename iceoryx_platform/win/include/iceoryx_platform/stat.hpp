// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_STAT_HPP
#define IOX_HOOFS_WIN_PLATFORM_STAT_HPP

#include <io.h>
#include <limits>
#include <sys/stat.h>

#if defined(_MSC_VER)

#define S_IRUSR 0
#define S_IWUSR 1
#define S_IRGRP 2
#define S_IWGRP 3
#define S_IROTH 4
#define S_IWOTH 5
#define S_IRWXU 6
#define S_IXUSR 7
#define S_IXGRP 8
#define S_IRWXO 9

#endif

using iox_stat = struct __stat64;
using iox_mode_t = int;

int iox_fstat(int fildes, iox_stat* buf);
int iox_fchmod(int fildes, iox_mode_t mode);

#endif // IOX_HOOFS_WIN_PLATFORM_STAT_HPP
