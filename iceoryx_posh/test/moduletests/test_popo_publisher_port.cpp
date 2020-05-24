// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"

#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;

struct DummySample
{
    uint64_t dummy{42};
};

class PublisherPort_test : public Test
{
  protected:
    PublisherPort_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~PublisherPort_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint32_t CHUNK_SIZE = 128;

    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    // publisher port w/o history
    iox::popo::PublisherPortData m_publisherPortData{
        iox::capro::ServiceDescription("a", "b", "c"), "myApp", &m_memoryManager};
    iox::popo::PublisherPortRouDi m_publisherPortRouDi{&m_publisherPortData};
    iox::popo::PublisherPortUser m_publisherPortUser{&m_publisherPortData};
};

TEST_F(PublisherPort_test, initial_state)
{
    EXPECT_FALSE(m_publisherPortUser.isOffered());
    EXPECT_FALSE(m_publisherPortUser.hasSubscribers());
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(PublisherPort_test, offering_stop_offering)
{
    // initial state
    EXPECT_FALSE(m_publisherPortUser.isOffered());
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_FALSE(maybeCaproMessage.has_value());

    // offering
    m_publisherPortUser.offer();
    EXPECT_TRUE(m_publisherPortUser.isOffered());
    maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
    EXPECT_THAT(caproMessage.m_subType, Eq(iox::capro::CaproMessageSubType::EVENT));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));

    // stop offering
    m_publisherPortUser.stopOffer();
    EXPECT_FALSE(m_publisherPortUser.isOffered());
    maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::STOP_OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
}

TEST_F(PublisherPort_test, offering_with_history)
{
    iox::popo::PublisherPortData m_publisherPortDataHistory{iox::capro::ServiceDescription("x", "y", "z"),
                                                            "myApp",
                                                            &m_memoryManager,
                                                            iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR};
    iox::popo::PublisherPortUser m_publisherPortUserHistory{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_publisherPortRouDiHistory{&m_publisherPortDataHistory};

    // offering
    m_publisherPortUserHistory.offer();
    EXPECT_TRUE(m_publisherPortUserHistory.isOffered());
    auto maybeCaproMessage = m_publisherPortRouDiHistory.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
    EXPECT_THAT(caproMessage.m_subType, Eq(iox::capro::CaproMessageSubType::FIELD));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR));
}

TEST_F(PublisherPort_test, allocate_free)
{
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
    auto maybeChunkHeader = m_publisherPortUser.allocateChunk(10u);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    auto chunkHeader = maybeChunkHeader.get_value();
    m_publisherPortUser.freeChunk(chunkHeader);
    // this one is not stored in the last chunk, so all chunks must be free again
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}


TEST_F(PublisherPort_test, allocate_send_no_subscriber)
{
    auto maybeChunkHeader = m_publisherPortUser.allocateChunk(10u);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    auto chunkHeader = maybeChunkHeader.get_value();
    m_publisherPortUser.sendChunk(chunkHeader);
    // this one is stored in the last chunk, so this chunk is still in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    // send another one, still only one chunk in use
    maybeChunkHeader = m_publisherPortUser.allocateChunk(10u);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    chunkHeader = maybeChunkHeader.get_value();
    m_publisherPortUser.sendChunk(chunkHeader);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(PublisherPort_test, subscribe_not_offered)
{
    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, unsubscribe_when_not_subscribed)
{
    m_publisherPortUser.offer();
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageReturned = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageReturned.m_type, Eq(iox::capro::CaproMessageType::OFFER));

    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    maybeCaproMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, subscribe_unsubscribe)
{
    m_publisherPortUser.offer();
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageReturned = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageReturned.m_type, Eq(iox::capro::CaproMessageType::OFFER));

    EXPECT_FALSE(m_publisherPortUser.hasSubscribers());

    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));

    EXPECT_TRUE(m_publisherPortUser.hasSubscribers());

    // set CaPro message to UNSUB, the other members are reused
    caproMessage.m_type = iox::capro::CaproMessageType::UNSUB;

    maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));

    EXPECT_FALSE(m_publisherPortUser.hasSubscribers());
}

