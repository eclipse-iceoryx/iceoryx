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
    using BaseSubscriber::invalidateTrigger;
    using BaseSubscriber::subscribe;
    using BaseSubscriber::unsubscribe;

    ///
    /// @brief take Take the chunk from the top of the receive queue.
    /// @return The payload pointer of the chunk taken.
    /// @details No automatic cleaunp of the associated chunk is performed.
    ///
    cxx::expected<const void*, ChunkReceiveResult> take_1_0() noexcept; // iox-#408 rename

    ///
    /// @brief hasChunks Check if chunks are available.
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

    ///
    /// @brief releaseChunk Releases the chunk provided by the payload pointer.
    /// @param payload pointer to the payload of the chunk to be released
    /// @details The chunk must have been previosly provided by take_1_0 and
    ///          not have been already released.
    ///
    void releaseChunk(const void* payload) noexcept;
};

using UntypedSubscriber = UntypedSubscriberImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_subscriber.inl"

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
