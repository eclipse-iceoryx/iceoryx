// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_CLIENT_IMPL_HPP
#define IOX_POSH_POPO_CLIENT_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iceoryx_posh/internal/popo/request_deleter.hpp"
#include "iceoryx_posh/internal/popo/response_deleter.hpp"
#include "iceoryx_posh/internal/popo/rpc_interface.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename Req, typename Res, typename BaseClientT = BaseClient<>>
class ClientImpl : public BaseClientT, public RpcInterface<Request<Req>>
{
  public:
    explicit ClientImpl(const capro::ServiceDescription& service, const ClientOptions& clientOptions = {}) noexcept;

    template <typename... Args>
    cxx::expected<Request<Req>, AllocationError> loan(Args&&... args) noexcept;

    void send(Request<Req>&& request) noexcept override;

    cxx::expected<Response<const Res>, ChunkReceiveResult> take() noexcept;

  private:
    using BaseClientT::port;

    cxx::expected<Request<Req>, AllocationError> loanUninitialized() noexcept;

    using RequestSampleDeleter = RequestDeleter<typename BaseClientT::PortType>;
    RequestSampleDeleter m_requestDeleter{port()};
    using ResponseSampleDeleter = ResponseDeleter<typename BaseClientT::PortType>;
    ResponseSampleDeleter m_responseDeleter{port()};
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/client_impl.inl"

#endif // IOX_POSH_POPO_CLIENT_IMPL_HPP
