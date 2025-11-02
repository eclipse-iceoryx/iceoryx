// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_MEMORY_SCOPED_STATIC_INL
#define IOX_HOOFS_MEMORY_SCOPED_STATIC_INL

#include "iox/scoped_static.hpp"

namespace iox
{
template <typename T, typename... CTorArgs>
inline ScopeGuard makeScopedStatic(T& memory, CTorArgs&&... ctorArgs) noexcept
{
    memory.emplace(std::forward<CTorArgs>(ctorArgs)...);
    /// @todo iox-#1392 make noexcept dependent on the actual used type
    return ScopeGuard([&memory]() noexcept { memory.reset(); });
}
} // namespace iox

#endif // IOX_HOOFS_MEMORY_SCOPED_STATIC_INL
