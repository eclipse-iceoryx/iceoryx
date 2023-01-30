// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POPO_SUBSCRIBER_PORT_MULTI_PRODUCER_HPP_
#define IOX_POPO_SUBSCRIBER_PORT_MULTI_PRODUCER_HPP_

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_roudi.hpp"
#include "iox/not_null.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace popo
{
/// @brief The SubscriberPortMultiProducer is the implementation of the SubscriberPortRouDi for a setup where
/// subscribers can have multiple matching publishers. I.e. a n:m pub/sub deployment. The handling of CaPro
/// messages is different for 1:m and n:m deployment
class SubscriberPortMultiProducer : public SubscriberPortRouDi
{
  public:
    using MemberType_t = SubscriberPortData;

    explicit SubscriberPortMultiProducer(not_null<MemberType_t* const> publisherPortDataPtr) noexcept;

    SubscriberPortMultiProducer(const SubscriberPortMultiProducer& other) = delete;
    SubscriberPortMultiProducer& operator=(const SubscriberPortMultiProducer&) = delete;
    SubscriberPortMultiProducer(SubscriberPortMultiProducer&& rhs) noexcept = default;
    SubscriberPortMultiProducer& operator=(SubscriberPortMultiProducer&& rhs) noexcept = default;
    ~SubscriberPortMultiProducer() = default;

    /// @brief get an optional CaPro message that changes the subscription state of the subscriber
    /// @return CaPro message with new subscription requet, empty optional if no state change
    optional<capro::CaproMessage> tryGetCaProMessage() noexcept override;

    /// @brief dispatch a CaPro message to the subscriber for processing
    /// @param[in] caProMessage to process
    /// @return CaPro message with an immediate response the provided CaPro message, empty optional if no response
    optional<capro::CaproMessage>
    dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept override;
};

} // namespace popo
} // namespace iox

#endif
