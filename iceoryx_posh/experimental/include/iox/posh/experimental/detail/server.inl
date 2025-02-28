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

#ifndef IOX_POSH_EXPERIMENTAL_SERVER_INL
#define IOX_POSH_EXPERIMENTAL_SERVER_INL

#include "iox/posh/experimental/server.hpp"

namespace iox::posh::experimental
{
inline ServerBuilder::ServerBuilder(runtime::PoshRuntime& runtime,
                                    const capro::ServiceDescription& service_description) noexcept
    : m_runtime(runtime)
    , m_service_description(service_description)
{
}


template <typename Req, typename Res>
inline expected<unique_ptr<Server<Req, Res>>, ServerBuilderError> ServerBuilder::create() noexcept
{
    auto* server_port_data = m_runtime.getMiddlewareServer(
        m_service_description,
        {m_request_queue_capacity, "", m_offer_on_create, m_request_queue_full_policy, m_client_too_slow_policy});
    if (server_port_data == nullptr)
    {
        return err(ServerBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<Server<Req, Res>>{
        new (std::nothrow) Server<Req, Res>{iox::popo::ServerPortUser{*server_port_data}}, [&](auto* const server) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
            delete server;
        }});
}

inline expected<unique_ptr<UntypedServer>, ServerBuilderError> ServerBuilder::create() noexcept
{
    auto* server_port_data = m_runtime.getMiddlewareServer(
        m_service_description,
        {m_request_queue_capacity, "", m_offer_on_create, m_request_queue_full_policy, m_client_too_slow_policy});
    if (server_port_data == nullptr)
    {
        return err(ServerBuilderError::OUT_OF_RESOURCES);
    }
    return ok(unique_ptr<UntypedServer>{new (std::nothrow) UntypedServer{iox::popo::ServerPortUser{*server_port_data}},
                                        [&](auto* const server) {
                                            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) raw pointer is required by the unique_ptr API
                                            delete server;
                                        }});
}

} // namespace iox::posh::experimental

#endif // IOX_POSH_EXPERIMENTAL_SERVER_INL
