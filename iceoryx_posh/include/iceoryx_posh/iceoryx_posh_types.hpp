// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
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
#ifndef IOX_POSH_ICEORYX_POSH_TYPES_HPP
#define IOX_POSH_ICEORYX_POSH_TYPES_HPP

#include "iceoryx_hoofs/cxx/variant_queue.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/ipc_channel.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_posh/iceoryx_posh_deployment.hpp"
#include "iox/duration.hpp"
#include "iox/function.hpp"
#include "iox/log/logstream.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"
#include "iox/vector.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class UniquePortId;
struct BasePortData;

class PublisherPortRouDi;
class PublisherPortUser;
class SubscriberPortRouDi;
class SubscriberPortUser;
} // namespace popo
namespace capro
{
class ServiceDescription;
}

using PublisherPortRouDiType = iox::popo::PublisherPortRouDi;
using PublisherPortUserType = iox::popo::PublisherPortUser;
using SubscriberPortRouDiType = iox::popo::SubscriberPortRouDi;
using SubscriberPortUserType = iox::popo::SubscriberPortUser;

using SubscriberPortType = iox::build::CommunicationPolicy;

//--------- Communication Resources Start---------------------
// Publisher
constexpr uint32_t MAX_PUBLISHERS = build::IOX_MAX_PUBLISHERS;
constexpr uint32_t MAX_SUBSCRIBERS_PER_PUBLISHER = build::IOX_MAX_SUBSCRIBERS_PER_PUBLISHER;
constexpr uint32_t MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY =
    build::IOX_MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY;
constexpr uint64_t MAX_PUBLISHER_HISTORY = build::IOX_MAX_PUBLISHER_HISTORY;
// Subscriber
constexpr uint32_t MAX_SUBSCRIBERS = build::IOX_MAX_SUBSCRIBERS;
constexpr uint32_t MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY =
    build::IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY;
constexpr uint32_t MAX_SUBSCRIBER_QUEUE_CAPACITY = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY;
// Introspection is using the following publisherPorts, which reduced the number of ports available for the user
// 1x publisherPort mempool introspection
// 1x publisherPort process introspection
// 3x publisherPort port introspection
constexpr uint32_t PUBLISHERS_RESERVED_FOR_INTROSPECTION = 5;
constexpr uint32_t PUBLISHERS_RESERVED_FOR_SERVICE_REGISTRY = 1;
constexpr uint32_t NUMBER_OF_INTERNAL_PUBLISHERS =
    PUBLISHERS_RESERVED_FOR_INTROSPECTION + PUBLISHERS_RESERVED_FOR_SERVICE_REGISTRY;
/// With MAX_SUBSCRIBER_QUEUE_CAPACITY = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY we couple the maximum number of
/// chunks a user is allowed to hold with the maximum queue capacity. This allows that a polling user can replace all
/// the held chunks in one execution with all new ones from a completely filled queue. Or the other way round, when we
/// have a contract with the user regarding how many chunks they are allowed to hold, then the queue size needs not be
/// bigger. We can provide this number of newest chunks, more the user would not be allowed to hold anyway
// Gateway
constexpr uint32_t MAX_INTERFACE_NUMBER = build::IOX_MAX_INTERFACE_NUMBER;
constexpr uint32_t MAX_INTERFACE_CAPRO_FIFO_SIZE = MAX_PUBLISHERS;
constexpr uint32_t MAX_CHANNEL_NUMBER = MAX_PUBLISHERS + MAX_SUBSCRIBERS;
constexpr uint32_t MAX_GATEWAY_SERVICES = 2 * MAX_CHANNEL_NUMBER;
// Client
constexpr uint32_t MAX_CLIENTS = build::IOX_MAX_SUBSCRIBERS;
constexpr uint32_t MAX_REQUESTS_ALLOCATED_SIMULTANEOUSLY = 4U;
constexpr uint32_t MAX_RESPONSES_PROCESSED_SIMULTANEOUSLY = build::IOX_MAX_RESPONSES_PROCESSED_SIMULTANEOUSLY;
constexpr uint32_t MAX_RESPONSE_QUEUE_CAPACITY = build::IOX_MAX_RESPONSE_QUEUE_CAPACITY;
// Server
constexpr uint32_t MAX_SERVERS = build::IOX_MAX_PUBLISHERS;
constexpr uint32_t MAX_CLIENTS_PER_SERVER = build::IOX_MAX_CLIENTS_PER_SERVER;
constexpr uint32_t MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY = 4U;
constexpr uint32_t MAX_RESPONSES_ALLOCATED_SIMULTANEOUSLY = MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY;
constexpr uint32_t MAX_REQUEST_QUEUE_CAPACITY = build::IOX_MAX_REQUEST_QUEUE_CAPACITY;
// Waitset
namespace popo
{
using WaitSetIsConditionSatisfiedCallback = optional<function<bool()>>;
}
constexpr uint32_t MAX_NUMBER_OF_CONDITION_VARIABLES = build::IOX_MAX_NUMBER_OF_CONDITION_VARIABLES;

