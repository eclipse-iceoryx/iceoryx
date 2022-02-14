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

#ifndef IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_INL
#define IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_INL

#include "iceoryx_posh/internal/popo/untyped_client_impl.hpp"

namespace iox
{
namespace popo
{
template <typename BaseClientT>
UntypedClientImpl<BaseClientT>::UntypedClientImpl(const capro::ServiceDescription& service,
                                                  const ClientOptions& clientOptions) noexcept
    : BaseClientT(service, clientOptions)
{
}

template <typename BaseClientT>
cxx::expected<void*, AllocationError> UntypedClientImpl<BaseClientT>::loan(const uint32_t payloadSize,
                                                                           const uint32_t payloadAlignment) noexcept
{
    auto allocationResult = port().allocateRequest(payloadSize, payloadAlignment);
    if (allocationResult.has_error())
    {
        return cxx::error<AllocationError>(allocationResult.get_error());
    }

    return cxx::success<void*>(mepoo::ChunkHeader::fromUserHeader(allocationResult.value())->userPayload());
}

template <typename BaseClientT>
void UntypedClientImpl<BaseClientT>::freeRequest(void* const requestPayload) noexcept
{
    port().freeRequest(static_cast<RequestHeader*>(mepoo::ChunkHeader::fromUserPayload(requestPayload)->userHeader()));
}

template <typename BaseClientT>
void UntypedClientImpl<BaseClientT>::send(void* const requestPayload) noexcept
{
    port().sendRequest(static_cast<RequestHeader*>(mepoo::ChunkHeader::fromUserPayload(requestPayload)->userHeader()));
}

template <typename BaseClientT>
cxx::expected<const void*, ChunkReceiveResult> UntypedClientImpl<BaseClientT>::take() noexcept
{
    auto responseResult = port().getResponse();
    if (responseResult.has_error())
    {
        return cxx::error<ChunkReceiveResult>(responseResult.get_error());
    }

    return cxx::success<const void*>(mepoo::ChunkHeader::fromUserHeader(responseResult.value())->userPayload());
}

template <typename BaseClientT>
void UntypedClientImpl<BaseClientT>::releaseResponse(const void* const responsePayload) noexcept
{
    port().releaseResponse(
        static_cast<const ResponseHeader*>(mepoo::ChunkHeader::fromUserPayload(responsePayload)->userHeader()));
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_INL
