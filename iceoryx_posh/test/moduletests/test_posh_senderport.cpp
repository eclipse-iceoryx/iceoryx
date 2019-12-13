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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::popo;
using namespace iox::capro;
using iox::mepoo::ChunkHeader;
using iox::mepoo::ChunkManagement;

struct DummySample
{
    uint64_t dummy{42};
};

class SenderPort_testBase : public Test
{
  protected:
    SenderPort_testBase(const bool f_hasLatchedTopic)
        : m_hasLatchedTopic(f_hasLatchedTopic)
        , m_memoryAllocator(m_memory, 1024 * 1024)
    {
        ActivateSender(m_sender);

        mempoolconf.addMemPool({128, 20});
        mempoolconf.addMemPool({256, 20});
        m_memPoolHandler.configureMemoryManager(mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
        SubscribeReceiverToSender(m_receiver, m_sender);
    }

    ~SenderPort_testBase()
    {
        for (auto port : m_ports)
        {
            delete port;
        }
        for (auto member : m_portData)
        {
            delete member;
        }
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    void SubscribeReceiverToSender(iox::ReceiverPortType* f_receiver, iox::SenderPortType* f_sender)
    {
        /// send subscription request to roudiPort
        f_receiver->subscribe(true, 10);

        auto returnedCaproMessage = f_receiver->getCaProMessage();
        if (returnedCaproMessage.has_value())
        {
            auto senderResponse = f_sender->dispatchCaProMessage(returnedCaproMessage.value());
        }
    }

    iox::ReceiverPortType* CreateReceiver(const ServiceDescription& f_service)
    {
        iox::ReceiverPortType::MemberType_t* data =
            new iox::ReceiverPortType::MemberType_t(f_service, "", iox::Interfaces::INTERNAL, nullptr);
        m_portData.emplace_back(data);
        iox::ReceiverPortType* l_receiver = new iox::ReceiverPortType(data);
        m_ports.emplace_back(l_receiver);
        return l_receiver;
    }

    iox::SenderPortType* CreateSender(const ServiceDescription& f_service)
    {
        iox::SenderPortType::MemberType_t* data =
            new iox::SenderPortType::MemberType_t(f_service, &m_memPoolHandler, "", iox::Interfaces::INTERNAL, nullptr);
        m_portData.emplace_back(data);
        iox::SenderPortType* l_sender = new iox::SenderPortType(data);
        m_ports.emplace_back(l_sender);

        return l_sender;
    }

    void ActivateSender(iox::SenderPortType* const f_sender)
    {
        f_sender->activate();
        CaproMessage expect_offer_msg = {iox::capro::CaproMessageType::OFFER, m_service};

        auto returnedCaproMessage = f_sender->getCaProMessage();
        EXPECT_THAT(returnedCaproMessage.has_value(), Eq(true));
        if (returnedCaproMessage.has_value())
        {
            auto msg = returnedCaproMessage.value();
            EXPECT_THAT(msg.m_type, Eq(expect_offer_msg.m_type));
        }
    }

    void ReceiveDummyData()
    {
        // Be sure to receive the chunk we just sent to be able to recycle it
        const iox::mepoo::ChunkHeader* receivedSample1;
        m_receiver->getChunk(receivedSample1);
        m_receiver->releaseChunk(receivedSample1);
    }

    bool m_hasLatchedTopic;
    char m_memory[1024 * 1024];
    bool m_useDynamicPayloadSizes = true;
    std::vector<BasePortData*> m_portData;
    std::vector<BasePort*> m_ports;
    iox::posix::Allocator m_memoryAllocator;
    iox::mepoo::MemoryManager m_memPoolHandler;
    ServiceDescription m_service{1, 1, 1};
    iox::SenderPortType* m_sender = CreateSender(m_service);
    iox::ReceiverPortType* m_receiver = CreateReceiver(m_service);
    iox::mepoo::MePooConfig mempoolconf;
};

class SenderPort_test : public SenderPort_testBase
{
  public:
    SenderPort_test()
        : SenderPort_testBase(false)
    {
    }
};

class SenderPort_testLatchedTopic : public SenderPort_testBase
{
  public:
    SenderPort_testLatchedTopic()
        : SenderPort_testBase(true)
    {
    }
};


TEST_F(SenderPort_test, noSamplesUsedOnStartup)
{
    printf("1\n");
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(0u));
    printf("2\n");
}

TEST_F(SenderPort_test, reserveSample_OneSample)
{
    printf("3\n");
    auto sample = m_sender->reserveChunk(sizeof(DummySample));
    EXPECT_THAT(sample, Ne(nullptr));
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(SenderPort_test, reserveSample_MultipleSamples)
{
    auto sample1 = m_sender->reserveChunk(sizeof(DummySample));
    auto sample2 = m_sender->reserveChunk(sizeof(DummySample));

    EXPECT_THAT(sample1, Ne(nullptr));
    EXPECT_THAT(sample2, Ne(nullptr));
    EXPECT_THAT(sample1, Ne(sample2));
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(2u));
}

TEST_F(SenderPort_test, reserveSample_DynamicSamplesSameSizeReturningValidLastChunk)
{
    auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample1);

