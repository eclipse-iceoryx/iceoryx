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

#include "iox/detail/posix_scheduler.hpp"
#include "iox/assertions.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
namespace detail
{
int32_t getSchedulerPriorityMinimum(const Scheduler scheduler) noexcept
{
    auto result = IOX_POSIX_CALL(sched_get_priority_min)(static_cast<int>(scheduler)).failureReturnValue(-1).evaluate();
    if (result.has_error())
    {
        IOX_PANIC("The \"sched_get_priority_min\" should never fail! Either the system is not POSIX compliant or an "
                  "invalid integer was casted to the \"Scheduler\" enum class.");
    }
    return result.value().value;
}

int32_t getSchedulerPriorityMaximum(const Scheduler scheduler) noexcept
{
    auto result = IOX_POSIX_CALL(sched_get_priority_max)(static_cast<int>(scheduler)).failureReturnValue(-1).evaluate();
    if (result.has_error())
    {
        IOX_PANIC("The \"sched_get_priority_max\" should never fail! Either the system is not POSIX compliant or an "
                  "invalid integer was casted to the \"Scheduler\" enum class.");
    }
    return result.value().value;
}
} // namespace detail
} // namespace iox
