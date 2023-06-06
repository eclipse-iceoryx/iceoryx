// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_UNTYPED_SUBSCRIBER_IMPL_INL
#define IOX_POSH_POPO_UNTYPED_SUBSCRIBER_IMPL_INL

#include "iceoryx_posh/internal/popo/untyped_subscriber_impl.hpp"

namespace iox
{
namespace popo
{
template <typename BaseSubscriberType>
inline UntypedSubscriberImpl<BaseSubscriberType>::UntypedSubscriberImpl(const capro::ServiceDescription& service,
                                                                        const SubscriberOptions& subscriberOptions)
    : BaseSubscriber(service, subscriberOptions)
{
}

template <typename BaseSubscriberType>
inline expected<const void*, ChunkReceiveResult> UntypedSubscriberImpl<BaseSubscriberType>::take() noexcept
{
    auto result = BaseSubscriber::takeChunk();
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok(result.value()->userPayload());
}

template <typename BaseSubscriberType>
inline void UntypedSubscriberImpl<BaseSubscriberType>::release(const void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().releaseChunk(chunkHeader);
}

template <typename BaseSubscriberType>
inline UntypedSubscriberImpl<BaseSubscriberType>::~UntypedSubscriberImpl() noexcept
{
    BaseSubscriberType::m_trigger.reset();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_IMPL_INL
