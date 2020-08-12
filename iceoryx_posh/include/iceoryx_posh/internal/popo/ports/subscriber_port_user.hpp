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

#ifndef IOX_POPO_SUBSCRIBER_PORT_USER_HPP_
#define IOX_POPO_SUBSCRIBER_PORT_USER_HPP_

#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
/// @brief The SubscriberPortUser provides the API for accessing a subscriber port from the user side. The subscriber
/// port is divided in the parts SubscriberPortData, SubscriberPortUser and different classes for RouDi side access.
/// The SubscriberPortUser uses the functionality of a ChunkReceiver for receiving shared memory chunks. Additionally it
/// provides the subscribe / unsubscribe API which controls whether the subscriber ports shall try to subscribe to
/// matching publisher ports
class SubscriberPortUser
{
  public:
    using MemberType_t = SubscriberPortData;

    explicit SubscriberPortUser(cxx::not_null<MemberType_t* const> subscriberPortDataPtr) noexcept;

    SubscriberPortUser(const SubscriberPortUser& other) = delete;
    SubscriberPortUser& operator=(const SubscriberPortUser&) = delete;
    SubscriberPortUser(SubscriberPortUser&& rhs) = default;
    SubscriberPortUser& operator=(SubscriberPortUser&& rhs) = default;
    ~SubscriberPortUser() = default;

    /// @brief try to subscribe to all matching publishers
    /// @param[in] queueCapacity, capacity of the queue where chunks are stored before they are passed to the user with
    /// getChunk. Caution: Depending on the underlying queue there can be a different overflow behavior
    void subscribe(const uint32_t queueCapacity = MAX_RECEIVER_QUEUE_CAPACITY) noexcept;

    /// @brief unsubscribe from publishers, if there are any to which we are currently subscribed
    void unsubscribe() noexcept;

    /// @brief get the current subscription state. Caution: There can be delays between calling subscribe and a change
    /// in the subscription state. The subscription state can also change without user interaction if publishers come
    /// and go
    /// @return SubscribeState
    SubscribeState getSubscriptionState() const noexcept;

    /// @brief Tries to get the next chunk from the queue. If there is a new one, the ChunkHeader of the oldest chunk in
    /// the queue is returned (FiFo queue)
    /// @return optional that has a new chunk header or no value if there are no new chunks in the underlying queue,
    /// ChunkReceiveError on error
    cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError> getChunk() noexcept;

    /// @brief Release a chunk that was obtained with getChunk
    /// @param[in] chunkHeader, pointer to the ChunkHeader to release
    void releaseChunk(const mepoo::ChunkHeader* chunkHeader) noexcept;

    /// @brief Release all the chunks that are currently queued up.
    void releaseQueuedChunks() noexcept;

    /// @brief check if there are chunks in the queue
    /// @return if there are chunks in the queue return true, otherwise false
    bool hasNewChunks() noexcept;

    /// @brief check if there was a queue overflow since the last call of hasLostChunks
    /// @return true if the underlying queue overflowed since last call of this method, otherwise false
    bool hasLostChunks() noexcept;

    /// @todo we first need the new condition variable
    void attachConditionVariable() noexcept;
    void detachConditionVaribale() noexcept;
    bool isConditionVariableAttached() noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

    MemberType_t* m_subscriberPortDataPtr;

    using ChunkQueuePopper_t = ChunkQueuePopper<SubscriberPortData::ChunkQueueData_t>;
    ChunkReceiver<ChunkQueuePopper_t> m_chunkReceiver;
};

} // namespace popo
} // namespace iox


#endif
