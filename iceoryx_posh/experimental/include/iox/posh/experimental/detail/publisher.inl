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

#ifndef IOX_POSH_EXPERIMENTAL_PUBLISHER_INL
#define IOX_POSH_EXPERIMENTAL_PUBLISHER_INL

#include "iox/posh/experimental/publisher.hpp"

namespace iox::posh::experimental
{
inline PublisherBuilder::PublisherBuilder(runtime::PoshRuntime& runtime,
                                          const capro::ServiceDescription& service_description) noexcept
    : m_runtime(runtime)
    , m_service_description(service_description)
{
}

template <typename T, typename H>
inline expected<unique_ptr<Publisher<T, H>>, PublisherBuilderError> PublisherBuilder::create() noexcept
{
    auto* publisher_port_data = m_runtime.getMiddlewarePublisher(
        m_service_description, {m_history_capacity, "", m_offer_on_create, m_subscriber_too_slow_policy});
    if (publisher_port_data == nullptr)
    {
        return err(PublisherBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<Publisher<T, H>>{
        new (std::nothrow) Publisher<T, H>{iox::PublisherPortUserType{publisher_port_data}}, [&](auto* const pub) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete pub;
        }});
}

inline expected<unique_ptr<UntypedPublisher>, PublisherBuilderError> PublisherBuilder::create() noexcept
{
    auto* publisher_port_data = m_runtime.getMiddlewarePublisher(
        m_service_description, {m_history_capacity, "", m_offer_on_create, m_subscriber_too_slow_policy});
    if (publisher_port_data == nullptr)
    {
        return err(PublisherBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<UntypedPublisher>{
        new (std::nothrow) UntypedPublisher{iox::PublisherPortUserType{publisher_port_data}}, [&](auto* const pub) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete pub;
        }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_PUBLISHER_INL
