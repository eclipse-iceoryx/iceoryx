// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_platform/file.hpp"
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"
#include "iceoryx_platform/windows.hpp"

int iox_flock(int fd, int op)
{
    HANDLE handle = HandleTranslator::getInstance().get(fd);
    if (op & LOCK_EX)
    {
        if (Win32Call(LockFile, handle, 0, 0, 0, 0).value == FALSE)
        {
            return -1;
        }
    }
    else if (op & LOCK_SH)
    {
        OVERLAPPED overlapped;
        if (Win32Call(LockFileEx, handle, 0, 0, 0, 0, &overlapped).value == FALSE)
        {
            return -1;
        }
    }
    else if (op & LOCK_UN)
    {
        if (Win32Call(UnlockFile, handle, 0, 0, 0, 0).value == FALSE)
        {
            return -1;
        }
    }
    return 0;
}
