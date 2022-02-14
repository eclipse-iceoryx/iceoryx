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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_IMPL_INL
#define IOX_POSH_POPO_UNTYPED_SERVER_IMPL_INL

#include "iceoryx_posh/internal/popo/untyped_server_impl.hpp"

namespace iox
{
namespace popo
{
template <typename BaseServerT>
UntypedServerImpl<BaseServerT>::UntypedServerImpl(const capro::ServiceDescription& service,
                                                  const ServerOptions& serverOptions) noexcept
    : BaseServerT(service, serverOptions)
{
}

template <typename BaseServerT>
cxx::expected<const void*, ServerRequestResult> UntypedServerImpl<BaseServerT>::take() noexcept
{
    auto requestResult = port().getRequest();
    if (requestResult.has_error())
    {
        return cxx::error<ServerRequestResult>(requestResult.get_error());
    }

    return cxx::success<const void*>(mepoo::ChunkHeader::fromUserHeader(requestResult.value())->userPayload());
}

template <typename BaseServerT>
void UntypedServerImpl<BaseServerT>::releaseRequest(const void* const requestPayload) noexcept
{
    port().releaseRequest(
        static_cast<const RequestHeader*>(mepoo::ChunkHeader::fromUserPayload(requestPayload)->userHeader()));
}

template <typename BaseServerT>
cxx::expected<void*, AllocationError> UntypedServerImpl<BaseServerT>::loan(const RequestHeader* requestHeader,
                                                                           const uint32_t payloadSize,
                                                                           const uint32_t payloadAlignment) noexcept
{
    auto allocationResult = port().allocateResponse(requestHeader, payloadSize, payloadAlignment);
    if (allocationResult.has_error())
    {
        return cxx::error<AllocationError>(allocationResult.get_error());
    }

    return cxx::success<void*>(mepoo::ChunkHeader::fromUserHeader(allocationResult.value())->userPayload());
}

template <typename BaseServerT>
void UntypedServerImpl<BaseServerT>::send(void* const responsePayload) noexcept
{
    port().sendResponse(
        static_cast<ResponseHeader*>(mepoo::ChunkHeader::fromUserPayload(responsePayload)->userHeader()));
}

template <typename BaseServerT>
void UntypedServerImpl<BaseServerT>::releaseResponse(void* const responsePayload) noexcept
{
    port().releaseResponse(
        static_cast<ResponseHeader*>(mepoo::ChunkHeader::fromUserPayload(responsePayload)->userHeader()));
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_SERVER_IMPL_INL
