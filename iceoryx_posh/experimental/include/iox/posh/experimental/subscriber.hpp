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

#ifndef IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP
#define IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{

using iox::mepoo::NoUserHeader;
using iox::popo::QueueFullPolicy;
using iox::popo::Subscriber;
using iox::popo::UntypedSubscriber;

enum class SubscriberBuilderError : uint8_t
{
    OUT_OF_RESOURCES,
};

/// @brief A builder for the subscriber
class SubscriberBuilder
{
  public:
    ~SubscriberBuilder() = default;

    SubscriberBuilder(const SubscriberBuilder& other) = delete;
    SubscriberBuilder& operator=(const SubscriberBuilder&) = delete;
    SubscriberBuilder(SubscriberBuilder&& rhs) noexcept = delete;
    SubscriberBuilder& operator=(SubscriberBuilder&& rhs) noexcept = delete;

    /// @brief The size of the receiver queue where chunks are stored before they are passed to the user
    /// @attention Depending on the underlying queue there can be a different overflow behavior
    IOX_BUILDER_PARAMETER(uint64_t, queue_capacity, popo::SubscriberChunkQueueData_t::MAX_CAPACITY)

    /// @brief The max number of chunks received after subscription if chunks are available
    IOX_BUILDER_PARAMETER(uint64_t, history_request, 0)

    /// @brief Indicates whether to enforce history support of the publisher,
    ///        i.e. require historyCapacity > 0 to be eligible to be connected
    IOX_BUILDER_PARAMETER(bool, requires_publisher_history_support, false)

    /// @brief Indicates whether the subscriber shall try to subscribe when creating it
    IOX_BUILDER_PARAMETER(bool, subscribe_on_create, true)

    /// @brief Indicates whether the publisher should block when the subscriber queue is full
    IOX_BUILDER_PARAMETER(QueueFullPolicy, queue_full_policy, QueueFullPolicy::DISCARD_OLDEST_DATA)

  public:
    /// @brief Creates a typed subscriber instance for the publish-subscribe messaging pattern
    /// @tparam T user payload type
    /// @tparam H user header type
    template <typename T, typename H = NoUserHeader>
    expected<unique_ptr<Subscriber<T, H>>, SubscriberBuilderError> create() noexcept;

    /// @brief Creates an untyped subscriber instance for the publish-subscribe messaging pattern
    expected<unique_ptr<UntypedSubscriber>, SubscriberBuilderError> create() noexcept;

  private:
    friend class Node;
    explicit SubscriberBuilder(runtime::PoshRuntime& runtime,
                               const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the SubscriberBuilder is not intended to be moved
    runtime::PoshRuntime& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/subscriber.inl"

#endif // IOX_POSH_EXPERIMENTAL_SUBSCRIBER_HPP
