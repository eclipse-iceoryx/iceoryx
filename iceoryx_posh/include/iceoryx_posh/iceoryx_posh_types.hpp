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
constexpr uint32_t MAX_PORT_NUMBER = 2048;
constexpr uint32_t MAX_INTERFACE_NUMBER = 16;
constexpr uint32_t MAX_RECEIVERS_PER_SENDERPORT = 256;
constexpr uint32_t MAX_SAMPLE_ALLOCATE_PER_SENDER = 16;
constexpr uint32_t MAX_RECEIVER_QUEUE_SIZE = 256;
constexpr uint32_t MAX_INTERFACE_CAPRO_FIFO_SIZE = MAX_PORT_NUMBER;
constexpr uint32_t MAX_APPLICATION_CAPRO_FIFO_SIZE = 128;

// Memory
constexpr uint32_t MAX_NUMBER_OF_MEMPOOLS = 32;
constexpr uint32_t MAX_SHM_SEGMENTS = 100;

// Message Queue
constexpr long ROUDI_MAX_MESSAGES = 5;
constexpr long ROUDI_MESSAGE_SIZE = 512;
constexpr long APP_MAX_MESSAGES = 5;
constexpr long APP_MESSAGE_SIZE = 512;

// Processes
constexpr uint32_t MAX_PROCESS_NUMBER = 300;

// Runnables
constexpr uint32_t MAX_RUNNABLE_NUMBER = 1000;
constexpr uint32_t MAX_RUNNABLE_PER_PROCESS = 50;
static_assert(MAX_PROCESS_NUMBER * MAX_RUNNABLE_PER_PROCESS > MAX_RUNNABLE_NUMBER,
              "Invalid configuration for runnables");

enum class Interfaces : uint8_t
{
    INTERNAL = 0,
    ESOC,
    SOMEIP,
    AMQP,
    DDS,
    SIGNAL,
    MTA,
    INTERFACE_END
};

constexpr const char* INTERFACE_NAMES[] = {
    "INTERNAL", "ESOC", "SOMEIP", "AMQP", "DDS", "SIGNAL", "MTA", "Interface End"};

enum class SubscribeState : uint32_t
{
    NOT_SUBSCRIBED = 0,
    SUBSCRIBE_REQUESTED,
    SUBSCRIBED,
    UNSUBSCRIBE_REQUESTED,
    WAIT_FOR_OFFER
};
} // namespace iox
