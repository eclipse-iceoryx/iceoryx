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
#ifndef IOX_HOOFS_LINUX_PLATFORM_TIME_HPP
#define IOX_HOOFS_LINUX_PLATFORM_TIME_HPP

#include <sys/time.h>
#include <time.h>

using iox_clockid_t = clockid_t;

inline int iox_clock_gettime(iox_clockid_t clk_id, struct timespec* tp)
{
    return clock_gettime(clk_id, tp);
}
inline int iox_gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    return gettimeofday(tp, tzp);
}

#endif // IOX_HOOFS_LINUX_PLATFORM_TIME_HPP
