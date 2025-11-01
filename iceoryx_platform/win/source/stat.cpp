// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/mman.hpp"

int iox_fstat(int fildes, iox_stat* buf)
{
    HANDLE handle = HandleTranslator::getInstance().get(fildes);
    if (handle == INVALID_HANDLE_VALUE)
    {
        int ret_val = _fstat64(fildes, buf);
        buf->st_mode = std::numeric_limits<decltype(buf->st_mode)>::max();
        return ret_val;
    }

    auto size = internal_iox_shm_get_size(fildes);
    if (size != -1)
    {
        buf->st_size = size;
        return 0;
    }

    buf->st_size = Win32Call(GetFileSize, (HANDLE)handle, (LPDWORD)NULL).value;
    return 0;
}

int iox_fchmod(int fildes, iox_mode_t mode)
{
    return 0;
}
