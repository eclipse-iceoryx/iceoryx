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
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender_data.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

using namespace ::testing;

struct DummySample
{
    uint64_t dummy{42};
};

class ChunkSender_testBase : public Test
{
  protected:
    ChunkSender_testBase()
    {
        m_mempoolconf.addMemPool({128, 20});
        m_mempoolconf.addMemPool({256, 20});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~ChunkSender_testBase()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    void ReceiveDummyData()
    {
        // // Be sure to receive the chunk we just sent to be able to recycle it
        // const iox::mepoo::ChunkHeader* receivedSample1;
        // m_receiver->getChunk(receivedSample1);
        // m_receiver->releaseChunk(receivedSample1);
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[1024 * 1024];
    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::ChunkSenderData m_chunkSenderData{&m_memoryManager, 0}; // history is assumed to be 0 in tests
    iox::popo::ChunkSender m_chunkSender{&m_chunkSenderData};
    static constexpr uint64_t HISTORY_CAPACITY = 4;
    iox::popo::ChunkSenderData m_chunkSenderDataWithHistory{&m_memoryManager, HISTORY_CAPACITY}; 
    iox::popo::ChunkSender m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};    
};

class ChunkSender_test : public ChunkSender_testBase
{
  public:
    ChunkSender_test()
        : ChunkSender_testBase()
    {
    }
};

TEST_F(ChunkSender_test, allocate_OneChunk)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, allocate_MultipleChunks)
{
    auto chunk1 = m_chunkSender.allocate(sizeof(DummySample));
    auto chunk2 = m_chunkSender.allocate(sizeof(DummySample));

    EXPECT_FALSE(chunk1.has_error());
    EXPECT_FALSE(chunk2.has_error());
    if (!chunk1.has_error() && !chunk2.has_error())
    {
        // must be different chunks
        EXPECT_THAT(*chunk1, Ne(*chunk2));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2u));
}

TEST_F(ChunkSender_test, allocate_Overflow)
{
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // allocate chunks until MAX_CHUNKS_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        if (!chunk.has_error())
        {
            chunks.push_back(*chunk);
        }
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        EXPECT_THAT(chunks[i], Ne(nullptr));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // Allocate one more sample for overflow
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_TRUE(chunk.has_error());
    EXPECT_THAT(chunk.get_error(), Eq(iox::popo::ChunkSenderError::TOO_MANY_CHUKS_ALLOCATED_IN_PARALLEL));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));
}

TEST_F(ChunkSender_test, freeChunk)
{
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // allocate chunks until MAX_CHUNKS_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        if (!chunk.has_error())
        {
            chunks.push_back(*chunk);
        }
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // free them all
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        m_chunkSender.free(chunks[i]);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(ChunkSender_test, freeInvalidChunk)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    iox::mepoo::ChunkHeader* myCrazyChunk = new iox::mepoo::ChunkHeader();
    m_chunkSender.free(myCrazyChunk);

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    delete myCrazyChunk;
}

TEST_F(ChunkSender_test, sendWithoutReceiver)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!chunk.has_error())
    {
        auto sample = *chunk;
        new (sample) DummySample();
        m_chunkSender.send(sample);
        // chunk is still used because last chunk is stored
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    }
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverAndAlwaysLast)
{
    for (size_t i = 0; i < 100; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        auto lastChunk = m_chunkSender.getLastChunk();
        if(i > 0)
        {
            EXPECT_TRUE(lastChunk.has_value());
            EXPECT_TRUE(*chunk == *lastChunk);
        }
        else
        {
            EXPECT_FALSE(lastChunk.has_value());
        }
        auto sample = *chunk;
        new (sample) DummySample();
        m_chunkSender.send(sample);
    }

    // Exactly one chunk is used because last chunk is stored
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

}

TEST_F(ChunkSender_test, sendOneWithReceiver)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!chunk.has_error())
    {
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*chunk);

        // consume the sample
        {
            iox::popo::ChunkQueue myQueue(&m_chunkQueueData);
            EXPECT_FALSE(myQueue.empty());
            auto popRet = myQueue.pop();
            EXPECT_TRUE(popRet.has_value());
            auto dummySample = *reinterpret_cast<DummySample*>(popRet->getPayload());
            EXPECT_THAT(dummySample.dummy, Eq(42));
        }
    }
}