constexpr uint32_t MAX_NUMBER_OF_NOTIFIERS = build::IOX_MAX_NUMBER_OF_NOTIFIERS;
/// @note Waitset and Listener share both the max available notifiers, if one of them is running out of of notifiers
/// the variable above must be increased
constexpr uint32_t MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET = MAX_NUMBER_OF_NOTIFIERS;
constexpr uint32_t MAX_NUMBER_OF_EVENTS_PER_LISTENER = MAX_NUMBER_OF_NOTIFIERS;
//--------- Communication Resources End---------------------

// Memory
constexpr uint32_t MAX_NUMBER_OF_MEMPOOLS = build::IOX_MAX_NUMBER_OF_MEMPOOLS;
constexpr uint32_t MAX_SHM_SEGMENTS = build::IOX_MAX_SHM_SEGMENTS;

constexpr uint32_t MAX_NUMBER_OF_MEMORY_PROVIDER = 8U;
constexpr uint32_t MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER = 64U;

constexpr uint32_t CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT{8U};
constexpr uint32_t CHUNK_NO_USER_HEADER_SIZE{0U};
constexpr uint32_t CHUNK_NO_USER_HEADER_ALIGNMENT{1U};

// Message Queue
constexpr uint32_t ROUDI_MAX_MESSAGES = 5U;
constexpr uint32_t ROUDI_MESSAGE_SIZE = 512U;
constexpr uint32_t APP_MAX_MESSAGES = 5U;
constexpr uint32_t APP_MESSAGE_SIZE = 512U;


// Processes
constexpr uint32_t MAX_PROCESS_NUMBER = build::IOX_MAX_PROCESS_NUMBER;

// Service Discovery
constexpr uint32_t SERVICE_REGISTRY_CAPACITY = MAX_PUBLISHERS + MAX_SERVERS;
constexpr uint32_t MAX_FINDSERVICE_RESULT_SIZE = SERVICE_REGISTRY_CAPACITY;

constexpr const char SERVICE_DISCOVERY_SERVICE_NAME[] = "ServiceDiscovery";
constexpr const char SERVICE_DISCOVERY_INSTANCE_NAME[] = "RouDi_ID";
constexpr const char SERVICE_DISCOVERY_EVENT_NAME[] = "ServiceRegistry";

// Nodes
constexpr uint32_t MAX_NODE_NUMBER = build::IOX_MAX_NODE_NUMBER;
constexpr uint32_t MAX_NODE_PER_PROCESS = build::IOX_MAX_NODE_PER_PROCESS;

constexpr uint32_t MAX_RUNTIME_NAME_LENGTH = build::IOX_MAX_RUNTIME_NAME_LENGTH;

static_assert(MAX_RUNTIME_NAME_LENGTH <= MAX_IPC_CHANNEL_NAME_LENGTH,
              "Invalid configuration of maximum runtime name length");
static_assert(MAX_PROCESS_NUMBER * MAX_NODE_PER_PROCESS > MAX_NODE_NUMBER, "Invalid configuration for nodes");

enum class SubscribeState : uint32_t
{
    NOT_SUBSCRIBED = 0,
    SUBSCRIBE_REQUESTED,
    SUBSCRIBED,
    UNSUBSCRIBE_REQUESTED,
    WAIT_FOR_OFFER
};