    ReceiveDummyData();

    // Do it again to see whether the same chunk is returned
    auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample2);
    EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample)));
    EXPECT_THAT(sentSample2->payload(), Eq(sentSample1->payload()));
}

TEST_F(SenderPort_test, reserveSample_DynamicSamplesSmallerSizeReturningValidLastChunk)
{
    auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample1);

    ReceiveDummyData();

    // Reserve a smaller chunk to see whether the same chunk is returned
    auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample) - 7, m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample2);
    EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample) - 7));
    EXPECT_THAT(sentSample2->payload(), Eq(sentSample1->payload()));
}

TEST_F(SenderPort_test, reserveSample_DynamicSamplesLargerSizeReturningNotLastChunk)
{
    auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample1);

    ReceiveDummyData();

    // Reserve a larger chunk to see whether a chunk of the larger mempool is supplied
    auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample) + 200, m_useDynamicPayloadSizes);
    m_sender->deliverChunk(sentSample2);
    EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample) + 200));
    EXPECT_THAT(sentSample2->payload(), Ne(sentSample1->payload()));
}

TEST_F(SenderPort_test, reserveSample_Overflow)
{
    std::vector<ChunkHeader*> samples;

    // allocate samples until MAX_SAMPLE_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_SAMPLE_ALLOCATE_PER_SENDER; i++)
    {
        samples.push_back(m_sender->reserveChunk(sizeof(DummySample)));
    }

    for (size_t i = 0; i < iox::MAX_SAMPLE_ALLOCATE_PER_SENDER; i++)
    {
        EXPECT_THAT(samples[i], Ne(nullptr));
    }
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(16u));

// Allocate one more sample for overflow
#if defined(NDEBUG)
    auto sample = m_sender->reserveChunk(sizeof(DummySample));
    EXPECT_EQ(sample, nullptr);
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(16u));
#else
    ASSERT_DEATH({ m_sender->reserveChunk(sizeof(DummySample)); }, "Application allocates too much chunks");
    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(16u));
#endif
}

TEST_F(SenderPort_test, freeChunk)
{
    auto sample = m_sender->reserveChunk(sizeof(DummySample));

    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    m_sender->freeChunk(sample);

    EXPECT_THAT(m_memPoolHandler.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(SenderPort_test, doNotDeliverDataOnSubscription)
{
    EXPECT_THAT(m_receiver->newData(), Eq(false));
}

TEST_F(SenderPort_test, deliverSample_OneSample)
{
    auto sample = m_sender->reserveChunk(sizeof(DummySample));

    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    sample->m_info.m_externalSequenceNumber_bl = true;
    sample->m_info.m_sequenceNumber = 1337;
    m_sender->deliverChunk(sample);

    ASSERT_THAT(m_receiver->newData(), Eq(true));
    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(1337u));
}

TEST_F(SenderPort_test, deliverSample_MultipleSample)
{
    auto sample1 = m_sender->reserveChunk(sizeof(DummySample));
    new (sample1->payload()) DummySample();
    sample1->m_info.m_payloadSize = sizeof(DummySample);
    sample1->m_info.m_externalSequenceNumber_bl = true;
    sample1->m_info.m_sequenceNumber = 14337;
    m_sender->deliverChunk(sample1);

    auto sample2 = m_sender->reserveChunk(sizeof(DummySample));
    new (sample2->payload()) DummySample();
    sample2->m_info.m_payloadSize = sizeof(DummySample);
    sample2->m_info.m_externalSequenceNumber_bl = true;
    sample2->m_info.m_sequenceNumber = 42u;
    m_sender->deliverChunk(sample2);


    ASSERT_THAT(m_receiver->newData(), Eq(true));
    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(14337u));

    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(42u));
}

