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

#ifndef IOX_POSH_POPO_SERVER_IMPL_INL
#define IOX_POSH_POPO_SERVER_IMPL_INL

#include "iceoryx_posh/popo/impl/server_impl.hpp"

namespace iox
{
namespace popo
{
template <typename Req, typename Res, typename BaseServerT>
inline ServerImpl<Req, Res, BaseServerT>::ServerImpl(const capro::ServiceDescription& service,
                                                     const ServerOptions& serverOptions) noexcept
    : BaseServerT(service, serverOptions)
{
}

template <typename Req, typename Res, typename BaseServerT>
inline ServerImpl<Req, Res, BaseServerT>::~ServerImpl() noexcept
{
    BaseServerT::m_trigger.reset();
}

template <typename Req, typename Res, typename BaseServerT>
cxx::expected<Request<const Req>, ServerRequestResult> ServerImpl<Req, Res, BaseServerT>::take() noexcept
{
    auto result = port().getRequest();
    if (result.has_error())
    {
        return cxx::error<ServerRequestResult>(result.get_error());
    }
    auto requestHeader = result.value();
    auto payload = mepoo::ChunkHeader::fromUserHeader(requestHeader)->userPayload();
    auto request = cxx::unique_ptr<const Req>(static_cast<const Req*>(payload), m_requestDeleter);
    return cxx::success<Request<const Req>>(Request<const Req>{std::move(request)});
}

template <typename Req, typename Res, typename BaseServerT>
cxx::expected<Response<Res>, AllocationError>
ServerImpl<Req, Res, BaseServerT>::loanUninitialized(const Request<const Req>& request) noexcept
{
    const auto* requestHeader = &request.getRequestHeader();
    auto result = port().allocateResponse(requestHeader, sizeof(Res), alignof(Res));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    auto responseHeader = result.value();
    auto payload = mepoo::ChunkHeader::fromUserHeader(responseHeader)->userPayload();
    auto response = cxx::unique_ptr<Res>(reinterpret_cast<Res*>(payload), m_responseDeleter);
    return cxx::success<Response<Res>>(Response<Res>{std::move(response), *this});
}

template <typename Req, typename Res, typename BaseServerT>
template <typename... Args>
cxx::expected<Response<Res>, AllocationError> ServerImpl<Req, Res, BaseServerT>::loan(const Request<const Req>& request,
                                                                                      Args&&... args) noexcept
{
    return std::move(loanUninitialized(request).and_then(
        [&](auto& response) { new (response.get()) Res(std::forward<Args>(args)...); }));
}

template <typename Req, typename Res, typename BaseServerT>
cxx::expected<ServerSendError> ServerImpl<Req, Res, BaseServerT>::send(Response<Res>&& response) noexcept
{
    // take the ownership of the chunk from the Response to transfer it to `sendResponse`
    auto payload = response.release();
    auto* responseHeader = static_cast<ResponseHeader*>(mepoo::ChunkHeader::fromUserPayload(payload)->userHeader());
    return port().sendResponse(responseHeader);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SERVER_IMPL_INL
