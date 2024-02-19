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

#ifndef IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP
#define IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"

namespace iox::posh::experimental
{

using iox::mepoo::NoUserHeader;
using iox::popo::ConsumerTooSlowPolicy;

struct Untyped
{
};

class Runtime;

class PublisherBuilder
{
  public:
    ~PublisherBuilder() = default;

    PublisherBuilder(const PublisherBuilder& other) = delete;
    PublisherBuilder& operator=(const PublisherBuilder&) = delete;
    PublisherBuilder(PublisherBuilder&& rhs) noexcept = delete;
    PublisherBuilder& operator=(PublisherBuilder&& rhs) noexcept = delete;

    enum class Error
    {
        OUT_OF_RESOURCES,
    };

    IOX_BUILDER_PARAMETER(uint64_t, history_capacity, 0)
    IOX_BUILDER_PARAMETER(bool, offer_on_create, true)
    IOX_BUILDER_PARAMETER(ConsumerTooSlowPolicy, subscriber_too_slow_policy, ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)

  public:
    template <typename T, typename H = NoUserHeader>
    expected<popo::Publisher<T, H>, Error> create() noexcept;

  private:
    friend class Runtime;
    explicit PublisherBuilder(runtime::PoshRuntimeImpl& runtime,
                              const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the PublisherBuilder is not intended to be moved
    runtime::PoshRuntimeImpl& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/publisher.inl"

#endif // IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP
