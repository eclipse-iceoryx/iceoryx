// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_SUBSCRIBER_HPP
#define IOX_POSH_POPO_TYPED_SUBSCRIBER_HPP

#include "iceoryx_posh/internal/popo/sample_deleter.hpp"
#include "iceoryx_posh/popo/base_subscriber.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename base_subscriber_t = BaseSubscriber<T>>
class TypedSubscriber : public base_subscriber_t
{
    using SelfType = TypedSubscriber<T, base_subscriber_t>;
    static_assert(!std::is_void<T>::value, "Type must not be void. Use the UntypedSubscriber for void types.");

  public:
    TypedSubscriber(const capro::ServiceDescription& service,
                    const SubscriberOptions& subscriberOptions = SubscriberOptions());
    TypedSubscriber(const TypedSubscriber& other) = delete;
    TypedSubscriber& operator=(const TypedSubscriber&) = delete;
    TypedSubscriber(TypedSubscriber&& rhs) = delete;
    TypedSubscriber& operator=(TypedSubscriber&& rhs) = delete;
    virtual ~TypedSubscriber() = default;

    ///
    /// @brief Take the samples from the top of the receive queue.
    /// @return Either a sample or a ChunkReceiveResult.
    /// @details The sample takes care of the cleanup. Don't store the raw pointer to the content of the sample, but
    /// always the whole sample.
    ///
    cxx::expected<Sample<const T>, ChunkReceiveResult> take() noexcept;

    using PortType = typename base_subscriber_t::PortType;
    using SubscriberSampleDeleter = SampleDeleter<PortType>;

    template <uint64_t Capacity>
    friend class WaitSet;

  protected:
    using base_subscriber_t::port;

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
                                            const EventInfo::Callback<SelfType> callback = nullptr) noexcept;

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
                                            const EventInfo::Callback<SelfType> callback) noexcept;

  private:
    SubscriberSampleDeleter m_sampleDeleter{port()};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/typed_subscriber.inl"

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_HPP