TEST_F(PublisherPort_test, subscribe_till_overflow)
{
    m_publisherPortUser.offer();
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageReturned = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageReturned.m_type, Eq(iox::capro::CaproMessageType::OFFER));

    // using dummy pointers for the provided chunk queue data
    uint64_t dummy;
    uint64_t* dummyPtr = &dummy;
    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = reinterpret_cast<iox::popo::ChunkQueueData*>(dummyPtr);
    caproMessage.m_historyCapacity = 0u;

    for (size_t i = 0; i < iox::MAX_SUBSCRIBERS_PER_PUBLISHER; i++)
    {
        auto maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
        EXPECT_TRUE(maybeCaProMessage.has_value());
        auto caproMessageResponse = maybeCaProMessage.value();
        EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
        dummyPtr++;
        caproMessage.m_chunkQueueData = reinterpret_cast<iox::popo::ChunkQueueData*>(dummyPtr);
    }

    auto maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, subscribe_and_send)
{
    m_publisherPortUser.offer();
    auto maybeCaproMessage = m_publisherPortRouDi.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageReturned = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageReturned.m_type, Eq(iox::capro::CaproMessageType::OFFER));

    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaProMessage = m_publisherPortRouDi.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));

    EXPECT_TRUE(m_publisherPortUser.hasSubscribers());

    auto maybeChunkHeader = m_publisherPortUser.allocateChunk(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    auto chunkHeader = maybeChunkHeader.get_value();
    auto sample = chunkHeader->payload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    m_publisherPortUser.sendChunk(chunkHeader);

    iox::popo::ChunkQueuePopper m_chunkQueuePopper(&m_chunkQueueData);
    auto maybeSharedChunk = m_chunkQueuePopper.pop();
    EXPECT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17));
}

TEST_F(PublisherPort_test, subscribe_with_history)
{
    iox::popo::PublisherPortData m_publisherPortDataHistory{
        iox::capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, 1u}; // history = 1
    iox::popo::PublisherPortUser m_publisherPortUserHistory{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_publisherPortRouDiHistory{&m_publisherPortDataHistory};

    // do it the ara field like way

    // 1. publish a chunk to a not yet offered publisher
    auto maybeChunkHeader = m_publisherPortUserHistory.allocateChunk(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    auto chunkHeader = maybeChunkHeader.get_value();
    auto sample = chunkHeader->payload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    m_publisherPortUserHistory.sendChunk(chunkHeader);

    // 2. offer
    m_publisherPortUserHistory.offer();
    auto maybeCaproMessage = m_publisherPortRouDiHistory.getCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageReturned = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageReturned.m_type, Eq(iox::capro::CaproMessageType::OFFER));

    // 3. subscribe with a history request
    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 1u; // request history of 1

    auto maybeCaProMessage = m_publisherPortRouDiHistory.dispatchCaProMessage(caproMessage);
    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));

    // 4. We get the history value on subscribe
    iox::popo::ChunkQueuePopper m_chunkQueuePopper(&m_chunkQueueData);
    auto maybeSharedChunk = m_chunkQueuePopper.pop();
    EXPECT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17));
}

TEST_F(PublisherPort_test, last_chunk)
{
    auto maybeLastChunkHeader = m_publisherPortUser.getLastChunk();
    EXPECT_FALSE(maybeLastChunkHeader.has_value());

    auto maybeChunkHeader = m_publisherPortUser.allocateChunk(10u);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    auto chunkHeader = maybeChunkHeader.get_value();
    auto firstPayloadPtr = chunkHeader->payload();
    m_publisherPortUser.sendChunk(chunkHeader);

    maybeLastChunkHeader = m_publisherPortUser.getLastChunk();
    EXPECT_TRUE(maybeLastChunkHeader.has_value());
    EXPECT_THAT(maybeLastChunkHeader.value()->payload(), Eq(firstPayloadPtr));
}

TEST_F(PublisherPort_test, cleanup)
{
    iox::popo::PublisherPortData m_publisherPortDataHistory{iox::capro::ServiceDescription("x", "y", "z"),
                                                            "myApp",
                                                            &m_memoryManager,
                                                            iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR};
    iox::popo::PublisherPortUser m_publisherPortUserHistory{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_publisherPortRouDiHistory{&m_publisherPortDataHistory};

    // push some chunks to history
    for (size_t i = 0; i < iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR; i++)
    {
        auto maybeChunkHeader = m_publisherPortUserHistory.allocateChunk(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        auto chunkHeader = maybeChunkHeader.get_value();
        m_publisherPortUserHistory.sendChunk(chunkHeader);
    }

    // allocate some samples
    auto maybeChunkHeader1 = m_publisherPortUserHistory.allocateChunk(sizeof(DummySample));
    auto maybeChunkHeader2 = m_publisherPortUserHistory.allocateChunk(sizeof(DummySample));
    auto maybeChunkHeader3 = m_publisherPortUserHistory.allocateChunk(sizeof(DummySample));

    m_publisherPortRouDiHistory.cleanup();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}