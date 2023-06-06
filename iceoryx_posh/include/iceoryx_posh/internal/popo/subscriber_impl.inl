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

#ifndef IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_INL
#define IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_INL

#include "iceoryx_posh/internal/popo/subscriber_impl.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename H, typename BaseSubscriberType>
inline SubscriberImpl<T, H, BaseSubscriberType>::SubscriberImpl(const capro::ServiceDescription& service,
                                                                const SubscriberOptions& subscriberOptions) noexcept
    : BaseSubscriberType(service, subscriberOptions)
{
}

template <typename T, typename H, typename BaseSubscriberType>
inline expected<Sample<const T, const H>, ChunkReceiveResult> SubscriberImpl<T, H, BaseSubscriberType>::take() noexcept
{
    auto result = BaseSubscriberType::takeChunk();
    if (result.has_error())
    {
        return err(result.error());
    }
    auto userPayloadPtr = static_cast<const T*>(result.value()->userPayload());
    auto samplePtr = iox::unique_ptr<const T>(userPayloadPtr, [this](const T* userPayload) {
        auto* chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(userPayload);
        this->port().releaseChunk(chunkHeader);
    });
    return ok<Sample<const T, const H>>(std::move(samplePtr));
}

template <typename T, typename H, typename BaseSubscriberType>
inline SubscriberImpl<T, H, BaseSubscriberType>::~SubscriberImpl() noexcept
{
    BaseSubscriberType::m_trigger.reset();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_INL
