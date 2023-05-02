// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_CLIENT_IMPL_INL
#define IOX_POSH_POPO_CLIENT_IMPL_INL

#include "iceoryx_posh/internal/popo/client_impl.hpp"

namespace iox
{
namespace popo
{
template <typename Req, typename Res, typename BaseClientT>
ClientImpl<Req, Res, BaseClientT>::ClientImpl(const capro::ServiceDescription& service,
                                              const ClientOptions& clientOptions) noexcept
    : BaseClientT(service, clientOptions)
{
}

template <typename Req, typename Res, typename BaseClientT>
ClientImpl<Req, Res, BaseClientT>::~ClientImpl() noexcept
{
    BaseClientT::m_trigger.reset();
}

template <typename Req, typename Res, typename BaseClientT>
expected<Request<Req>, AllocationError> ClientImpl<Req, Res, BaseClientT>::loanUninitialized() noexcept
{
    auto result = port().allocateRequest(sizeof(Req), alignof(Req));
    if (result.has_error())
    {
        return err(result.error());
    }
    auto requestHeader = result.value();
    auto payload = mepoo::ChunkHeader::fromUserHeader(requestHeader)->userPayload();
    auto request = iox::unique_ptr<Req>(static_cast<Req*>(payload), [this](Req* payload) {
        auto* requestHeader = iox::popo::RequestHeader::fromPayload(payload);
        this->port().releaseRequest(requestHeader);
    });
    return ok(Request<Req>{std::move(request), *this});
}

template <typename Req, typename Res, typename BaseClientT>
template <typename... Args>
expected<Request<Req>, AllocationError> ClientImpl<Req, Res, BaseClientT>::loan(Args&&... args) noexcept
{
    return std::move(
        loanUninitialized().and_then([&](auto& request) { new (request.get()) Req(std::forward<Args>(args)...); }));
}

template <typename Req, typename Res, typename BaseClientT>
expected<void, ClientSendError> ClientImpl<Req, Res, BaseClientT>::send(Request<Req>&& request) noexcept
{
    // take the ownership of the chunk from the Request to transfer it to 'sendRequest'
    auto payload = request.release();
    auto* requestHeader = static_cast<RequestHeader*>(mepoo::ChunkHeader::fromUserPayload(payload)->userHeader());
    return port().sendRequest(requestHeader);
}

template <typename Req, typename Res, typename BaseClientT>
expected<Response<const Res>, ChunkReceiveResult> ClientImpl<Req, Res, BaseClientT>::take() noexcept
{
    auto result = port().getResponse();
    if (result.has_error())
    {
        return err(result.error());
    }
    auto responseHeader = result.value();
    auto payload = mepoo::ChunkHeader::fromUserHeader(responseHeader)->userPayload();
    auto response = iox::unique_ptr<const Res>(static_cast<const Res*>(payload), [this](const Res* payload) {
        auto* responseHeader = iox::popo::ResponseHeader::fromPayload(payload);
        this->port().releaseResponse(responseHeader);
    });
    return ok(Response<const Res>{std::move(response)});
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_CLIENT_IMPL_INL
