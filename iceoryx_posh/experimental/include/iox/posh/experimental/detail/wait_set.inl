// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_EXPERIMENTAL_WAIT_SET_INL
#define IOX_POSH_EXPERIMENTAL_WAIT_SET_INL

#include "iox/posh/experimental/wait_set.hpp"

namespace iox::posh::experimental
{
inline WaitSetBuilder::WaitSetBuilder(runtime::PoshRuntime& runtime) noexcept
    : m_runtime(runtime)
{
}

template <uint64_t Capacity>
inline expected<unique_ptr<WaitSet<Capacity>>, WaitSetBuilderError> WaitSetBuilder::create() noexcept
{
    auto* condition_variable_data = m_runtime.getMiddlewareConditionVariable();

    if (condition_variable_data == nullptr)
    {
        return err(WaitSetBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<WaitSet<Capacity>>{new (std::nothrow) WaitSet<Capacity>{*condition_variable_data},
                                            [&](auto* const ws) {
                                                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
                                                delete ws;
                                            }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_WAIT_SET_INL
