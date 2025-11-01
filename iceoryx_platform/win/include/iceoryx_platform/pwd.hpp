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
#ifndef IOX_HOOFS_WIN_PLATFORM_PWD_HPP
#define IOX_HOOFS_WIN_PLATFORM_PWD_HPP

#include "iceoryx_platform/types.hpp"

struct passwd
{
    const char* pw_name;
    const char* pw_passwd;
    iox_uid_t pw_uid;
    iox_gid_t pw_gid;
    const char* pw_gecos;
    const char* pw_dir;
    const char* pw_shell;
};


inline struct passwd* getpwnam(const char* name)
{
    static const char* value = "iceoryx_windows_dummy";
    static struct passwd dummy;
    dummy.pw_name = value;
    dummy.pw_passwd = value;
    dummy.pw_uid = 0;
    dummy.pw_gid = 0;
    dummy.pw_gecos = 0;
    dummy.pw_dir = value;
    dummy.pw_shell = value;
    return &dummy;
}

inline struct passwd* getpwuid(iox_uid_t uid)
{
    static const char* value = "iceoryx_windows_dummy";
    static struct passwd dummy;
    dummy.pw_name = value;
    dummy.pw_passwd = value;
    dummy.pw_uid = 0;
    dummy.pw_gid = 0;
    dummy.pw_gecos = 0;
    dummy.pw_dir = value;
    dummy.pw_shell = value;
    return &dummy;
}

#endif // IOX_HOOFS_WIN_PLATFORM_PWD_HPP
