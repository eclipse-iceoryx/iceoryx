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
#include "iceoryx_posh/internal/popo/rpc_interface.hpp"
#include "iceoryx_posh/internal/popo/typed_port_api_trait.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
/// @brief The ClientImpl class implements the typed client API
/// @note Not intended for public usage! Use the 'Client' instead!
template <typename Req, typename Res, typename BaseClientT = BaseClient<>>
class ClientImpl : public BaseClientT, private RpcInterface<Request<Req>, ClientSendError>
{
    using RequestTypeAssert = typename TypedPortApiTrait<Req>::Assert;
    using ResponseTypeAssert = typename TypedPortApiTrait<Res>::Assert;

  public:
    /// @brief Constructor for a client
    /// @param[in] service is the ServiceDescription for the new client
    /// @param[in] clientOptions like the queue capacity and queue full policy by a client
    explicit ClientImpl(const capro::ServiceDescription& service, const ClientOptions& clientOptions = {}) noexcept;
    virtual ~ClientImpl() noexcept;

    ClientImpl(const ClientImpl&) = delete;
    ClientImpl(ClientImpl&&) = delete;
    ClientImpl& operator=(const ClientImpl&) = delete;
    ClientImpl& operator=(ClientImpl&&) = delete;

    /// @brief Get a Request from loaned shared memory and construct the data with the given arguments.
    /// @param[in] args Arguments used to construct the data.
    /// @return An instance of the Request that resides in shared memory or an error if unable to allocate memory to
    /// loan.
    /// @details The loaned Request is automatically released when it goes out of scope.
    template <typename... Args>
    expected<Request<Req>, AllocationError> loan(Args&&... args) noexcept;

    /// @brief Sends the given Request and then releases its loan.
    /// @param request to send.
    /// @return Error if sending was not successful
    expected<void, ClientSendError> send(Request<Req>&& request) noexcept override;

    /// @brief Take the Response from the top of the receive queue.
    /// @return Either a Response or a ChunkReceiveResult.
    /// @details The Response takes care of the cleanup. Don't store the raw pointer to the content of the Response, but
    /// always the whole Response.
    expected<Response<const Res>, ChunkReceiveResult> take() noexcept;

  protected:
    using BaseClientT::port;

  private:
    expected<Request<Req>, AllocationError> loanUninitialized() noexcept;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/client_impl.inl"

#endif // IOX_POSH_POPO_CLIENT_IMPL_HPP
