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

#ifndef IOX_POSH_EXPERIMENTAL_LISTENER_HPP
#define IOX_POSH_EXPERIMENTAL_LISTENER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{

using Listener = iox::popo::Listener;

enum class ListenerBuilderError : uint8_t
{
    OUT_OF_RESOURCES,
};

/// @brief A builder for the listener
class ListenerBuilder
{
  public:
    /// @brief Creates a listener
    /// @return a 'listener' on success and a 'ListenerBuilderError' on failure
    expected<unique_ptr<Listener>, ListenerBuilderError> create() noexcept;

  private:
    friend class Node;
    explicit ListenerBuilder(runtime::PoshRuntime& runtime) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the ListenerBuilder is not intended to be moved
    runtime::PoshRuntime& m_runtime;
};
} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/listener.inl"

#endif // IOX_POSH_EXPERIMENTAL_LISTENER_HPP
