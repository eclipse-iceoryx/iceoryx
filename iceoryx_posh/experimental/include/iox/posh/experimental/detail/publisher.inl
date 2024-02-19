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

#ifndef IOX_POSH_EXPERIMENTAL_PUBLISHER_INL
#define IOX_POSH_EXPERIMENTAL_PUBLISHER_INL

#include "iox/posh/experimental/publisher.hpp"

namespace iox::posh::experimental
{
inline PublisherBuilder::PublisherBuilder(runtime::PoshRuntimeImpl& runtime,
                                          const capro::ServiceDescription& service_description) noexcept
    : m_runtime(runtime)
    , m_service_description(service_description)
{
}

template <typename T, typename H>
inline expected<popo::Publisher<T, H>, PublisherBuilder::Error> PublisherBuilder::create() noexcept
{
    auto* publisher_port_data = m_runtime.getMiddlewarePublisher(
        m_service_description, {m_history_capacity, "", m_offer_on_create, m_subscriber_too_slow_policy});
    if (publisher_port_data == nullptr)
    {
        return err(Error::OUT_OF_RESOURCES);
    }
    return ok(popo::Publisher<T, H>{std::move(iox::PublisherPortUserType{publisher_port_data})});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_PUBLISHER_INL
