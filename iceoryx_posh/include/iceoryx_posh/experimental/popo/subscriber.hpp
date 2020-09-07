// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{

using uid_t = uint64_t;

enum class SubscriptionState
{
    SUBSCRIBED,
    NOT_SUBSCRIBED,
    SUBSCRIPTION_PENDING
};

enum class SubscriberError : uint8_t
{
    UNKNOWN
};

///
/// Subscribe to a particular service in the iceoryx system.
/// Data can be retrieved by either:
/// * Registering a callback to process the incoming samples
/// * Manually polling for available samples
///
template<typename T, typename recvport_t = iox::popo::ReceiverPort>
class BaseSubscriber {
public:

    BaseSubscriber(const BaseSubscriber& other) = delete;
    BaseSubscriber& operator=(const BaseSubscriber&) = delete;
    BaseSubscriber(BaseSubscriber&& rhs) = default;
    BaseSubscriber& operator=(BaseSubscriber&& rhs) = default;
    ~BaseSubscriber() = default;

    capro::ServiceDescription getServiceDescription() const noexcept;
    uid_t uid() const noexcept;

    ///
    /// @brief subscribe Initiate subscription.
    /// @param cacheSize
    /// @return
    ///
    cxx::expected<SubscriberError> subscribe(const uint64_t cacheSize = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;

    ///
    /// @brief subscribe
    /// @param cb A callable with the signature void(unique_ptr<T>) to be used to process incoming data
    /// @param cacheSize
    ///
    /// @details The provided callback should be as simple as possible to prevent backing up the receive thread. If more
    /// costly processing is required, the callback should delegate this to a separate thread.
    ///
    /// @return
    ///
    template<typename Callback>
    cxx::expected<SubscriberError> subscribe(Callback cb, const uint64_t cacheSize = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;

    // Note this might need revision. How costly is it to evaluate a predicate for every incoming pt ?
    ///
    /// @brief subscribe
    /// @param cb A callable with the signature void(unique_ptr<T>) to be used to process incoming data
    /// @param p A callable with the signature void(unique_ptr<T>) to be used as a predicate that selects data to consider
    /// @param cacheSize
    /// @return
    ///
    template<typename Callback, typename Predicate>
    cxx::expected<SubscriberError> subscribe(Callback cb,  Predicate p, const uint64_t cacheSize = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;

    SubscriptionState getSubscriptionState() const noexcept;
    void unsubscribe() noexcept;

    ///
    /// @brief hasData Check if sample is available.
    /// @return True if a new sample is available.
    ///
    bool hasData() const noexcept;

    ///
    /// @brief receive Receive the next sample if available.
    /// @return
    ///
    cxx::optional<cxx::unique_ptr<T>> receive() noexcept;

    ///
    /// @brief clearReceiveBuffer Releases all unread items in the receive buffer.
    ///
    void clearReceiveBuffer() noexcept;

    ///
    /// @brief setCallback Sets a callback to execute on the received data
    /// @param cb Callback to execute
    ///
    template<typename Callback>
    void setCallback(Callback cb) noexcept;

    ///
    /// @brief setCallback Sets a callback to execute on the received data if the provided predicate evaluates to true
    /// @param cb Callback to execute
    /// @param p A predicate to evaluate with each incoming data point
    ///
    template<typename Callback, typename Predicate>
    void setCallback(Callback cb, Predicate p) noexcept;

    ///
    /// @brief unsetCallback
    ///
    void unsetCallback() noexcept;

    // Are these functions still relevant with the new API ?
    //
    // void overrideCallbackReference(const Subscriber_t& receiverWithRererenceToUse) noexcept;
    // posix::Semaphore* getSemaphore() const noexcept;
    // void setChunkReceiveSemaphore(posix::Semaphore* semaphore) noexcept;
    // bool isChunkReceiveSemaphoreSet() noexcept;
    // void unsetChunkReceiveSemaphore() noexcept;
    // void setNotifyOnOverflow(const bool value) noexcept;

    // What is the use-case for these two functions ?
    //
    // bool waitForChunk(const uint32_t timeoutMs) noexcept;
    // bool tryWaitForChunk() noexcept;

    // ===== Discard Pile ===== //
    //
    // bool getChunk(const mepoo::ChunkHeader** chunkHeader) noexcept;
    // bool releaseChunk(const mepoo::ChunkHeader* const chunkHeader) noexcept;
    // bool releaseChunk(const void* const payload) noexcept;

protected:
    BaseSubscriber(const capro::ServiceDescription& service);

protected:
    bool m_subscriptionRequested;

private:
    recvport_t m_port{nullptr};

};

template<typename T>
class TypedSubscriber : protected BaseSubscriber<T>
{
public:
    TypedSubscriber(const capro::ServiceDescription& service);
    TypedSubscriber(const TypedSubscriber& other) = delete;
    TypedSubscriber& operator=(const TypedSubscriber&) = delete;
    TypedSubscriber(TypedSubscriber&& rhs) = default;
    TypedSubscriber& operator=(TypedSubscriber&& rhs) = default;
    ~TypedSubscriber() = default;

    cxx::expected<SubscriberError> subscribe(const uint64_t cacheSize = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;
    SubscriptionState getSubscriptionState() const noexcept;
    void unsubscribe() noexcept;

    bool hasData() const noexcept;
    cxx::optional<cxx::unique_ptr<T>> receive() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/subscriber.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_HPP