// TEST_F(ChunkSender_test, reserveSample_DynamicSamplesSameSizeReturningValidLastChunk)
// {
//     auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample1);

//     ReceiveDummyData();

//     // Do it again to see whether the same chunk is returned
//     auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample2);
//     EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample)));
//     EXPECT_THAT(sentSample2->payload(), Eq(sentSample1->payload()));
// }


// TEST_F(ChunkSender_test, reserveSample_DynamicSamplesSmallerSizeReturningValidLastChunk)
// {
//     auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample1);

//     ReceiveDummyData();

//     // Reserve a smaller chunk to see whether the same chunk is returned
//     auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample) - 7, m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample2);
//     EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample) - 7));
//     EXPECT_THAT(sentSample2->payload(), Eq(sentSample1->payload()));
// }

// TEST_F(ChunkSender_test, reserveSample_DynamicSamplesLargerSizeReturningNotLastChunk)
// {
//     auto sentSample1 = m_sender->reserveChunk(sizeof(DummySample), m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample1);

//     ReceiveDummyData();

//     // Reserve a larger chunk to see whether a chunk of the larger mempool is supplied
//     auto sentSample2 = m_sender->reserveChunk(sizeof(DummySample) + 200, m_useDynamicPayloadSizes);
//     m_sender->deliverChunk(sentSample2);
//     EXPECT_THAT(sentSample2->m_info.m_payloadSize, Eq(sizeof(DummySample) + 200));
//     EXPECT_THAT(sentSample2->payload(), Ne(sentSample1->payload()));
// }


// TEST_F(ChunkSender_test, doNotDeliverDataOnSubscription)
// {
//     EXPECT_THAT(m_receiver->newData(), Eq(false));
// }

// TEST_F(ChunkSender_test, deliverSample_OneSample)
// {
//     auto sample = m_sender->reserveChunk(sizeof(DummySample));

//     new (sample) DummySample();
//     sample->m_info.m_payloadSize = sizeof(DummySample);
//     sample->m_info.m_externalSequenceNumber_bl = true;
//     sample->m_info.m_sequenceNumber = 1337;
//     m_sender->deliverChunk(sample);

//     ASSERT_THAT(m_receiver->newData(), Eq(true));
//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(1337u));
// }

// TEST_F(ChunkSender_test, deliverSample_MultipleSample)
// {
//     auto sample1 = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample1->payload()) DummySample();
//     sample1->m_info.m_payloadSize = sizeof(DummySample);
//     sample1->m_info.m_externalSequenceNumber_bl = true;
//     sample1->m_info.m_sequenceNumber = 14337;
//     m_sender->deliverChunk(sample1);

//     auto sample2 = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample2->payload()) DummySample();
//     sample2->m_info.m_payloadSize = sizeof(DummySample);
//     sample2->m_info.m_externalSequenceNumber_bl = true;
//     sample2->m_info.m_sequenceNumber = 42u;
//     m_sender->deliverChunk(sample2);


//     ASSERT_THAT(m_receiver->newData(), Eq(true));
//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(14337u));

//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(m_receiver->releaseChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(42u));
// }

// TEST_F(ChunkSender_test, DISABLED_doDeliverOnSubscription_InitialValue)
// {
//     ServiceDescription l_service2{2, 2, 2};
//     auto m_sender2 = CreateSender(l_service2);
//     m_sender2->enableDoDeliverOnSubscription();

//     auto latestValue = m_sender2->reserveChunk(sizeof(DummySample));
//     latestValue->m_info.m_externalSequenceNumber_bl = true;
//     latestValue->m_info.m_sequenceNumber = 4711;
//     m_sender2->deliverChunk(latestValue);

//     auto m_receiver2 = CreateReceiver(m_service);
//     SubscribeReceiverToSender(m_receiver2, m_sender2);

