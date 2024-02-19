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

#ifndef IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP
#define IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"

namespace iox::posh::experimental
{

using iox::mepoo::NoUserHeader;
using iox::popo::QueueFullPolicy;

class Runtime;

class SubscriberBuilder
{
  public:
    ~SubscriberBuilder() = default;

    SubscriberBuilder(const SubscriberBuilder& other) = delete;
    SubscriberBuilder& operator=(const SubscriberBuilder&) = delete;
    SubscriberBuilder(SubscriberBuilder&& rhs) noexcept = delete;
    SubscriberBuilder& operator=(SubscriberBuilder&& rhs) noexcept = delete;

    enum class Error
    {
        OUT_OF_RESOURCES,
    };

    IOX_BUILDER_PARAMETER(uint64_t, queue_capacity, popo::SubscriberChunkQueueData_t::MAX_CAPACITY)
    IOX_BUILDER_PARAMETER(uint64_t, history_request, 0)
    IOX_BUILDER_PARAMETER(bool, requires_publisher_history_support, false)
    IOX_BUILDER_PARAMETER(bool, subscribe_on_create, true)
    IOX_BUILDER_PARAMETER(QueueFullPolicy, queue_full_policy, QueueFullPolicy::DISCARD_OLDEST_DATA)

  public:
    template <typename T, typename H = NoUserHeader>
    expected<popo::Subscriber<T, H>, Error> create() noexcept;

  private:
    friend class Runtime;
    explicit SubscriberBuilder(runtime::PoshRuntimeImpl& runtime,
                               const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the SubscriberBuilder is not intended to be moved
    runtime::PoshRuntimeImpl& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/subscriber.inl"

#endif // IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP
