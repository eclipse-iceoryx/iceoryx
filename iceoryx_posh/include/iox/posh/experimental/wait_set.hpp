// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_WAIT_SET_HPP
#define IOX_POSH_EXPERIMENTAL_WAIT_SET_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"

namespace iox::posh::experimental
{
template <uint64_t Capacity = MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET>
using WaitSet = iox::popo::WaitSet<Capacity>;

enum class WaitSetBuilderError
{
    OUT_OF_RESOURCES,
};

class WaitSetBuilder
{
  public:
    template <uint64_t Capacity = MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET>
    expected<void, WaitSetBuilderError> create(iox::optional<WaitSet<Capacity>>& ws) noexcept;

  private:
    friend class Runtime;
    explicit WaitSetBuilder(runtime::PoshRuntimeImpl& runtime) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the WaitSetBuilder is not intended to be moved
    runtime::PoshRuntimeImpl& m_runtime;
};
} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/wait_set.inl"

#endif // IOX_POSH_EXPERIMENTAL_WAIT_SET_HPP
