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

#ifndef IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_HPP
#define IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_HPP

#include "iceoryx_posh/internal/popo/base_subscriber.hpp"
#include "iceoryx_posh/internal/popo/typed_port_api_trait.hpp"

namespace iox
{
namespace popo
{
/// @brief The SubscriberImpl class implements the typed subscriber API
/// @note Not intended for public usage! Use the 'Subscriber' instead!
template <typename T, typename H = iox::mepoo::NoUserHeader, typename BaseSubscriberType = BaseSubscriber<>>
class SubscriberImpl : public BaseSubscriberType
{
    using SelfType = SubscriberImpl<T, H, BaseSubscriberType>;

    using DataTypeAssert = typename TypedPortApiTrait<T>::Assert;
    using HeaderTypeAssert = typename TypedPortApiTrait<H>::Assert;

  public:
    explicit SubscriberImpl(const capro::ServiceDescription& service,
                            const SubscriberOptions& subscriberOptions = SubscriberOptions()) noexcept;
    SubscriberImpl(const SubscriberImpl& other) = delete;
    SubscriberImpl& operator=(const SubscriberImpl&) = delete;
    SubscriberImpl(SubscriberImpl&& rhs) = delete;
    SubscriberImpl& operator=(SubscriberImpl&& rhs) = delete;
    virtual ~SubscriberImpl() noexcept;

    ///
    /// @brief Take the samples from the top of the receive queue.
    /// @return Either a sample or a ChunkReceiveResult.
    /// @details The sample takes care of the cleanup. Don't store the raw pointer to the content of the sample, but
    /// always the whole sample.
    ///
    expected<Sample<const T, const H>, ChunkReceiveResult> take() noexcept;

    using PortType = typename BaseSubscriberType::PortType;

  protected:
    using BaseSubscriberType::port;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/subscriber_impl.inl"

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_IMPL_HPP
