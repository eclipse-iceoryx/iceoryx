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
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

namespace iox
{
namespace roudi
{
constexpr char INTROSPECTION_MQ_APP_NAME[] = "/introspection";
constexpr char MEMPOOL_INTROSPECTION_MQ_APP_NAME[] = "/mempool_introspection";
const capro::ServiceDescription IntrospectionMempoolService("Introspection", "RouDi_ID", "MemPool");
constexpr int MAX_GROUP_NAME_LENGTH = 32;

/// @brief struct for the storage of mempool usage information.
/// This data container is used by the introstpection::MemPoolInfoContainer array
/// to store information on all available memmpools.
struct MemPoolInfo
{
    uint32_t m_usedChunks{0};
    uint32_t m_minFreeChunks{0};
    uint32_t m_numChunks{0};
    uint32_t m_chunkSize{0};
    uint32_t m_payloadSize{0};
};

/// @brief container for MemPoolInfo structs of all available mempools.
using MemPoolInfoContainer = cxx::vector<MemPoolInfo, MAX_NUMBER_OF_MEMPOOLS>;

/// @brief the topic for the mempool introspection that a user can subscribe to
struct MemPoolIntrospectionTopic
{
    uint32_t m_id;
    char m_writerGroupName[MAX_GROUP_NAME_LENGTH];
    char m_readerGroupName[MAX_GROUP_NAME_LENGTH];
    MemPoolInfoContainer m_mempoolInfo;
};

constexpr char PORT_INTROSPECTION_MQ_APP_NAME[] = "/port_introspection";
const capro::ServiceDescription IntrospectionPortService("Introspection", "RouDi_ID", "Port");

/// @brief sender/receiver port information consisting of a process name,a capro service description string
/// and a runnable name
/// @todo if future fixed string is aligned to 8 byte, the alignment here can be removed
struct PortData
{
    alignas(8) cxx::CString100 m_name;
    alignas(8) cxx::CString100 m_caproInstanceID;
    alignas(8) cxx::CString100 m_caproServiceID;
    alignas(8) cxx::CString100 m_caproEventMethodID;
    alignas(8) cxx::CString100 m_runnable;
};


/// @brief container for receiver port introspection data.
struct ReceiverPortData : public PortData
{
    /// @brief identifier of the sender port to which this receiver port is connected.
    /// If no matching sender port exists, this should equal -1.
    int32_t m_senderIndex{-1};
};

/// @brief container for sender port introspection data.
struct SenderPortData : public PortData
{
    uint64_t m_senderPortID{0};
};

/// @brief the topic for the port introspection that a user can subscribe to
struct PortIntrospectionFieldTopic
{
    cxx::vector<ReceiverPortData, MAX_PORT_NUMBER> m_receiverList;
    cxx::vector<SenderPortData, MAX_PORT_NUMBER> m_senderList;
};

constexpr char PORT_THROUGHPUT_INTROSPECTION_MQ_APP_NAME[] = "/portThroughput_introspection";
const capro::ServiceDescription IntrospectionPortThroughputService("Introspection", "RouDi_ID", "PortThroughput");

struct PortThroughputData
{
    uint64_t m_senderPortID{0};
    uint32_t m_sampleSize{0};
    uint32_t m_chunkSize{0};
    double m_chunksPerMinute{0};
    uint64_t m_lastSendIntervalInNanoseconds{0};
    bool m_isField{false};
};

/// @brief the topic for the port throughput that a user can subscribe to
struct PortThroughputIntrospectionFieldTopic
{
    cxx::vector<PortThroughputData, MAX_PORT_NUMBER> m_throughputList;
};

const capro::ServiceDescription
    IntrospectionReceiverPortChangingDataService("Introspection", "RouDi_ID", "ReceiverPortsData");

struct ReceiverPortChangingData
{
    // index used to identify receiver is same as in PortIntrospectionFieldTopic->receiverList
    uint64_t fifoSize{0};
    uint64_t fifoCapacity{0};
    iox::SubscribeState subscriptionState{iox::SubscribeState::NOT_SUBSCRIBED};
    bool sampleSendCallbackActive{false};
    capro::Scope propagationScope{capro::Scope::INVALID};
};

struct ReceiverPortChangingIntrospectionFieldTopic
{
    cxx::vector<ReceiverPortChangingData, MAX_PORT_NUMBER> receiverPortChangingDataList;
};

constexpr char PROCESS_INTROSPECTION_MQ_APP_NAME[] = "/process_introspection";
const capro::ServiceDescription IntrospectionProcessService("Introspection", "RouDi_ID", "Process");

struct ProcessIntrospectionData
{
    int m_pid{0};
    cxx::CString100 m_name;
    cxx::vector<iox::cxx::CString100, MAX_RUNNABLE_PER_PROCESS> m_runnables;
};

/// @brief the topic for the process introspection that a user can subscribe to
struct ProcessIntrospectionFieldTopic
{
    cxx::vector<ProcessIntrospectionData, MAX_PROCESS_NUMBER> m_processList;
};

} // namespace roudi
} // namespace iox
