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

#ifndef IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
#define IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/popo/base_subscriber.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
class Void
{
};

template <template <typename, typename, typename> class base_subscriber_t = BaseSubscriber>
class UntypedSubscriberImpl
    : public base_subscriber_t<void, UntypedSubscriberImpl<base_subscriber_t>, iox::SubscriberPortUserType>
{
  public:
    using BaseSubscriber =
        base_subscriber_t<void, UntypedSubscriberImpl<base_subscriber_t>, iox::SubscriberPortUserType>;

    UntypedSubscriberImpl(const capro::ServiceDescription& service,
                          const SubscriberOptions& subscriberOptions = SubscriberOptions());
    UntypedSubscriberImpl(const UntypedSubscriberImpl& other) = delete;
    UntypedSubscriberImpl& operator=(const UntypedSubscriberImpl&) = delete;
    UntypedSubscriberImpl(UntypedSubscriberImpl&& rhs) = delete;
    UntypedSubscriberImpl& operator=(UntypedSubscriberImpl&& rhs) = delete;
    virtual ~UntypedSubscriberImpl() = default;

    using BaseSubscriber::getServiceDescription;
    using BaseSubscriber::getSubscriptionState;
    using BaseSubscriber::getUid;
    using BaseSubscriber::hasMissedSamples;
    using BaseSubscriber::hasSamples;
    using BaseSubscriber::invalidateTrigger;
    using BaseSubscriber::releaseQueuedSamples;
    using BaseSubscriber::subscribe;
    using BaseSubscriber::take;
    using BaseSubscriber::unsubscribe;
};

using UntypedSubscriber = UntypedSubscriberImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_subscriber.inl"

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
