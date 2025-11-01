// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
#define IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/detail/mpmc_resizeable_lockfree_queue.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/detail/resizeable_lockfree_queue.hpp' instead.")

namespace iox
{
namespace concurrent
{
template <typename ElementType, uint64_t MaxCapacity>
using ResizeableLockFreeQueue IOX_DEPRECATED_SINCE(3,
                                                   "Please use 'iox::concurrent::MpmcResizeableLockFreeQueue' from "
                                                   "'iox/detail/resizeable_lockfree_queue.hpp' instead.") =
    MpmcResizeableLockFreeQueue<ElementType, MaxCapacity>;
}
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_RESIZEABLE_LOCKFREE_QUEUE_HPP
