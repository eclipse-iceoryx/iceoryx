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

#ifndef IOX_POSH_EXPERIMENTAL_SUBSCRIBER_INL
#define IOX_POSH_EXPERIMENTAL_SUBSCRIBER_INL

#include "iox/posh/experimental/subscriber.hpp"

namespace iox::posh::experimental
{
inline SubscriberBuilder::SubscriberBuilder(runtime::PoshRuntime& runtime,
                                            const capro::ServiceDescription& service_description) noexcept
    : m_runtime(runtime)
    , m_service_description(service_description)
{
}

template <typename T, typename H>
inline expected<unique_ptr<Subscriber<T, H>>, SubscriberBuilderError> SubscriberBuilder::create() noexcept
{
    auto* subscriber_port_data = m_runtime.getMiddlewareSubscriber(m_service_description,
                                                                   {m_queue_capacity,
                                                                    m_history_request,
                                                                    "",
                                                                    m_subscribe_on_create,
                                                                    m_queue_full_policy,
                                                                    m_requires_publisher_history_support});
    if (subscriber_port_data == nullptr)
    {
        return err(SubscriberBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<Subscriber<T, H>>{
        new (std::nothrow) Subscriber<T, H>{iox::SubscriberPortUserType{subscriber_port_data}}, [&](auto* const sub) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete sub;
        }});
}

inline expected<unique_ptr<UntypedSubscriber>, SubscriberBuilderError> SubscriberBuilder::create() noexcept
{
    auto* subscriber_port_data = m_runtime.getMiddlewareSubscriber(m_service_description,
                                                                   {m_queue_capacity,
                                                                    m_history_request,
                                                                    "",
                                                                    m_subscribe_on_create,
                                                                    m_queue_full_policy,
                                                                    m_requires_publisher_history_support});
    if (subscriber_port_data == nullptr)
    {
        return err(SubscriberBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<UntypedSubscriber>{
        new (std::nothrow) UntypedSubscriber{iox::SubscriberPortUserType{subscriber_port_data}}, [&](auto* const sub) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete sub;
        }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_SUBSCRIBER_INL
