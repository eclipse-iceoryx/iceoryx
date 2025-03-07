// Copyright (c) 2025 by Valour inc. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_CLIENT_INL
#define IOX_POSH_EXPERIMENTAL_CLIENT_INL

#include "iox/posh/experimental/client.hpp"

namespace iox::posh::experimental
{
inline ClientBuilder::ClientBuilder(runtime::PoshRuntime& runtime,
                                    const capro::ServiceDescription& service_description) noexcept
    : m_runtime(runtime)
    , m_service_description(service_description)
{
}

template <typename Req, typename Res>
inline expected<unique_ptr<Client<Req, Res>>, ClientBuilderError> ClientBuilder::create() noexcept
{
    auto* client_port_data = m_runtime.getMiddlewareClient(
        m_service_description,
        {m_response_queue_capacity, "", m_connect_on_create, m_response_queue_full_policy, m_server_too_slow_policy});
    if (client_port_data == nullptr)
    {
        return err(ClientBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<Client<Req, Res>>{
        new (std::nothrow) Client<Req, Res>{iox::popo::ClientPortUser{*client_port_data}}, [&](auto* const client) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete client;
        }});
}

inline expected<unique_ptr<iox::popo::UntypedClient>, ClientBuilderError> ClientBuilder::create() noexcept
{
    auto* client_port_data = m_runtime.getMiddlewareClient(
        m_service_description,
        {m_response_queue_capacity, "", m_connect_on_create, m_response_queue_full_policy, m_server_too_slow_policy});
    if (client_port_data == nullptr)
    {
        return err(ClientBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<iox::popo::UntypedClient>{
        new (std::nothrow) iox::popo::UntypedClient{iox::popo::ClientPortUser{*client_port_data}},
        [&](auto* const client) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete client;
        }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_CLIENT_INL
