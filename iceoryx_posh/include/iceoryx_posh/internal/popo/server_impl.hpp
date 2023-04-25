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

#ifndef IOX_POSH_POPO_SERVER_IMPL_HPP
#define IOX_POSH_POPO_SERVER_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iceoryx_posh/internal/popo/rpc_interface.hpp"
#include "iceoryx_posh/internal/popo/typed_port_api_trait.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
/// @brief The ServerImpl class implements the typed server API
/// @note Not intended for public usage! Use the 'Server' instead!
template <typename Req, typename Res, typename BaseServerT = BaseServer<>>
class ServerImpl : public BaseServerT, private RpcInterface<Response<Res>, ServerSendError>
{
    using RequestTypeAssert = typename TypedPortApiTrait<Req>::Assert;
    using ResponseTypeAssert = typename TypedPortApiTrait<Res>::Assert;

  public:
    /// @brief Constructor for a sserver
    /// @param[in] service is the ServiceDescription for the new server
    /// @param[in] serverOptions like the queue capacity and queue full policy by a server
    explicit ServerImpl(const capro::ServiceDescription& service, const ServerOptions& serverOptions = {}) noexcept;
    virtual ~ServerImpl() noexcept;

    ServerImpl(const ServerImpl&) = delete;
    ServerImpl(ServerImpl&&) = delete;
    ServerImpl& operator=(const ServerImpl&) = delete;
    ServerImpl& operator=(ServerImpl&&) = delete;

    /// @brief Take the Request from the top of the receive queue.
    /// @return Either a Request or a ServerRequestResult.
    /// @details The Request takes care of the cleanup. Don't store the raw pointer to the content of the Request, but
    /// always the whole Request.
    expected<Request<const Req>, ServerRequestResult> take() noexcept;

    /// @brief Get a Response from loaned shared memory and construct the data with the given arguments.
    /// @param[in] request The request to which the Response belongs to, to determine where to send the response
    /// @param[in] args Arguments used to construct the data.
    /// @return An instance of the Response that resides in shared memory or an error if unable to allocate memory to
    /// loan.
    /// @details The loaned Response is automatically released when it goes out of scope.
    template <typename... Args>
    expected<Response<Res>, AllocationError> loan(const Request<const Req>& request, Args&&... args) noexcept;

    /// @brief Sends the given Response and then releases its loan.
    /// @param response to send.
    /// @return Error if sending was not successful
    expected<void, ServerSendError> send(Response<Res>&& response) noexcept override;

  protected:
    using BaseServerT::port;

  private:
    expected<Response<Res>, AllocationError> loanUninitialized(const Request<const Req>& request) noexcept;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/server_impl.inl"

#endif // IOX_POSH_POPO_SERVER_IMPL_HPP
