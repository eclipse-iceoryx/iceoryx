// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_CLIENT_INL
#define IOX_POSH_POPO_TYPED_CLIENT_INL

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename RequestType, typename ResponseType, typename BaseClient_t>
inline ClientImpl<RequestType, ResponseType, BaseClient_t>::ClientImpl(const capro::ServiceDescription& service,
                                                           const ClientOptions& clientOptions)
    : BaseClient_t(service, clientOptions)
{
}

template <typename RequestType, typename ResponseType, typename BaseClient_t>
template <typename... Args>
inline cxx::expected<Request<RequestType>, AllocationError> ClientImpl<RequestType, ResponseType, BaseClient_t>::loan(Args&&... args) noexcept
{
    return std::move(loanRequest().and_then([&](auto& request) { new (request.get()) RequestType(std::forward<Args>(args)...); }));
}

template <typename RequestType, typename ResponseType, typename BaseClient_t>
inline cxx::expected<Request<RequestType>, AllocationError> ClientImpl<RequestType, ResponseType, BaseClient_t>::loanRequest() noexcept
{
    auto result = port().allocateRequest(sizeof(RequestType), alignof(RequestType), sizeof(RequestHeader), alignof(RequestHeader));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Request<RequestType>>(convertRequestHeaderToRequest(result.value()));
    }
}

template <typename RequestType, typename ResponseType, typename BaseClient_t>
inline void ClientImpl<RequestType, ResponseType, BaseClient_t>::send(Request<RequestType>&& request) noexcept
{
    port().sendRequest(request);
}

template <typename RequestType, typename ResponseType, typename BaseClient_t>
inline cxx::expected<Response<const ResponseType>, ChunkReceiveResult>
ClientImpl<RequestType, ResponseType, BaseClient_t>::take() noexcept
{
    auto result = BaseClient_t::takeResponses();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto userPayloadPtr = static_cast<const ResponseType*>(result.value()->getUserPayload());
    auto requestPtr = cxx::unique_ptr<const ResponseType>(userPayloadPtr, m_requestDeleter);
    return cxx::success<Response<const ResponseType>>(std::move(requestPtr));
}

template <typename RequestType, typename ResponseType, typename BaseClient_t>
inline Request<RequestType>
ClientImpl<RequestType, ResponseType, BaseClient_t>::convertRequestHeaderToRequest(RequestHeader* const header) noexcept
{
    return Request<RequestType>(cxx::unique_ptr<RequestType>(reinterpret_cast<RequestType*>(header->getUserPayload()), m_requestDeleter), *this);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_CLIENT_INL
