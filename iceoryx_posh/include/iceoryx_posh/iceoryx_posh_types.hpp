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
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class SenderPort;
class ReceiverPort;
} // namespace popo
namespace posix
{
class UnixDomainSocket;
class MessageQueue;
} // namespace posix

using SenderPortType = iox::popo::SenderPort;
using ReceiverPortType = iox::popo::ReceiverPort;

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
constexpr units::Duration PROCESS_WAITING_FOR_ROUDI_TIMEOUT = 60_s;
constexpr units::Duration DISCOVERY_INTERVAL = 100_ms;
constexpr units::Duration PROCESS_KEEP_ALIVE_INTERVAL = 3 * DISCOVERY_INTERVAL;         // > DISCOVERY_INTERVAL
constexpr units::Duration PROCESS_KEEP_ALIVE_TIMEOUT = 5 * PROCESS_KEEP_ALIVE_INTERVAL; // > PROCESS_KEEP_ALIVE_INTERVAL


// Communication Resources
constexpr uint32_t MAX_PORT_NUMBER = build::IOX_MAX_PORT_NUMBER;
constexpr uint32_t MAX_INTERFACE_NUMBER = build::IOX_MAX_INTERFACE_NUMBER;
/// @todo remove MAX_RECEIVERS_PER_SENDERPORT when the new port building blocks are used
constexpr uint32_t MAX_RECEIVERS_PER_SENDERPORT = build::IOX_MAX_SUBSCRIBERS_PER_PUBLISHER;
constexpr uint32_t MAX_SUBSCRIBERS_PER_PUBLISHER = build::IOX_MAX_SUBSCRIBERS_PER_PUBLISHER;
constexpr uint32_t MAX_CHUNKS_ALLOCATE_PER_SENDER = build::IOX_MAX_CHUNKS_ALLOCATE_PER_SENDER;
constexpr uint64_t MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR = build::IOX_MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR;
constexpr uint32_t MAX_CHUNKS_HELD_PER_RECEIVER = build::IOX_MAX_CHUNKS_HELD_PER_RECEIVER;
constexpr uint32_t MAX_RECEIVER_QUEUE_CAPACITY = MAX_CHUNKS_HELD_PER_RECEIVER;
/// With MAX_RECEIVER_QUEUE_CAPACITY = MAX_CHUNKS_HELD_PER_RECEIVER we couple the maximum number of chunks a user is
/// allowed to hold with the maximum queue capacity.
/// This allows that a polling user can replace all the held chunks in one execution with all new ones
/// from a completely filled queue. Or the other way round, when we have a contract with the user
/// regarding how many chunks they are allowed to hold, then the queue size needs not be bigger. We
/// can provide this number of newest chunks, more the user would not be allowed to hold anyway
constexpr uint32_t MAX_INTERFACE_CAPRO_FIFO_SIZE = MAX_PORT_NUMBER;
constexpr uint32_t MAX_APPLICATION_CAPRO_FIFO_SIZE = 128u;

// Memory
constexpr uint64_t SHARED_MEMORY_ALIGNMENT = 32u;
constexpr uint32_t MAX_NUMBER_OF_MEMPOOLS = 32u;
constexpr uint32_t MAX_SHM_SEGMENTS = 100u;

constexpr uint32_t MAX_NUMBER_OF_MEMORY_PROVIDER = 8u;
constexpr uint32_t MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER = 64u;

// Message Queue
constexpr uint32_t ROUDI_MAX_MESSAGES = 5u;
constexpr uint32_t ROUDI_MESSAGE_SIZE = 512u;
constexpr uint32_t APP_MAX_MESSAGES = 5u;
constexpr uint32_t APP_MESSAGE_SIZE = 512u;

// Waitset
constexpr uint32_t NUMBER_OF_SEMAPHORES = 1024u;
constexpr uint32_t MAX_NUMBER_OF_CONDITIONS = 128u;

// Processes
constexpr uint32_t MAX_PROCESS_NUMBER = 300u;
/// Maximum number of instances of a given service, which can be found.
/// This limitation is coming due to the fixed capacity of the cxx::vector (This doesn't limit the offered number of
/// instances)
constexpr uint32_t MAX_NUMBER_OF_INSTANCES = 50u;

// Runnables
constexpr uint32_t MAX_RUNNABLE_NUMBER = 1000u;
constexpr uint32_t MAX_RUNNABLE_PER_PROCESS = 50u;

constexpr uint32_t MAX_PROCESS_NAME_LENGTH = 100u;
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

// Default properties of ChunkDistributorData
struct DefaultChunkDistributorConfig
{
    static constexpr uint32_t MAX_QUEUES = MAX_SUBSCRIBERS_PER_PUBLISHER;
    static constexpr uint64_t MAX_HISTORY_CAPACITY = MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR;
};

// Default properties of ChunkQueueData
struct DefaultChunkQueueConfig
{
    static constexpr uint32_t MAX_QUEUE_CAPACITY = MAX_RECEIVER_QUEUE_CAPACITY;
};

// alias for cxx::string
using ConfigFilePathString_t = cxx::string<1024>;
using ProcessName_t = cxx::string<100>;

namespace runtime
{
// alias for IdString
using IdString = iox::capro::IdString;
using InstanceContainer = iox::cxx::vector<IdString, MAX_NUMBER_OF_INSTANCES>;
} // namespace runtime

} // namespace iox

#endif // IOX_POSH_ICEORYX_POSH_TYPES_HPP