TEST_F(SenderPort_test, DISABLED_doDeliverOnSubscription_InitialValue)
{
    ServiceDescription l_service2{2, 2, 2};
    auto m_sender2 = CreateSender(l_service2);
    m_sender2->enableDoDeliverOnSubscription();

    auto latestValue = m_sender2->reserveChunk(sizeof(DummySample));
    latestValue->m_info.m_externalSequenceNumber_bl = true;
    latestValue->m_info.m_sequenceNumber = 4711;
    m_sender2->deliverChunk(latestValue);

    auto m_receiver2 = CreateReceiver(m_service);
    SubscribeReceiverToSender(m_receiver2, m_sender2);

    ASSERT_THAT(m_receiver2->newData(), Eq(true));
    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(4711u));
    m_receiver2->releaseChunk(receivedSample);
}

TEST_F(SenderPort_test, doDeliverOnSubscription_LatestValue)
{
    m_sender->enableDoDeliverOnSubscription();

    auto latestValue = m_sender->reserveChunk(sizeof(DummySample));
    latestValue->m_info.m_externalSequenceNumber_bl = true;
    latestValue->m_info.m_sequenceNumber = 41112;
    m_sender->deliverChunk(latestValue);

    auto m_receiver2 = CreateReceiver(m_service);
    SubscribeReceiverToSender(m_receiver2, m_sender);


    EXPECT_THAT(m_sender->isPortActive(), Eq(true));
    ASSERT_THAT(m_receiver2->newData(), Eq(true));
    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(41112u));
    m_receiver2->releaseChunk(latestValue);
}

TEST_F(SenderPort_test, testCaPro)
{
    m_sender->enableDoDeliverOnSubscription();

    auto latestValue = m_sender->reserveChunk(sizeof(DummySample));
    latestValue->m_info.m_externalSequenceNumber_bl = true;
    latestValue->m_info.m_sequenceNumber = 47112;
    m_sender->deliverChunk(latestValue);

    auto m_receiver2 = CreateReceiver(m_service);
    SubscribeReceiverToSender(m_receiver2, m_sender);


    EXPECT_THAT(m_sender->isPortActive(), Eq(true));
    ASSERT_THAT(m_receiver2->newData(), Eq(true));
    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
    ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(47112u));
    m_receiver2->releaseChunk(receivedSample);
}

TEST_F(SenderPort_testLatchedTopic, getSameSampleAfterOneDeliver)
{
    auto sample = m_sender->reserveChunk(sizeof(DummySample));
    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    m_sender->deliverChunk(sample);


    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    m_receiver->releaseChunk(receivedSample);

    uintptr_t sampleAddress = reinterpret_cast<uintptr_t>(sample);
    EXPECT_THAT(reinterpret_cast<uintptr_t>(m_sender->reserveChunk(sizeof(DummySample))), Eq(sampleAddress));
}

TEST_F(SenderPort_testLatchedTopic, getDifferentSampleWhenStillInUse)
{
    auto sample = m_sender->reserveChunk(sizeof(DummySample));
    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    m_sender->deliverChunk(sample);

    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));

    uintptr_t sampleAddress = reinterpret_cast<uintptr_t>(sample);
    EXPECT_THAT(reinterpret_cast<uintptr_t>(m_sender->reserveChunk(sizeof(DummySample))), Ne(sampleAddress));
    m_receiver->releaseChunk(receivedSample);
}

TEST_F(SenderPort_testLatchedTopic, getSameSampleAfterSecondDelivery)
{
    auto sample = m_sender->reserveChunk(sizeof(DummySample));
    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    m_sender->deliverChunk(sample);

    sample = m_sender->reserveChunk(sizeof(DummySample));
    new (sample) DummySample();
    sample->m_info.m_payloadSize = sizeof(DummySample);
    m_sender->deliverChunk(sample);

    const iox::mepoo::ChunkHeader* receivedSample;
    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    m_receiver->releaseChunk(receivedSample);

    ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
    m_receiver->releaseChunk(receivedSample);

    uintptr_t sampleAddress = reinterpret_cast<uintptr_t>(sample);
    EXPECT_THAT(reinterpret_cast<uintptr_t>(m_sender->reserveChunk(sizeof(DummySample))), Eq(sampleAddress));
}
