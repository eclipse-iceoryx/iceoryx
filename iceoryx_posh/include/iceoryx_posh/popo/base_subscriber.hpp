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

    ///
    /// @brief hasData Check if sample is available.
    /// @return True if a new sample is available.
    ///
    bool hasSamples() const noexcept;

    ///
    /// @brief hasMissedSamples Check if samples have been missed since the last hasMissedSamples() call.
    /// @return True if samples have been missed.
    /// @details Samples may be missed due to overflowing receive queue.
    ///
    bool hasMissedSamples() noexcept;

    ///
    /// @brief take Take the a sample from the top of the receive queue.
    /// @return An expected containing populated optional if there is a sample available, otherwise empty.
    /// @details The memory loan for the sample is automatically released when it goes out of scope.
    ///
    cxx::expected<cxx::optional<Sample<const T>>, ChunkReceiveError> take() noexcept;

    ///
    /// @brief releaseQueuedSamples Releases any unread queued samples.
    ///
    void releaseQueuedSamples() noexcept;

    template <uint64_t Capacity>
    friend class WaitSet;

  protected:
    BaseSubscriber() noexcept; // Required for testing.
    BaseSubscriber(const capro::ServiceDescription& service, const SubscriberOptions& subscriberOptions) noexcept;

    void invalidateTrigger(const uint64_t trigger) noexcept;

    /// @brief attaches a WaitSet to the subscriber
    /// @param[in] waitset reference to the waitset to which the subscriber should be attached to
    /// @param[in] subscriberEvent the event which should be attached
    /// @param[in] eventId a custom uint64_t which can be set by the user with no restriction. could be used to either
    ///            identify an event uniquely or to group multiple events together when they share the same eventId
    /// @param[in] callback callback which is attached to the trigger and which can be called
    ///            later by the user
    /// @return success if the subscriber is attached otherwise an WaitSetError enum which describes
    ///            the error
    template <uint64_t WaitSetCapacity>
    cxx::expected<WaitSetError> enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                            const SubscriberEvent subscriberEvent,
                                            const uint64_t eventId = EventInfo::INVALID_ID,
                                            const EventInfo::Callback<Subscriber> callback = nullptr) noexcept;

    /// @brief attaches a WaitSet to the subscriber
    /// @param[in] waitset reference to the waitset to which the subscriber should be attached to
    /// @param[in] subscriberEvent the event which should be attached
    /// @param[in] callback callback which is attached to the trigger and which can be called
    ///            later by the user
    /// @return success if the subscriber is attached otherwise an WaitSetError enum which describes
    ///            the error
    template <uint64_t WaitSetCapacity>
    cxx::expected<WaitSetError> enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                            const SubscriberEvent subscriberEvent,
                                            const EventInfo::Callback<Subscriber> callback) noexcept;

    /// @brief detaches a specified event from the subscriber, if the event was not attached nothing happens
    /// @param[in] subscriberEvent the event which should be detached
    void disableEvent(const SubscriberEvent subscriberEvent) noexcept;

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
