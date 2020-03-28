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
#pragma once

#include "iceoryx_posh/capro/service_description.hpp"
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

using SenderPortType = iox::popo::SenderPort;
using ReceiverPortType = iox::popo::ReceiverPort;

constexpr char MQ_ROUDI_NAME[] = "/roudi";

/// shared memmory segment for the iceoryx managment data
constexpr char SHM_NAME[] = "/iceoryx_mgmt";

// Make the user-defined literal operators accessible
using namespace units::duration_literals;

// Timeout
constexpr units::Duration PROCESS_WAITING_FOR_ROUDI_TIMEOUT = 30_s;
constexpr units::Duration DISCOVERY_INTERVAL = 100_ms;
constexpr units::Duration PROCESS_KEEP_ALIVE_INTERVAL = 3 * DISCOVERY_INTERVAL;         // > DISCOVERY_INTERVAL
constexpr units::Duration PROCESS_KEEP_ALIVE_TIMEOUT = 5 * PROCESS_KEEP_ALIVE_INTERVAL; // > PROCESS_KEEP_ALIVE_INTERVAL

// Communication Resources
#ifdef ICEORYX_LARGE_DEPLOYMENT
constexpr uint32_t MAX_PORT_NUMBER = 4096u;
#else
constexpr uint32_t MAX_PORT_NUMBER = 1024u;
#endif
constexpr uint32_t MAX_INTERFACE_NUMBER = 4u;
constexpr uint32_t MAX_RECEIVERS_PER_SENDERPORT = 256u;
constexpr uint32_t MAX_SAMPLE_ALLOCATE_PER_SENDER = 8u;
constexpr uint64_t MAX_SENDER_SAMPLE_HISTORY_CAPACITY = 16u;
constexpr uint32_t MAX_RECEIVER_QUEUE_CAPACITY = 256u;
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

// Semaphore Pool
constexpr uint32_t NUMBER_OF_SEMAPHORES = 1024u;

// Processes
constexpr uint32_t MAX_PROCESS_NUMBER = 300u;
/// Maximum number of instances of a given service, which can be found.
/// This limitation is coming due to the fixed capacity of the cxx::vector (This doesn't limit the offered number of
/// instances)
constexpr uint32_t MAX_NUMBER_OF_INSTANCES = 50u;
/// Maximum number of callbacks that can be registered with PoshRuntime::startFindService
constexpr uint32_t MAX_START_FIND_SERVICE_CALLBACKS = 50u;

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

// alias for cxx::string
using ConfigFilePathString_t = cxx::string<1024>;

namespace runtime
{
// alias for IdString
using IdString = iox::capro::IdString;
using InstanceContainer = iox::cxx::vector<IdString, MAX_NUMBER_OF_INSTANCES>;

// Return type of StartFindService() method
using FindServiceHandle = uint32_t;
// Signature for service discovery callback function
using FindServiceHandler = std::function<void(InstanceContainer&, FindServiceHandle)>;
} // namespace runtime

} // namespace iox