//     ASSERT_THAT(m_receiver2->newData(), Eq(true));
//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(4711u));
//     m_receiver2->releaseChunk(receivedSample);
// }

// TEST_F(ChunkSender_test, doDeliverOnSubscription_LatestValue)
// {
//     m_sender->enableDoDeliverOnSubscription();

//     auto latestValue = m_sender->reserveChunk(sizeof(DummySample));
//     latestValue->m_info.m_externalSequenceNumber_bl = true;
//     latestValue->m_info.m_sequenceNumber = 41112;
//     m_sender->deliverChunk(latestValue);

//     auto m_receiver2 = CreateReceiver(m_service);
//     SubscribeReceiverToSender(m_receiver2, m_sender);


//     EXPECT_THAT(m_sender->isPortActive(), Eq(true));
//     ASSERT_THAT(m_receiver2->newData(), Eq(true));
//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(41112u));
//     m_receiver2->releaseChunk(latestValue);
// }

// TEST_F(ChunkSender_test, testCaPro)
// {
//     m_sender->enableDoDeliverOnSubscription();

//     auto latestValue = m_sender->reserveChunk(sizeof(DummySample));
//     latestValue->m_info.m_externalSequenceNumber_bl = true;
//     latestValue->m_info.m_sequenceNumber = 47112;
//     m_sender->deliverChunk(latestValue);

//     auto m_receiver2 = CreateReceiver(m_service);
//     SubscribeReceiverToSender(m_receiver2, m_sender);


//     EXPECT_THAT(m_sender->isPortActive(), Eq(true));
//     ASSERT_THAT(m_receiver2->newData(), Eq(true));
//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver2->getChunk(receivedSample), Eq(true));
//     ASSERT_THAT(receivedSample->m_info.m_sequenceNumber, Eq(47112u));
//     m_receiver2->releaseChunk(receivedSample);
// }

// TEST_F(ChunkSender_testLatchedTopic, getSameSampleAfterOneDeliver)
// {
//     auto sample = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample) DummySample();
//     sample->m_info.m_payloadSize = sizeof(DummySample);
//     m_sender->deliverChunk(sample);


//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     m_receiver->releaseChunk(receivedSample);

//     uint64_t sampleAddress = reinterpret_cast<uint64_t>(sample);
//     EXPECT_THAT(reinterpret_cast<uint64_t>(m_sender->reserveChunk(sizeof(DummySample))), Eq(sampleAddress));
// }

// TEST_F(ChunkSender_testLatchedTopic, getDifferentSampleWhenStillInUse)
// {
//     auto sample = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample) DummySample();
//     sample->m_info.m_payloadSize = sizeof(DummySample);
//     m_sender->deliverChunk(sample);

//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));

//     uint64_t sampleAddress = reinterpret_cast<uint64_t>(sample);
//     EXPECT_THAT(reinterpret_cast<uint64_t>(m_sender->reserveChunk(sizeof(DummySample))), Ne(sampleAddress));
//     m_receiver->releaseChunk(receivedSample);
// }

// TEST_F(ChunkSender_testLatchedTopic, getSameSampleAfterSecondDelivery)
// {
//     auto sample = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample) DummySample();
//     sample->m_info.m_payloadSize = sizeof(DummySample);
//     m_sender->deliverChunk(sample);

//     sample = m_sender->reserveChunk(sizeof(DummySample));
//     new (sample) DummySample();
//     sample->m_info.m_payloadSize = sizeof(DummySample);
//     m_sender->deliverChunk(sample);

//     const iox::mepoo::ChunkHeader* receivedSample;
//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     m_receiver->releaseChunk(receivedSample);

//     ASSERT_THAT(m_receiver->getChunk(receivedSample), Eq(true));
//     m_receiver->releaseChunk(receivedSample);

//     uint64_t sampleAddress = reinterpret_cast<uint64_t>(sample);
//     EXPECT_THAT(reinterpret_cast<uint64_t>(m_sender->reserveChunk(sizeof(DummySample))), Eq(sampleAddress));
// }
