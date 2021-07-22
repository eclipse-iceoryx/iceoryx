// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_PORT_QUEUE_POLICIES_HPP
#define IOX_POSH_POPO_PORT_QUEUE_POLICIES_HPP

#include <cstdint>

namespace iox
{
namespace popo
{
/// @brief Used by publisher
/// @todo iox-#27 replace with ConsumerTooSlowPolicy
enum class SubscriberTooSlowPolicy : uint8_t
{
    WAIT_FOR_SUBSCRIBER,
    DISCARD_OLDEST_DATA
};
/// @brief Used by subscriber
/// @todo iox-#27 replace with QueueFullPolicy2
enum class QueueFullPolicy : uint8_t
{
    BLOCK_PUBLISHER,
    DISCARD_OLDEST_DATA
};

/// @brief Used by producers how to adjust to slow consumer
enum class ConsumerTooSlowPolicy
{
    /// Waits for the consumer it it's queue is full
    WAIT_FOR_CONSUMER,
    /// Discards the oldest data and pushes the newest one into the queue
    DISCARD_OLDEST_DATA
};

/// @brief Used by consumers to request a specific behavior from the producer
/// @todo iox-#27 Rename this to QueueFullPolicy
enum class QueueFullPolicy2
{
    /// Requests the producer to block when the consumer queue is full
    BLOCK_PRODUCER,
    /// Request to discard the oldest data and push the newest one into the queue
    DISCARD_OLDEST_DATA
};

} // namespace popo
} // namespace iox
#endif // IOX_POSH_POPO_PORT_QUEUE_POLICIES_HPP
