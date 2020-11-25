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

#ifndef IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
#define IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/modern_api/base_subscriber.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
template <typename base_subscriber_t = BaseSubscriber<void>>
class UntypedSubscriberImpl : public base_subscriber_t
{
  public:
    UntypedSubscriberImpl(const capro::ServiceDescription& service);
    UntypedSubscriberImpl(const UntypedSubscriberImpl& other) = delete;
    UntypedSubscriberImpl& operator=(const UntypedSubscriberImpl&) = delete;
    UntypedSubscriberImpl(UntypedSubscriberImpl&& rhs) = delete;
    UntypedSubscriberImpl& operator=(UntypedSubscriberImpl&& rhs) = delete;
    virtual ~UntypedSubscriberImpl() = default;

    using base_subscriber_t::getServiceDescription;
    using base_subscriber_t::getSubscriptionState;
    using base_subscriber_t::getUid;
    using base_subscriber_t::hasMissedSamples;
    using base_subscriber_t::hasNewSamples;
    using base_subscriber_t::hasTriggered;
    using base_subscriber_t::releaseQueuedSamples;
    using base_subscriber_t::setConditionVariable;
    using base_subscriber_t::subscribe;
    using base_subscriber_t::take;
    using base_subscriber_t::unsetConditionVariable;
    using base_subscriber_t::unsubscribe;
};

using UntypedSubscriber = UntypedSubscriberImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/modern_api/untyped_subscriber.inl"

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
