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
    using BaseSubscriber::hasMissedSamples; // iox-#408 remove
    using BaseSubscriber::hasSamples;       // iox-#408 remove
    using BaseSubscriber::invalidateTrigger;
    using BaseSubscriber::releaseChunk;
    using BaseSubscriber::releaseQueuedSamples; // iox-#408 remove
    using BaseSubscriber::subscribe;
    using BaseSubscriber::take; // iox-#408 replace
    using BaseSubscriber::unsubscribe;

    // the 1_0 suffix is only used temporarily to not cause regressions in all examples and tests and keep the changes
    // as small as possible, it will replace the function without suffix in a follow-up pull request (which changes
    // all examples)

    cxx::expected<const void*, ChunkReceiveResult> take_1_0() noexcept;

    // the untyped API is supposed to deal with chunks, hence the renaming iox #408 remove comment
    // calling it chunks looks inapproriate in the function names (use data instead of chunks?)...

    ///
    /// @brief hasData Check if chunks are available.
    /// @return True if a new chunk is available.
    ///
    bool hasChunks() const noexcept;

    ///
    /// @brief hasMissedChunks Check if chunks have been missed since the last hasMissedData() call.
    /// @return True if chunks have been missed.
    /// @details Chunks may be missed due to overflowing receive queue.
    ///
    bool hasMissedChunks() noexcept;

    ///
    /// @brief releaseQueuedChunks Releases any unread queued data chunk.
    ///
    void releaseQueuedChunks() noexcept;
};

using UntypedSubscriber = UntypedSubscriberImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_subscriber.inl"

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
