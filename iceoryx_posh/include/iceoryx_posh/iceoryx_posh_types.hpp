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
#ifndef IOX_POSH_ICEORYX_POSH_TYPES_HPP
#define IOX_POSH_ICEORYX_POSH_TYPES_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_deployment.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/cxx/variant_queue.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename>
class TypedUniqueId;
struct BasePortData;

class SenderPort;   /// @deprecated #25
class ReceiverPort; /// @deprecated #25

class PublisherPortUser;
class PublisherPortRouDi;

class SubscriberPortUser;
} // namespace popo
namespace posix
{
class UnixDomainSocket;
class MessageQueue;
} // namespace posix

using SenderPortType = iox::popo::SenderPort;     /// @deprecated #25
using ReceiverPortType = iox::popo::ReceiverPort; /// @deprecated #25
using PublisherPortRouDiType = iox::popo::PublisherPortRouDi;
using PublisherPortUserType = iox::popo::PublisherPortUser;
using SubscriberPortUserType = iox::popo::SubscriberPortUser;
using UniquePortId = popo::TypedUniqueId<popo::BasePortData>;

using SubscriberPortType = iox::build::CommunicationPolicy;

constexpr char MQ_ROUDI_NAME[] = "/roudi";

/// @brief The socket is created in the current path if no absolute path is given hence
///      we need an absolut path so that every application knows where our sockets can
///      be found.
#if defined(__APPLE__)
using IpcChannelType = iox::posix::UnixDomainSocket;
#else
using IpcChannelType = iox::posix::MessageQueue;
#endif

/// shared memmory segment for the iceoryx managment data
constexpr char SHM_NAME[] = "/iceoryx_mgmt";

// Make the user-defined literal operators accessible
using namespace units::duration_literals;

// Timeout
constexpr units::Duration PROCESS_DEFAULT_KILL_DELAY = 45_s;
constexpr units::Duration PROCESS_TERMINATED_CHECK_INTERVAL = 250_ms;
constexpr units::Duration PROCESS_WAITING_FOR_ROUDI_TIMEOUT = 60_s;
constexpr units::Duration DISCOVERY_INTERVAL = 100_ms;
constexpr units::Duration PROCESS_KEEP_ALIVE_INTERVAL = 3 * DISCOVERY_INTERVAL;         // > DISCOVERY_INTERVAL
constexpr units::Duration PROCESS_KEEP_ALIVE_TIMEOUT = 5 * PROCESS_KEEP_ALIVE_INTERVAL; // > PROCESS_KEEP_ALIVE_INTERVAL

/// @todo remove MAX_RECEIVERS_PER_SENDERPORT when the new port building blocks are used
constexpr uint32_t MAX_RECEIVERS_PER_SENDERPORT = build::IOX_MAX_SUBSCRIBERS_PER_PUBLISHER;

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
constexpr uint32_t MAX_CLIENTS = build::IOX_MAX_SUBSCRIBERS; /// @todo
constexpr uint32_t MAX_REQUESTS_ALLOCATED_SIMULTANEOUSLY = 4U;
constexpr uint32_t MAX_RESPONSES_PROCESSED_SIMULTANEOUSLY = 16U;
constexpr uint32_t MAX_RESPONSE_QUEUE_CAPACITY = 16U;
// Server
constexpr uint32_t MAX_SERVERS = build::IOX_MAX_PUBLISHERS; /// @todo
constexpr uint32_t MAX_CLIENTS_PER_SERVER = 256U;
constexpr uint32_t MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY = 4U;
constexpr uint32_t MAX_RESPONSES_ALLOCATED_SIMULTANEOUSLY = MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY;
constexpr uint32_t MAX_REQUEST_QUEUE_CAPACITY = 1024;
// Waitset
constexpr uint32_t MAX_NUMBER_OF_CONDITION_VARIABLES = 1024U;
constexpr uint32_t MAX_NUMBER_OF_CONDITIONS_PER_WAITSET = 128U;
//--------- Communication Resources End---------------------

constexpr uint32_t MAX_APPLICATION_CAPRO_FIFO_SIZE = 128U;

// Memory
constexpr uint64_t SHARED_MEMORY_ALIGNMENT = 32U;
constexpr uint32_t MAX_NUMBER_OF_MEMPOOLS = 32U;
constexpr uint32_t MAX_SHM_SEGMENTS = 100U;

constexpr uint32_t MAX_NUMBER_OF_MEMORY_PROVIDER = 8U;
constexpr uint32_t MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER = 64U;

// Message Queue
constexpr uint32_t ROUDI_MAX_MESSAGES = 5U;
constexpr uint32_t ROUDI_MESSAGE_SIZE = 512U;
constexpr uint32_t APP_MAX_MESSAGES = 5U;
constexpr uint32_t APP_MESSAGE_SIZE = 512U;


// Processes
constexpr uint32_t MAX_PROCESS_NUMBER = 300U;
/// Maximum number of instances of a given service, which can be found.
/// This limitation is coming due to the fixed capacity of the cxx::vector (This doesn't limit the offered number of
/// instances)
constexpr uint32_t MAX_NUMBER_OF_INSTANCES = 50U;

// Runnables
constexpr uint32_t MAX_RUNNABLE_NUMBER = 1000U;
constexpr uint32_t MAX_RUNNABLE_PER_PROCESS = 50U;

#if defined(__APPLE__)
/// @note on macOS the process name length needs to be decreased since the process name is used for the unix domain
/// socket path which has a capacity for only 103 characters. The full path consists of UnixDomainSocket::PATH_PREFIX,
/// which is currently 5 characters and the specified process name
constexpr uint32_t MAX_PROCESS_NAME_LENGTH = 98U;
#else
constexpr uint32_t MAX_PROCESS_NAME_LENGTH = 100U;
#endif

static_assert(MAX_PROCESS_NUMBER * MAX_RUNNABLE_PER_PROCESS > MAX_RUNNABLE_NUMBER,
              "Invalid configuration for runnables");

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
    CONNNECTED,
    DISCONNECT_REQUESTED,
    WAIT_FOR_OFFER
};

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

// alias for cxx::string
using ConfigFilePathString_t = cxx::string<1024>;
using ProcessName_t = cxx::string<MAX_PROCESS_NAME_LENGTH>;
using RunnableName_t = cxx::string<100>;

namespace runtime
{
// alias for IdString
using IdString = iox::capro::IdString;
using InstanceContainer = iox::cxx::vector<IdString, MAX_NUMBER_OF_INSTANCES>;
} // namespace runtime

} // namespace iox

#endif // IOX_POSH_ICEORYX_POSH_TYPES_HPP
