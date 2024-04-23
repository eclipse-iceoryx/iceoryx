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

#ifndef IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP
#define IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{

using iox::mepoo::NoUserHeader;
using iox::popo::ConsumerTooSlowPolicy;

using iox::popo::Publisher;
using iox::popo::UntypedPublisher;

enum class PublisherBuilderError : uint8_t
{
    OUT_OF_RESOURCES,
};

/// @brief A builder for the publisher
class PublisherBuilder
{
  public:
    ~PublisherBuilder() = default;

    PublisherBuilder(const PublisherBuilder& other) = delete;
    PublisherBuilder& operator=(const PublisherBuilder&) = delete;
    PublisherBuilder(PublisherBuilder&& rhs) noexcept = delete;
    PublisherBuilder& operator=(PublisherBuilder&& rhs) noexcept = delete;

    /// @brief The size of the history chunk queue
    IOX_BUILDER_PARAMETER(uint64_t, history_capacity, 0)

    /// @brief Indicates whether the publisher should already be offered when creating it
    IOX_BUILDER_PARAMETER(bool, offer_on_create, true)

    /// @brief Indicates whether the publisher should block when the subscriber queue is full
    IOX_BUILDER_PARAMETER(ConsumerTooSlowPolicy, subscriber_too_slow_policy, ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)

  public:
    /// @brief Creates a typed publisher instance for the publish-subscribe messaging pattern
    /// @tparam T user payload type
    /// @tparam H user header type
    template <typename T, typename H = NoUserHeader>
    expected<unique_ptr<Publisher<T, H>>, PublisherBuilderError> create() noexcept;

    /// @brief Creates an untyped publisher instance for the publish-subscribe messaging pattern
    expected<unique_ptr<UntypedPublisher>, PublisherBuilderError> create() noexcept;

  private:
    friend class Node;
    explicit PublisherBuilder(runtime::PoshRuntime& runtime,
                              const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the PublisherBuilder is not intended to be moved
    runtime::PoshRuntime& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/publisher.inl"

#endif // IOX_POSH_EXPERIMENTAL_PUBLISHER_HPP