enum class ConnectionState : uint32_t
{
    NOT_CONNECTED = 0,
    CONNECT_REQUESTED,
    CONNECTED,
    DISCONNECT_REQUESTED,
    WAIT_FOR_OFFER
};

/// @brief Converts the ConnectionState to a string literal
/// @param[in] value to convert to a string literal
/// @return pointer to a string literal
inline constexpr const char* asStringLiteral(ConnectionState value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline std::ostream& operator<<(std::ostream& stream, ConnectionState value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with iox::log::LogStream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline log::LogStream& operator<<(log::LogStream& stream, ConnectionState value) noexcept;

// Default properties of ChunkDistributorData
struct DefaultChunkDistributorConfig
{
    static constexpr uint32_t MAX_QUEUES = MAX_SUBSCRIBERS_PER_PUBLISHER;
    static constexpr uint64_t MAX_HISTORY_CAPACITY = MAX_PUBLISHER_HISTORY;
};

// Default properties of ChunkQueueData
struct DefaultChunkQueueConfig
{
    static constexpr uint64_t MAX_QUEUE_CAPACITY = MAX_SUBSCRIBER_QUEUE_CAPACITY;
};

// alias for string
using RuntimeName_t = string<MAX_RUNTIME_NAME_LENGTH>;
using NodeName_t = string<build::IOX_MAX_NODE_NAME_LENGTH>;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
using ShmName_t = string<128>;

namespace capro
{
using IdString_t = string<build::IOX_MAX_ID_STRING_LENGTH>;
} // namespace capro

/// @todo iox-#539 Move everything in this namespace to iceoryx_roudi_types.hpp once we move RouDi to a separate CMake
/// target
namespace roudi
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
using ConfigFilePathString_t = string<1024>;

constexpr const char ROUDI_LOCK_NAME[] = "iox-unique-roudi";
constexpr const char IPC_CHANNEL_ROUDI_NAME[] = "roudi";

/// shared memory segment for the iceoryx management data
constexpr const char SHM_NAME[] = "iceoryx_mgmt";

// this is used by the UniquePortId
constexpr uint16_t DEFAULT_UNIQUE_ROUDI_ID{0U};

// Timeout
using namespace units::duration_literals;
constexpr units::Duration PROCESS_DEFAULT_TERMINATION_DELAY = 0_s;
constexpr units::Duration PROCESS_DEFAULT_KILL_DELAY = 45_s;
constexpr units::Duration PROCESS_TERMINATED_CHECK_INTERVAL = 250_ms;
constexpr units::Duration DISCOVERY_INTERVAL = 100_ms;

/// @brief Controls process alive monitoring. Upon timeout, a monitored process is removed
/// and its resources are made available. The process can then start and register itself again.
/// Contrarily, unmonitored processes can be restarted but registration will fail.
/// Once Runlevel Management is extended, it will detect absent processes. Those processes can register again.
/// ON - all processes are monitored
/// OFF - no process is monitored
enum class MonitoringMode
{
    ON,
    OFF
};

iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const MonitoringMode& mode) noexcept;
} // namespace roudi

namespace mepoo
{
using SequenceNumber_t = std::uint64_t;
} // namespace mepoo

namespace runtime
{
using namespace units::duration_literals;
constexpr units::Duration PROCESS_WAITING_FOR_ROUDI_TIMEOUT = 60_s;
constexpr units::Duration PROCESS_KEEP_ALIVE_INTERVAL = 3 * roudi::DISCOVERY_INTERVAL;  // > DISCOVERY_INTERVAL
constexpr units::Duration PROCESS_KEEP_ALIVE_TIMEOUT = 5 * PROCESS_KEEP_ALIVE_INTERVAL; // > PROCESS_KEEP_ALIVE_INTERVAL
} // namespace runtime

namespace version
{
static const uint64_t COMMIT_ID_STRING_SIZE = 12U;
using CommitIdString_t = string<COMMIT_ID_STRING_SIZE>;
static const uint64_t BUILD_DATE_STRING_SIZE = 36U;
using BuildDateString_t = string<BUILD_DATE_STRING_SIZE>;
} // namespace version

} // namespace iox

#include "iceoryx_posh/iceoryx_posh_types.inl"

#endif // IOX_POSH_ICEORYX_POSH_TYPES_HPP
