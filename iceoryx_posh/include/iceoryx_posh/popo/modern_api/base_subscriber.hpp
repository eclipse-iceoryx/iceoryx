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

#ifndef IOX_POSH_POPO_BASE_SUBSCRIBER_HPP
#define IOX_POSH_POPO_BASE_SUBSCRIBER_HPP

#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/popo/modern_api/sample.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

template <typename T, typename port_t = iox::SubscriberPortUserType>
class BaseSubscriber : public Condition
{
  public:
    BaseSubscriber(const BaseSubscriber& other) = delete;
    BaseSubscriber& operator=(const BaseSubscriber&) = delete;
    BaseSubscriber(BaseSubscriber&& rhs) = delete;
    BaseSubscriber& operator=(BaseSubscriber&& rhs) = delete;
    ~BaseSubscriber() = default;

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
    /// @param queueCapacity
    /// @return
    ///
    void
    subscribe(const uint64_t queueCapacity = SubscriberPortUser::MemberType_t::ChunkQueueData_t::MAX_CAPACITY) noexcept;

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
    bool hasNewSamples() const noexcept;

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

    // Condition overrides
    virtual bool setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept override;
    virtual bool unsetConditionVariable() noexcept override;
    virtual bool hasTriggered() const noexcept override;

  protected:
    BaseSubscriber() noexcept // Required for testing.
    {};
    BaseSubscriber(const capro::ServiceDescription& service);

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
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/modern_api/base_subscriber.inl"

#endif // IOX_POSH_POPO_BASE_SUBSCRIBER_HPP
