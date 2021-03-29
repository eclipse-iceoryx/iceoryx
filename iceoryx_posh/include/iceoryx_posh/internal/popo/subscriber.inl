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

#ifndef IOX_POSH_POPO_TYPED_SUBSCRIBER_INL
#define IOX_POSH_POPO_TYPED_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
template <typename T, typename H, typename base_subscriber_t>
inline Subscriber<T, H, base_subscriber_t>::Subscriber(const capro::ServiceDescription& service,
                                                       const SubscriberOptions& subscriberOptions)
    : base_subscriber_t(service, subscriberOptions)
{
}

template <typename T, typename H, typename base_subscriber_t>
inline cxx::expected<Sample<const T, const H>, ChunkReceiveResult> Subscriber<T, H, base_subscriber_t>::take() noexcept
{
    auto result = base_subscriber_t::takeChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto payloadPtr = static_cast<T*>(result.value()->payload());
    auto samplePtr = cxx::unique_ptr<const T>(static_cast<const T*>(payloadPtr), m_sampleDeleter);
    return cxx::success<Sample<const T, const H>>(std::move(samplePtr));
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_INL
