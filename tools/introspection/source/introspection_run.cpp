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

#include "iceoryx_introspection/introspection_run.hpp"
#include "iceoryx_introspection/introspection_print.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <chrono>
#include <ctime>
#include <deque>
#include <thread>

namespace iox
{
namespace client
{
namespace introspection
{
bool waitForSubscription(SubscriberType& port)
{
    int numberOfLoopsTillTimeout{100};
    bool subscribed{false};
    while ((subscribed = (port.getSubscriptionState() == iox::popo::SubscriptionState::SUBSCRIBED)),
           !subscribed && numberOfLoopsTillTimeout > 0)
    {
        numberOfLoopsTillTimeout--;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return subscribed;
}

std::vector<ComposedSenderPortData> composeSenderPortData(const PortIntrospectionFieldTopic* portData,
                                                          const PortThroughputIntrospectionFieldTopic* throughputData)
{
    std::vector<ComposedSenderPortData> senderPortData;
    senderPortData.reserve(portData->m_senderList.size());

    const PortThroughputData dummyThroughputData;

    auto& m_senderList = portData->m_senderList;
    auto& m_throughputList = throughputData->m_throughputList;
    const bool fastLookup = (m_senderList.size() == m_throughputList.size());
    for (uint64_t i = 0; i < m_senderList.size(); ++i)
    {
        bool found = (fastLookup && m_senderList[i].m_senderPortID == m_throughputList[i].m_senderPortID);
        if (found)
        {
            senderPortData.push_back({m_senderList[i], m_throughputList[i]});
            continue;
        }
        else
        {
            for (const auto& throughput : m_throughputList)
            {
                if (m_senderList[i].m_senderPortID == throughput.m_senderPortID)
                {
                    senderPortData.push_back({m_senderList[i], throughput});
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                senderPortData.push_back({m_senderList[i], dummyThroughputData});
            }
        }
    }

    auto senderSortCriterion = [](const ComposedSenderPortData& sender1, const ComposedSenderPortData& sender2) {
        std::string name1(sender1.portData->m_name.to_cstring());
        std::string name2(sender2.portData->m_name.to_cstring());
        return name1.compare(name2) < 0;
    };
    std::sort(senderPortData.begin(), senderPortData.end(), senderSortCriterion);

    return senderPortData;
}

std::vector<ComposedReceiverPortData>
composeReceiverPortData(const PortIntrospectionFieldTopic* portData,
                        const ReceiverPortChangingIntrospectionFieldTopic* receiverPortChangingData)
{
    std::vector<ComposedReceiverPortData> receiverPortData;
    receiverPortData.reserve(portData->m_receiverList.size());

    int i = 0;
    if (portData->m_receiverList.size() == receiverPortChangingData->receiverPortChangingDataList.size())
    { // should be the same, else it will be soon
        for (const auto& port : portData->m_receiverList)
        {
            receiverPortData.push_back(
                {port,
                 (port.m_senderIndex != -1) ? &portData->m_senderList[port.m_senderIndex] : nullptr,
                 receiverPortChangingData->receiverPortChangingDataList[i++]});
        }
    }

    auto receiverSortCriterion = [](const ComposedReceiverPortData& receiver1,
                                    const ComposedReceiverPortData& receiver2) {
        std::string name1(receiver1.portData->m_name.to_cstring());
        std::string name2(receiver2.portData->m_name.to_cstring());
        return name1.compare(name2) < 0;
    };
    std::sort(receiverPortData.begin(), receiverPortData.end(), receiverSortCriterion);

    return receiverPortData;
}

void runIntrospection(const int updatePeriodMs, const IntrospectionSelection introspectionSelection)
{
    iox::runtime::PoshRuntime::getInstance(iox::roudi::INTROSPECTION_MQ_APP_NAME);

    using namespace iox::roudi;

    initTerminal();
    prettyPrint("### Iceoryx Introspection Client ###\n\n", PrettyOptions::title);

    // mempool
    SubscriberType memPoolSubscriber(IntrospectionMempoolService);
    if (introspectionSelection.mempool == true)
    {
        memPoolSubscriber.subscribe(iox::MAX_SHM_SEGMENTS + 1);

        if (waitForSubscription(memPoolSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for mempool introspection data!\n",
                        PrettyOptions::error);
        }
    }


    // process
    SubscriberType processSubscriber(IntrospectionProcessService);
    if (introspectionSelection.process == true)
    {
        processSubscriber.subscribe(1);

        if (waitForSubscription(processSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for process introspection data!\n",
                        PrettyOptions::error);
        }
    }

    // port
    SubscriberType portSubscriber(IntrospectionPortService);

    SubscriberType portThroughputSubscriber(IntrospectionPortThroughputService);

    SubscriberType receiverPortChangingDataSubscriber(IntrospectionReceiverPortChangingDataService);

    if (introspectionSelection.port == true)
    {
        portSubscriber.subscribe(1);
        portThroughputSubscriber.subscribe(1);
        receiverPortChangingDataSubscriber.subscribe(1);

        if (waitForSubscription(portSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for port introspection data!\n", PrettyOptions::error);
        }
        if (waitForSubscription(portThroughputSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for subscription for port throughput introspection data!\n",
                        PrettyOptions::error);
        }
        if (waitForSubscription(receiverPortChangingDataSubscriber) == false)
        {
            prettyPrint("Timeout while waiting for Subscription for Receiver Port Introspection Changing Data!\n",
                        PrettyOptions::error);
        }
    }

    // Refresh once in case of timeout messages
    refreshTerminal();

    const void* rawProcessSample{nullptr};
    const ProcessIntrospectionFieldTopic* typedProcessSample{nullptr};

    const void* rawPortSample{nullptr};
    const PortIntrospectionFieldTopic* typedPortSample{nullptr};

    const void* rawPortThroughputSample{nullptr};
    const PortThroughputIntrospectionFieldTopic* typedPortThroughputSample{nullptr};

    const void* rawReceiverPortChangingDataSamples{nullptr};
    const ReceiverPortChangingIntrospectionFieldTopic* typedReceiverPortChangingDataSamples{nullptr};

    while (true)
    {
        // get and print time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        char timeBuf[128];
        strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %X", std::localtime(&in_time_t));
        prettyPrint("### Iceoryx Introspection Client ### ", PrettyOptions::title);
        prettyPrint(timeBuf, PrettyOptions::bold);
        prettyPrint("\n\n", PrettyOptions::bold);

        // print mempool information
        if (introspectionSelection.mempool == true)
        {
            prettyPrint("### MemPool Status ###\n\n", PrettyOptions::highlight);

            std::deque<const MemPoolIntrospectionTopic*> mempoolSamples;
            const void* rawMempoolSample{nullptr};


            while (memPoolSubscriber.getChunk(&rawMempoolSample))
            {
                decltype(mempoolSamples)::value_type typedMempoolSample =
                    static_cast<decltype(mempoolSamples)::value_type>(rawMempoolSample);
                mempoolSamples.push_back(std::move(typedMempoolSample));
                memPoolSubscriber.releaseChunk(rawMempoolSample);
            }

            if (mempoolSamples.empty())
            {
                prettyPrint("Waiting for mempool introspection data ...\n");
            }
            else
            {
                auto it = mempoolSamples.end() - 1;
                auto currentIter = it;
                auto itBegin = mempoolSamples.begin();

                uint32_t lastId = (*it)->m_id;

                while (it != itBegin)
                {
                    --it;
                    if ((*it)->m_id == lastId)
                    {
                        break;
                    }
                    currentIter = it;
                }

                std::for_each(
                    currentIter, mempoolSamples.end(), [](decltype(*it) samplePtr) { printMemPoolInfo(*samplePtr); });
            }
        }

        // print process information
        if (introspectionSelection.process == true)
        {
            prettyPrint("### Processes ###\n\n", PrettyOptions::highlight);
            if (!processSubscriber.hasNewChunks())
            {
                // No new data sent, hence print the old data
                if (typedProcessSample != nullptr)
                {
                    printProcessIntrospectionData(typedProcessSample);
                }
                else
                {
                    prettyPrint("Waiting for process introspection data ...\n");
                }
            }
            else
            {
                if (processSubscriber.getChunk(&rawProcessSample))
                {
                    typedProcessSample = static_cast<const ProcessIntrospectionFieldTopic*>(rawProcessSample);
                    printProcessIntrospectionData(typedProcessSample);
                    processSubscriber.releaseChunk(rawProcessSample);
                }
            }
        }

        // print port information
        if (introspectionSelection.port == true)
        {
            bool newPortSampleArrived{false};
            bool newPortThroughputSampleeArrived{false};
            bool newReceiverPortChangingDataSamplesArrived{false};

            if (portSubscriber.getChunk(&rawPortSample))
            {
                typedPortSample = static_cast<const PortIntrospectionFieldTopic*>(rawPortSample);
                newPortSampleArrived = true;
            }
            if (portThroughputSubscriber.getChunk(&rawPortThroughputSample))
            {
                typedPortThroughputSample =
                    static_cast<const PortThroughputIntrospectionFieldTopic*>(rawPortThroughputSample);
                newPortThroughputSampleeArrived = true;
            }
            if (receiverPortChangingDataSubscriber.getChunk(&rawReceiverPortChangingDataSamples))
            {
                typedReceiverPortChangingDataSamples =
                    static_cast<const ReceiverPortChangingIntrospectionFieldTopic*>(rawReceiverPortChangingDataSamples);
                newReceiverPortChangingDataSamplesArrived = true;
            }

            if (typedPortSample != nullptr && typedPortThroughputSample != nullptr
                && typedReceiverPortChangingDataSamples != nullptr)
            {
                prettyPrint("### Connections ###\n\n", PrettyOptions::highlight);

                auto composedSenderPortData = composeSenderPortData(typedPortSample, typedPortThroughputSample);
                auto composedReceiverPortData =
                    composeReceiverPortData(typedPortSample, typedReceiverPortChangingDataSamples);

                printPortIntrospectionData(composedSenderPortData, composedReceiverPortData);
            }
            else
            {
                prettyPrint("Waiting for port introspection data ...\n");
            }

            if (newPortSampleArrived)
            {
                portSubscriber.releaseChunk(rawPortSample);
            }
            if (newPortThroughputSampleeArrived)
            {
                portThroughputSubscriber.releaseChunk(rawPortThroughputSample);
            }
            if (newReceiverPortChangingDataSamplesArrived)
            {
                receiverPortChangingDataSubscriber.releaseChunk(rawReceiverPortChangingDataSamples);
            }
        }

        prettyPrint("\n");
        clearToBottom();
        refreshTerminal();

        // Watch user input for updatePeriodMs
        auto tWaitRemaining = std::chrono::milliseconds(updatePeriodMs);
        auto tWaitBegin = std::chrono::system_clock::now();
        while (tWaitRemaining.count() >= 0)
        {
            waitForUserInput(static_cast<int32_t>(tWaitRemaining.count()));
            auto tWaitElapsed = std::chrono::system_clock::now() - tWaitBegin;
            tWaitRemaining = std::chrono::milliseconds(updatePeriodMs)
                             - std::chrono::duration_cast<std::chrono::milliseconds>(tWaitElapsed);
        }
    }

    getchar();
    closeTerminal();
}

} // namespace introspection
} // namespace client
} // namespace iox
