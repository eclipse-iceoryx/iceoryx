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
UntypedClientImpl<BaseClientT>::~UntypedClientImpl() noexcept
{
    BaseClientT::m_trigger.reset();
}

template <typename BaseClientT>
expected<void*, AllocationError> UntypedClientImpl<BaseClientT>::loan(const uint64_t payloadSize,
                                                                      const uint32_t payloadAlignment) noexcept
{
    auto allocationResult = port().allocateRequest(payloadSize, payloadAlignment);
    if (allocationResult.has_error())
    {
        return err(allocationResult.error());
    }

    return ok(mepoo::ChunkHeader::fromUserHeader(allocationResult.value())->userPayload());
}

template <typename BaseClientT>
void UntypedClientImpl<BaseClientT>::releaseRequest(void* const requestPayload) noexcept
{
    auto* chunkHeader = mepoo::ChunkHeader::fromUserPayload(requestPayload);
    if (chunkHeader != nullptr)
    {
        port().releaseRequest(static_cast<RequestHeader*>(chunkHeader->userHeader()));
    }
}

template <typename BaseClientT>
expected<void, ClientSendError> UntypedClientImpl<BaseClientT>::send(void* const requestPayload) noexcept
{
    auto* chunkHeader = mepoo::ChunkHeader::fromUserPayload(requestPayload);
    if (chunkHeader == nullptr)
    {
        return err(ClientSendError::INVALID_REQUEST);
    }

    return port().sendRequest(static_cast<RequestHeader*>(chunkHeader->userHeader()));
}

template <typename BaseClientT>
expected<const void*, ChunkReceiveResult> UntypedClientImpl<BaseClientT>::take() noexcept
{
    auto responseResult = port().getResponse();
    if (responseResult.has_error())
    {
        return err(responseResult.error());
    }

    return ok(mepoo::ChunkHeader::fromUserHeader(responseResult.value())->userPayload());
}

template <typename BaseClientT>
void UntypedClientImpl<BaseClientT>::releaseResponse(const void* const responsePayload) noexcept
{
    const auto* chunkHeader = mepoo::ChunkHeader::fromUserPayload(responsePayload);
    if (chunkHeader != nullptr)
    {
        port().releaseResponse(static_cast<const ResponseHeader*>(chunkHeader->userHeader()));
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_INL
