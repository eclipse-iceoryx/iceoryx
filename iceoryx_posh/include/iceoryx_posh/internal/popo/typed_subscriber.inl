// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
template <typename T, template <typename, typename, typename> class base_subscriber_t>
inline TypedSubscriber<T, base_subscriber_t>::TypedSubscriber(const capro::ServiceDescription& service,
                                                              const SubscriberOptions& subscriberOptions)
    : BaseSubscriber(service, subscriberOptions)
{
}

template <typename T, template <typename, typename, typename> class base_subscriber_t>
inline cxx::expected<Sample<const T>, ChunkReceiveResult> TypedSubscriber<T, base_subscriber_t>::take_1_0() noexcept
{
    auto result = BaseSubscriber::takeChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto payloadPtr = static_cast<T*>(result.value()->payload());
    auto samplePtr = cxx::unique_ptr<T>(static_cast<T*>(payloadPtr), BaseSubscriber::m_sampleDeleter);
    return cxx::success<Sample<const T>>(std::move(samplePtr));
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_INL
