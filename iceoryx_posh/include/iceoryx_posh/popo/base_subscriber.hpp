// Copyright (c) 2020, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_SUBSCRIBER_HPP
#define IOX_POSH_POPO_BASE_SUBSCRIBER_HPP

#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

enum class SubscriberEvent
{
    HAS_SAMPLES
};

/// @brief base class for all types of subscriber
/// @param[in] T the sample type
/// @param[in] Subscriber type of the child which is inheriting from BaseSubscriber. This type is required for the
/// TriggerCallback since a trigger provides a pointer to the originating class as parameter for the callback. If we
/// wouldn't have the type the user would have to cast it correctly via dynamic_cast or reinterpret_cast which can be
/// error prone.
/// @param[in] port_t type of the underlying port, required for testing
template <typename T, typename Subscriber, typename port_t = iox::SubscriberPortUserType>
class BaseSubscriber
{
  protected:
    using SelfType = BaseSubscriber<T, Subscriber, port_t>;

    BaseSubscriber(const BaseSubscriber& other) = delete;
    BaseSubscriber& operator=(const BaseSubscriber&) = delete;
    BaseSubscriber(BaseSubscriber&& rhs) = delete;
    BaseSubscriber& operator=(BaseSubscriber&& rhs) = delete;
    virtual ~BaseSubscriber();

    ///
    /// @brief uid Get the unique ID of the subscriber.
    /// @return The subscriber's unique ID.
    ///
    uid_t getUid() const noexcept;

    ///
    /// @brief getServiceDescription Get the service description of the subscriber.
    /// @return The service description.
    ///
    capro::ServiceDescription getServiceDescription() const noexcept;

    ///
    /// @brief subscribe Initiate subscription.
    /// @return
    ///
    void subscribe() noexcept;

    ///
    /// @brief getSubscriptionState Get current subscription state.
    /// @return The current subscription state.
    ///
    SubscribeState getSubscriptionState() const noexcept;

    ///
    /// @brief unsubscribe Unsubscribes if currently subscribed, otherwise do nothing.
    ///
    void unsubscribe() noexcept;

    // iox-#408 replace
    ///
    /// @brief hasData Check if sample is available.
    /// @return True if a new sample is available.
    ///
    bool hasSamples() const noexcept;

    // iox-#408 replace
    ///
    /// @brief hasMissedSamples Check if samples have been missed since the last hasMissedSamples() call.
    /// @return True if samples have been missed.
    /// @details Samples may be missed due to overflowing receive queue.
    ///
    bool hasMissedSamples() noexcept;

    // iox-#408 remove
    ///
    /// @brief take Take the a sample from the top of the receive queue.
    /// @return An expected containing populated optional if there is a sample available, otherwise empty.
    /// @details The memory loan for the sample is automatically released when it goes out of scope.
    ///
    cxx::expected<cxx::optional<Sample<const T>>, ChunkReceiveResult> take() noexcept;

    ///
    /// @brief takeChunk Take the chunk from the top of the receive queue.
    /// @return The header of the chunk taken.
    /// @details No automatic cleaunp of the associated chunk is performed.
    ///
    cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> takeChunk() noexcept;

    // iox-#408 replace
    ///
    /// @brief releaseQueuedSamples Releases any unread queued samples.
    ///
    void releaseQueuedSamples() noexcept;

    /// @brief releaseChunk Releases the chunk associated with the header pointer.
    /// @details The chunk must have been previosly provided by takeChunk and
    ///          not have been already released.
    void releaseChunk(const mepoo::ChunkHeader* header) noexcept;

    friend class EventAttorney;
    /// @brief Only usable by the WaitSet, not for public use. Invalidates the internal triggerHandle.
    /// @param[in] uniqueTriggerId the id of the corresponding trigger
    /// @brief Only usable by the WaitSet, not for public use
    void invalidateTrigger(const uint64_t trigger) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] subscriberEvent the event which should be attached
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const SubscriberEvent subscriberEvent) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Returns method pointer to the event corresponding
    /// hasTriggered method callback
    /// @param[in] subscriberEvent the event to which the hasTriggeredCallback is required
    WaitSetHasTriggeredCallback getHasTriggeredCallbackForEvent(const SubscriberEvent subscriberEvent) const noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    /// @param[in] subscriberEvent the event which should be detached
    void disableEvent(const SubscriberEvent subscriberEvent) noexcept;

  protected:
    BaseSubscriber() noexcept; // Required for testing.
    BaseSubscriber(const capro::ServiceDescription& service, const SubscriberOptions& subscriberOptions) noexcept;

  private:
    ///
    /// @brief The SubscriberSampleDeleter struct is a custom deleter in functor form which releases loans to a sample's
    /// underlying memory chunk via the subscriber port.
    /// Each subscriber should create its own instance of this deleter struct to work with its specific port.
    ///
    /// @note As this deleter is coupled to the Subscriber implementation, it should only be used within the subscriber
    /// context.
    ///
    struct SubscriberSampleDeleter
    {
      public:
        SubscriberSampleDeleter(port_t& port);
        void operator()(T* const ptr) const;

      private:
        std::reference_wrapper<port_t> m_port;
    };

  protected:
    port_t m_port{nullptr};
    SubscriberSampleDeleter m_sampleDeleter{m_port};
    TriggerHandle m_trigger;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_subscriber.inl"

#endif // IOX_POSH_POPO_BASE_SUBSCRIBER_HPP
