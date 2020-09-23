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
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"

#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
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

    using ChunkQueueData_t = iox::popo::ChunkQueueData<iox::DefaultChunkQueueConfig, iox::popo::ThreadSafePolicy>;

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};

    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    // publisher port w/o history
    iox::popo::PublisherPortData m_publisherPortData{
        iox::capro::ServiceDescription("a", "b", "c"), "myApp", &m_memoryManager};
    iox::popo::PublisherPortRouDi m_sutRouDiSide{&m_publisherPortData};
    iox::popo::PublisherPortUser m_sutUserSide{&m_publisherPortData};

    // publisher port w/ history
    iox::popo::PublisherPortData m_publisherPortDataHistory{
        iox::capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, iox::MAX_PUBLISHER_HISTORY};
    iox::popo::PublisherPortUser m_sutWithHistoryUseriSide{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_sutWithHistoryRouDiSide{&m_publisherPortDataHistory};
};

TEST_F(PublisherPort_test, initialStateIsNotOffered)
{
    EXPECT_FALSE(m_sutUserSide.isOffered());
}

TEST_F(PublisherPort_test, initialStateIsNoSubscribers)
{
    EXPECT_FALSE(m_sutUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, initialStateReturnsNoCaProMessage)
{
    auto maybeCaproMessage = m_sutRouDiSide.tryGetCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(PublisherPort_test, offerCallResultsInOfferedState)
{
    m_sutUserSide.offer();

    EXPECT_TRUE(m_sutUserSide.isOffered());
}

TEST_F(PublisherPort_test, offerCallResultsInOfferCaProMessage)
{
    m_sutUserSide.offer();

    auto maybeCaproMessage = m_sutRouDiSide.tryGetCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
    EXPECT_THAT(caproMessage.m_subType, Eq(iox::capro::CaproMessageSubType::EVENT));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(PublisherPort_test, stopOfferCallResultsInNotOfferedState)
{
    m_sutUserSide.offer();

    m_sutUserSide.stopOffer();

    EXPECT_FALSE(m_sutUserSide.isOffered());
}

TEST_F(PublisherPort_test, stopOfferCallResultsInStopOfferCaProMessage)
{
    // arrange, we need a transition from offer to stop offer, also form a RouDi point of view
    // therefore we must also get the offer CapPro message (but ignore it here)
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    m_sutUserSide.stopOffer();

    auto maybeCaproMessage = m_sutRouDiSide.tryGetCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::STOP_OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
}

TEST_F(PublisherPort_test, offerStateChangesThatEndUpInTheSameStateDoNotReturnACaProMessage)
{
    m_sutUserSide.offer();
    m_sutUserSide.stopOffer();

    auto maybeCaproMessage = m_sutRouDiSide.tryGetCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(PublisherPort_test,
       offerCallWhenHavingHistoryResultsInOfferCaProMessageWithSubTypeFieldAndCorrectHistoryCapacity)
{
    m_sutWithHistoryUseriSide.offer();

    auto maybeCaproMessage = m_sutWithHistoryRouDiSide.tryGetCaProMessage();
    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
    EXPECT_THAT(caproMessage.m_subType, Eq(iox::capro::CaproMessageSubType::FIELD));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(iox::MAX_PUBLISHER_HISTORY));
}

TEST_F(PublisherPort_test, allocatingAChunk)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);

    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(PublisherPort_test, freeingAnAllocatedChunkReleasesTheMemory)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    auto chunkHeader = maybeChunkHeader.get_value();

    m_sutUserSide.freeChunk(chunkHeader);

    // this one is not stored in the last chunk, so all chunks must be free again
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(PublisherPort_test, allocatedChunkContainsPublisherIdAsOriginId)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    auto chunkHeader = maybeChunkHeader.get_value();

    EXPECT_THAT(chunkHeader->m_originId, Eq(m_sutUserSide.getUniqueID()));
    m_sutUserSide.freeChunk(chunkHeader);
}

TEST_F(PublisherPort_test, allocateAndSendAChunkWithoutSubscriberHoldsTheLast)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    auto chunkHeader = maybeChunkHeader.get_value();

    m_sutUserSide.sendChunk(chunkHeader);

    // this one is stored in the last chunk, so this chunk is still in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(PublisherPort_test, allocateAndSendMultipleChunksWithoutSubscriberHoldsOnlyTheLast)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    auto chunkHeader = maybeChunkHeader.get_value();
    m_sutUserSide.sendChunk(chunkHeader);
    maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    chunkHeader = maybeChunkHeader.get_value();
    m_sutUserSide.sendChunk(chunkHeader);
    maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    chunkHeader = maybeChunkHeader.get_value();
    m_sutUserSide.sendChunk(chunkHeader);

    // the last is stored in the last chunk, so one chunk is still in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(PublisherPort_test, subscribeWhenNotOfferedReturnsNACK)
{
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, unsubscribeWhenNotSubscribedReturnsNACK)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaproMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, subscribeWhenOfferedReturnsACKAndWeHaveSubscribers)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;

    auto maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
    EXPECT_TRUE(m_sutUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, unsubscribeWhenSubscribedReturnsACKAndWeHaveNoMoreSubscribers)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;
    auto maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    // set CaPro message to UNSUB, the other members are reused
    caproMessage.m_type = iox::capro::CaproMessageType::UNSUB;

    maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
    EXPECT_FALSE(m_sutUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, subscribeManyIsFine)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    // using dummy pointers for the provided chunk queue data
    uint64_t dummy;
    uint64_t* dummyPtr = &dummy;
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    caproMessage.m_historyCapacity = 0u;

    for (size_t i = 0; i < iox::MAX_SUBSCRIBERS_PER_PUBLISHER; i++)
    {
        auto maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        EXPECT_TRUE(maybeCaProMessage.has_value());
        auto caproMessageResponse = maybeCaProMessage.value();
        EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
        dummyPtr++;
        caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    }
}

