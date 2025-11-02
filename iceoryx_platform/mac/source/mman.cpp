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

#include "iceoryx_platform/mman.hpp"

#include <errno.h>
#include <sys/shm.h>
#include <unistd.h>

int iox_shm_open(const char* name, int oflag, mode_t mode)
{
    return shm_open(name, oflag, mode);
}

int iox_shm_unlink(const char* name)
{
    int state = shm_unlink(name);
    // according to mac os shm_unlink sets errno to ENOENT when the name is invalid
    // and never EINVAL - but it is actually set when ENOENT should be set.
    // See:
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/shm_unlink.2.html
    if (errno == EINVAL)
    {
        errno = ENOENT;
    }
    return state;
}

int iox_shm_close(int fd)
{
    return close(fd);
}
