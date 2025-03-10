// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
// Copyright (c) 2025 by Valour inc. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_LISTENER_INL
#define IOX_POSH_EXPERIMENTAL_LISTENER_INL

#include "iox/posh/experimental/listener.hpp"

namespace iox::posh::experimental
{
inline ListenerBuilder::ListenerBuilder(runtime::PoshRuntime& runtime) noexcept
    : m_runtime(runtime)
{
}

inline expected<unique_ptr<Listener>, ListenerBuilderError> ListenerBuilder::create() noexcept
{
    auto* condition_variable_data = m_runtime.getMiddlewareConditionVariable();

    if (condition_variable_data == nullptr)
    {
        return err(ListenerBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<Listener>{new (std::nothrow) Listener{*condition_variable_data}, [&](auto* const listener) {
                                       // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
                                       delete listener;
                                   }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_LISTENER_INL