TEST_F(PublisherPort_test, subscribeTillOverflowReturnsNACK)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    // using dummy pointers for the provided chunk queue data
    uint64_t dummy;
    uint64_t* dummyPtr = &dummy;
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    caproMessage.m_historyCapacity = 0u;
    for (size_t i = 0; i < iox::MAX_SUBSCRIBERS_PER_PUBLISHER; i++)
    {
        m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        dummyPtr++;
        caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    }

    auto maybeCaProMessage = m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, sendWhenSubscribedDeliversAChunk)
{
    m_sutUserSide.offer();
    m_sutRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0u;
    m_sutRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(sizeof(DummySample));
    auto chunkHeader = maybeChunkHeader.get_value();
    auto sample = chunkHeader->payload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    m_sutUserSide.sendChunk(chunkHeader);
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);

    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    EXPECT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17));
}

TEST_F(PublisherPort_test, subscribeWithHistoryLikeTheARAField)
{
    iox::popo::PublisherPortData m_publisherPortDataHistory{
        iox::capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, 1u}; // history = 1
    iox::popo::PublisherPortUser m_sutWithHistoryUseriSide{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_sutWithHistoryRouDiSide{&m_publisherPortDataHistory};
    // do it the ara field like way
    // 1. publish a chunk to a not yet offered publisher
    auto maybeChunkHeader = m_sutWithHistoryUseriSide.tryAllocateChunk(sizeof(DummySample));
    auto chunkHeader = maybeChunkHeader.get_value();
    auto sample = chunkHeader->payload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    m_sutWithHistoryUseriSide.sendChunk(chunkHeader);
    // 2. offer
    m_sutWithHistoryUseriSide.offer();
    m_sutWithHistoryRouDiSide.tryGetCaProMessage();
    // 3. subscribe with a history request
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 1u; // request history of 1
    m_sutWithHistoryRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);

    // 4. We get the history value on subscribe
    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    EXPECT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17));
}

TEST_F(PublisherPort_test, noLastChunkWhenNothingSent)
{
    auto maybeLastChunkHeader = m_sutUserSide.tryGetPreviousChunk();

    EXPECT_FALSE(maybeLastChunkHeader.has_value());
}

TEST_F(PublisherPort_test, lastChunkAvailableAfterSend)
{
    auto maybeChunkHeader = m_sutUserSide.tryAllocateChunk(10u);
    auto chunkHeader = maybeChunkHeader.get_value();
    auto firstPayloadPtr = chunkHeader->payload();
    m_sutUserSide.sendChunk(chunkHeader);

    auto maybeLastChunkHeader = m_sutUserSide.tryGetPreviousChunk();

    EXPECT_TRUE(maybeLastChunkHeader.has_value());
    EXPECT_THAT(maybeLastChunkHeader.value()->payload(), Eq(firstPayloadPtr));
}

TEST_F(PublisherPort_test, cleanupReleasesAllChunks)
{
    // push some chunks to history
    for (size_t i = 0; i < iox::MAX_PUBLISHER_HISTORY; i++)
    {
        auto maybeChunkHeader = m_sutWithHistoryUseriSide.tryAllocateChunk(sizeof(DummySample));
        auto chunkHeader = maybeChunkHeader.get_value();
        m_sutWithHistoryUseriSide.sendChunk(chunkHeader);
    }
    // allocate some samples
    auto maybeChunkHeader1 = m_sutWithHistoryUseriSide.tryAllocateChunk(sizeof(DummySample));
    auto maybeChunkHeader2 = m_sutWithHistoryUseriSide.tryAllocateChunk(sizeof(DummySample));
    auto maybeChunkHeader3 = m_sutWithHistoryUseriSide.tryAllocateChunk(sizeof(DummySample));

    m_sutWithHistoryRouDiSide.releaseAllChunks();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}
