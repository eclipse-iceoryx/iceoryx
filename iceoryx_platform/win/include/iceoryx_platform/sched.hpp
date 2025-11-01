// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_SCHED_HPP
#define IOX_HOOFS_WIN_PLATFORM_SCHED_HPP

#if defined(__GNUC__) || defined(__GNUG__)

#include <sched.h>

#elif defined(_MSC_VER)

#define SCHED_FIFO 1

int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);

#endif

#endif
