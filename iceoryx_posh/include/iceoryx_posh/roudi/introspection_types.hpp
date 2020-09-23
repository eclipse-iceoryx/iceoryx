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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_TYPES_HPP
#define IOX_POSH_ROUDI_INTROSPECTION_TYPES_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

namespace iox
{
namespace roudi
{
constexpr const char INTROSPECTION_SERVICE_ID[] = "Introspection";
constexpr const char INTROSPECTION_MQ_APP_NAME[] = "/introspection";
const capro::ServiceDescription IntrospectionMempoolService(INTROSPECTION_SERVICE_ID, "RouDi_ID", "MemPool");
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
struct MemPoolIntrospectionInfo
{
    uint32_t m_id;
    char m_writerGroupName[MAX_GROUP_NAME_LENGTH];
    char m_readerGroupName[MAX_GROUP_NAME_LENGTH];
    MemPoolInfoContainer m_mempoolInfo;
};

/// @brief container for MemPoolInfo structs of all available mempools.
using MemPoolIntrospectionInfoContainer = cxx::vector<MemPoolIntrospectionInfo, MAX_SHM_SEGMENTS + 1>;

/// @brief sender/receiver port information consisting of a process name,a capro service description string
/// and a runnable name
const capro::ServiceDescription IntrospectionPortService(INTROSPECTION_SERVICE_ID, "RouDi_ID", "Port");
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
    iox::capro::Interfaces m_sourceInterface{iox::capro::Interfaces::INTERFACE_END};
};

/// @brief the topic for the port introspection that a user can subscribe to
struct PortIntrospectionFieldTopic
{
    cxx::vector<ReceiverPortData, MAX_SUBSCRIBERS> m_receiverList;
    cxx::vector<SenderPortData, MAX_PUBLISHERS> m_senderList;
};

const capro::ServiceDescription
    IntrospectionPortThroughputService(INTROSPECTION_SERVICE_ID, "RouDi_ID", "PortThroughput");

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
    cxx::vector<PortThroughputData, MAX_PUBLISHERS> m_throughputList;
};

const capro::ServiceDescription
    IntrospectionReceiverPortChangingDataService(INTROSPECTION_SERVICE_ID, "RouDi_ID", "ReceiverPortsData");

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
    cxx::vector<ReceiverPortChangingData, MAX_SUBSCRIBERS> receiverPortChangingDataList;
};

const capro::ServiceDescription IntrospectionProcessService(INTROSPECTION_SERVICE_ID, "RouDi_ID", "Process");

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

#endif // IOX_POSH_ROUDI_INTROSPECTION_TYPES_HPP
