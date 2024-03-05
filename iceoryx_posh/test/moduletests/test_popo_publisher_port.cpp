// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/bump_allocator.hpp"
#include "test.hpp"

#include <memory>

namespace
{
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
        m_mempoolconf.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_mempoolconf.addMemPool({BIG_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);
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
    static constexpr uint64_t SMALL_CHUNK = 128;
    static constexpr uint64_t BIG_CHUNK = 256;

    static constexpr uint32_t USER_PAYLOAD_ALIGNMENT = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
    static constexpr uint32_t USER_HEADER_SIZE = iox::CHUNK_NO_USER_HEADER_SIZE;
    static constexpr uint32_t USER_HEADER_ALIGNMENT = iox::CHUNK_NO_USER_HEADER_ALIGNMENT;

    using ChunkQueueData_t = iox::popo::ChunkQueueData<iox::DefaultChunkQueueConfig, iox::popo::ThreadSafePolicy>;

    iox::BumpAllocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;


    // publisher port w/o offer on create
    iox::popo::PublisherOptions m_noOfferOnCreatePublisherOptions{
        0U, iox::NodeName_t{""}, false, iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};
    iox::popo::PublisherPortData m_publisherPortDataNoOfferOnCreate{iox::capro::ServiceDescription("a", "b", "c"),
                                                                    "myApp",
                                                                    iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                                    &m_memoryManager,
                                                                    m_noOfferOnCreatePublisherOptions};
    iox::popo::PublisherPortRouDi m_sutNoOfferOnCreateRouDiSide{&m_publisherPortDataNoOfferOnCreate};
    iox::popo::PublisherPortUser m_sutNoOfferOnCreateUserSide{&m_publisherPortDataNoOfferOnCreate};

    // publisher port that waits for subscriber when queue is full
    iox::popo::PublisherOptions m_waitForSubscriberPublisherOptions{
        0U, iox::NodeName_t{""}, false, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    iox::popo::PublisherPortData m_publisherPortDataWaitForSubscriber{iox::capro::ServiceDescription("a", "b", "c"),
                                                                      "myApp",
                                                                      iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                                      &m_memoryManager,
                                                                      m_waitForSubscriberPublisherOptions};
    iox::popo::PublisherPortRouDi m_sutWaitForSubscriberRouDiSide{&m_publisherPortDataWaitForSubscriber};
    iox::popo::PublisherPortUser m_sutWaitForSubscriberUserSide{&m_publisherPortDataWaitForSubscriber};

    // publisher port w/ history
    iox::popo::PublisherOptions m_withHistoryPublisherOptions{
        iox::MAX_PUBLISHER_HISTORY, iox::NodeName_t{""}, true, iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};
    iox::popo::PublisherPortData m_publisherPortDataHistory{iox::capro::ServiceDescription("x", "y", "z"),
                                                            "myApp",
                                                            iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                            &m_memoryManager,
                                                            m_withHistoryPublisherOptions};
    iox::popo::PublisherPortUser m_sutWithHistoryUserSide{&m_publisherPortDataHistory};
    iox::popo::PublisherPortRouDi m_sutWithHistoryRouDiSide{&m_publisherPortDataHistory};

    // publisher port w/ default options
    iox::popo::PublisherOptions m_withDefaultPublisherOptions{};
    iox::popo::PublisherPortData m_publisherPortDataDefault{iox::capro::ServiceDescription("x", "y", "z"),
                                                            "myApp",
                                                            iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                            &m_memoryManager,
                                                            m_withDefaultPublisherOptions};
    iox::popo::PublisherPortUser m_sutWithDefaultOptionsUserSide{&m_publisherPortDataDefault};
    iox::popo::PublisherPortRouDi m_sutWithDefaultOptionsRouDiSide{&m_publisherPortDataDefault};
};

TEST_F(PublisherPort_test, initialStateIsOfferedWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "70bd6717-6ccf-4191-b4e5-f9e3470eae07");
    EXPECT_TRUE(m_sutWithDefaultOptionsUserSide.isOffered());
}

TEST_F(PublisherPort_test, initialStateIsNotOfferedWhenNoOfferOnCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "46e14a35-8264-45a3-b157-f335c4564276");
    EXPECT_FALSE(m_sutNoOfferOnCreateUserSide.isOffered());
}

TEST_F(PublisherPort_test, initialStateIsNoSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5be59ad-3921-45e9-a5f8-74c8015ddced");
    EXPECT_FALSE(m_sutNoOfferOnCreateUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, noWaitingForSubscriberWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "d1f74874-257a-4e8f-aabf-8eadad5b4367");
    EXPECT_THAT(m_sutWithDefaultOptionsRouDiSide.getOptions().subscriberTooSlowPolicy,
                Eq(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PublisherPort_test, initialStateReturnsOfferCaProMessageWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "033a2229-609b-47a7-adc1-ab696ab36d46");
    auto maybeCaproMessage = m_sutWithDefaultOptionsRouDiSide.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
}

TEST_F(PublisherPort_test, initialStateReturnsNoCaProMessageWhenNoOfferOnCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "93112fd3-f67e-424f-aac5-7758a7a6ea27");
    auto maybeCaproMessage = m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(PublisherPort_test, waitingForSubscriberWhenDesired)
{
    ::testing::Test::RecordProperty("TEST_ID", "49526d1a-e81a-4e4a-8fb4-1a96dee83ae7");
    EXPECT_THAT(m_sutWaitForSubscriberRouDiSide.getOptions().subscriberTooSlowPolicy,
                Eq(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER));
}

TEST_F(PublisherPort_test, offerCallResultsInOfferedState)
{
    ::testing::Test::RecordProperty("TEST_ID", "d15f9164-7c9a-46cf-aecb-253e5a7e1b79");
    m_sutNoOfferOnCreateUserSide.offer();

    EXPECT_TRUE(m_sutNoOfferOnCreateUserSide.isOffered());
}

TEST_F(PublisherPort_test, offerCallResultsInOfferCaProMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "328fa84e-ca6b-4e58-b47c-559709855751");
    m_sutNoOfferOnCreateUserSide.offer();

    auto maybeCaproMessage = m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
    EXPECT_THAT(caproMessage.m_serviceType, Eq(iox::capro::CaproServiceType::PUBLISHER));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0U));
}

TEST_F(PublisherPort_test, stopOfferCallResultsInNotOfferedState)
{
    ::testing::Test::RecordProperty("TEST_ID", "49985d1e-e7ed-4fc2-9d0a-d78d61b74e3c");
    m_sutNoOfferOnCreateUserSide.offer();

    m_sutNoOfferOnCreateUserSide.stopOffer();

    EXPECT_FALSE(m_sutNoOfferOnCreateUserSide.isOffered());
}

TEST_F(PublisherPort_test, stopOfferCallResultsInStopOfferCaProMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "0980c54a-2420-4f25-8546-8ca4b36e504b");
    // arrange, we need a transition from offer to stop offer, also form a RouDi point of view
    // therefore we must also get the offer CapPro message (but ignore it here)
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    m_sutNoOfferOnCreateUserSide.stopOffer();

    auto maybeCaproMessage = m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::STOP_OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("a", "b", "c")));
}

TEST_F(PublisherPort_test, offerStateChangesThatEndUpInTheSameStateDoNotReturnACaProMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "885962f8-b5f1-4ed8-9001-ba95aa2b8db2");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateUserSide.stopOffer();

    auto maybeCaproMessage = m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(PublisherPort_test,
       offerCallWhenHavingHistoryResultsInOfferCaProMessageWithSubTypeFieldAndCorrectHistoryCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc607126-8bee-4e02-b1c6-4f8eb27076a8");
    m_sutWithHistoryUserSide.offer();

    auto maybeCaproMessage = m_sutWithHistoryRouDiSide.tryGetCaProMessage();
    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::OFFER));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
    EXPECT_THAT(caproMessage.m_serviceType, Eq(iox::capro::CaproServiceType::PUBLISHER));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(iox::MAX_PUBLISHER_HISTORY));
}

TEST_F(PublisherPort_test, allocatingAChunkWithoutUserHeaderAndSmallUserPayloadAlignmentResultsInSmallChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "467e0f06-3450-4cc9-ab84-5ccd5efab69d");
    constexpr uint64_t USER_PAYLOAD_SIZE{SMALL_CHUNK / 2};
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);

    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(PublisherPort_test, allocatingAChunkWithoutUserHeaderAndLargeUserPayloadAlignmentResultsInLargeChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bdf0578-93b3-470d-84af-9139919665db");
    constexpr uint64_t USER_PAYLOAD_SIZE{SMALL_CHUNK / 2};
    constexpr uint32_t LARGE_USER_PAYLOAD_ALIGNMENT{SMALL_CHUNK};
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        USER_PAYLOAD_SIZE, LARGE_USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);

    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));
}

TEST_F(PublisherPort_test, allocatingAChunkWithLargeUserHeaderResultsInLargeChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "598e04d8-8a37-43ef-b686-64e7b2723ffe");
    constexpr uint64_t USER_PAYLOAD_SIZE{SMALL_CHUNK / 2};
    constexpr uint32_t LARGE_USER_HEADER_SIZE{SMALL_CHUNK};
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT, LARGE_USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);

    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));
}

TEST_F(PublisherPort_test, releasingAnAllocatedChunkReleasesTheMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a88a36b-73c5-4699-8d88-bfe4c19bfd81");
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();

    m_sutNoOfferOnCreateUserSide.releaseChunk(chunkHeader);

    // this one is not stored in the last chunk, so all chunks must be free again
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(PublisherPort_test, allocatedChunkContainsPublisherIdAsOriginId)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b873fcb-d67d-48ca-a67d-b807311161d4");
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();

    EXPECT_THAT(chunkHeader->originId(), Eq(m_sutNoOfferOnCreateUserSide.getUniqueID()));
    m_sutNoOfferOnCreateUserSide.releaseChunk(chunkHeader);
}

TEST_F(PublisherPort_test, allocateAndSendAChunkWithoutSubscriberHoldsTheLast)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b2e2930-4271-4e56-ac84-810d6d5745e4");
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();

    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);

    // this one is stored in the last chunk, so this chunk is still in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(PublisherPort_test, allocateAndSendMultipleChunksWithoutSubscriberHoldsOnlyTheLast)
{
    ::testing::Test::RecordProperty("TEST_ID", "761cdd5c-2692-4e0b-b978-609524c48708");
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();
    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);
    maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    chunkHeader = maybeChunkHeader.value();
    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);
    maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    chunkHeader = maybeChunkHeader.value();
    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);

    // the last is stored in the last chunk, so one chunk is still in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(PublisherPort_test, subscribeWhenNotOfferedReturnsNACK)
{
    ::testing::Test::RecordProperty("TEST_ID", "71148938-58f1-4189-8461-8bab912e32c6");
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0U;

    auto maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, unsubscribeWhenNotSubscribedReturnsNACK)
{
    ::testing::Test::RecordProperty("TEST_ID", "c68043b2-e7e1-4b73-a860-3b2980505545");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0U;

    auto maybeCaproMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, subscribeWhenOfferedReturnsACKAndWeHaveSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e5fa8bb-7b07-49f7-9228-47b66afb00c7");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0U;

    auto maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
    EXPECT_TRUE(m_sutNoOfferOnCreateUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, unsubscribeWhenSubscribedReturnsACKAndWeHaveNoMoreSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "d11815bc-0d63-481e-83c7-4eed60322062");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0U;
    auto maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    // set CaPro message to UNSUB, the other members are reused
    caproMessage.m_type = iox::capro::CaproMessageType::UNSUB;

    maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
    EXPECT_FALSE(m_sutNoOfferOnCreateUserSide.hasSubscribers());
}

TEST_F(PublisherPort_test, subscribeManyIsFine)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ee3c448-7091-4a99-b03b-6ae321cf96ba");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    // using dummy pointers for the provided chunk queue data
    uint64_t dummy;
    uint64_t* dummyPtr = &dummy;
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    caproMessage.m_historyCapacity = 0U;

    for (size_t i = 0; i < iox::MAX_SUBSCRIBERS_PER_PUBLISHER; i++)
    {
        auto maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        ASSERT_TRUE(maybeCaProMessage.has_value());
        auto caproMessageResponse = maybeCaProMessage.value();
        EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::ACK));
        dummyPtr++;
        caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    }
}

TEST_F(PublisherPort_test, subscribeTillOverflowReturnsNACK)
{
    ::testing::Test::RecordProperty("TEST_ID", "4726b002-93df-48cd-b190-757fe772d694");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    // using dummy pointers for the provided chunk queue data
    uint64_t dummy;
    uint64_t* dummyPtr = &dummy;
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    caproMessage.m_historyCapacity = 0U;
    for (size_t i = 0; i < iox::MAX_SUBSCRIBERS_PER_PUBLISHER; i++)
    {
        m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        dummyPtr++;
        caproMessage.m_chunkQueueData = reinterpret_cast<ChunkQueueData_t*>(dummyPtr);
    }

    auto maybeCaProMessage = m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaProMessage.has_value());
    auto caproMessageResponse = maybeCaProMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::NACK));
}

TEST_F(PublisherPort_test, sendWhenSubscribedDeliversAChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "659db6ee-7843-4aa7-b633-916614b6a711");
    m_sutNoOfferOnCreateUserSide.offer();
    m_sutNoOfferOnCreateRouDiSide.tryGetCaProMessage();
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 0U;
    m_sutNoOfferOnCreateRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();
    auto sample = chunkHeader->userPayload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);

    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    ASSERT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getUserPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17U));
}

TEST_F(PublisherPort_test, subscribeWithHistoryLikeTheARAField)
{
    ::testing::Test::RecordProperty("TEST_ID", "12ea9650-c928-4185-8519-be949e2afcf7");
    iox::popo::PublisherOptions options;
    options.historyCapacity = 1U;
    iox::popo::PublisherPortData publisherPortDataHistory{iox::capro::ServiceDescription("x", "y", "z"),
                                                          "myApp",
                                                          iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                          &m_memoryManager,
                                                          options};
    iox::popo::PublisherPortUser sutWithHistoryUseriSide{&publisherPortDataHistory};
    iox::popo::PublisherPortRouDi sutWithHistoryRouDiSide{&publisherPortDataHistory};
    // do it the ara field like way
    // 1. publish a chunk to a not yet offered publisher
    auto maybeChunkHeader = sutWithHistoryUseriSide.tryAllocateChunk(
        sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();
    auto sample = chunkHeader->userPayload();
    new (sample) DummySample();
    static_cast<DummySample*>(sample)->dummy = 17;
    sutWithHistoryUseriSide.sendChunk(chunkHeader);
    // 2. offer
    sutWithHistoryUseriSide.offer();
    sutWithHistoryRouDiSide.tryGetCaProMessage();
    // 3. subscribe with a history request
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("a", "b", "c"));
    caproMessage.m_chunkQueueData = &m_chunkQueueData;
    caproMessage.m_historyCapacity = 1U; // request history of 1
    sutWithHistoryRouDiSide.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);

    // 4. We get the history value on subscribe
    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    ASSERT_TRUE(maybeSharedChunk.has_value());
    auto sharedChunk = maybeSharedChunk.value();
    auto dummySample = *reinterpret_cast<DummySample*>(sharedChunk.getUserPayload());
    EXPECT_THAT(dummySample.dummy, Eq(17U));
}

TEST_F(PublisherPort_test, noLastChunkWhenNothingSent)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9a076d8-ed09-4344-9053-d3d513a17d0a");
    auto maybeLastChunkHeader = m_sutNoOfferOnCreateUserSide.tryGetPreviousChunk();

    EXPECT_FALSE(maybeLastChunkHeader.has_value());
}

TEST_F(PublisherPort_test, lastChunkAvailableAfterSend)
{
    ::testing::Test::RecordProperty("TEST_ID", "b44de075-2a53-4576-92db-5fcb41d68700");
    auto maybeChunkHeader = m_sutNoOfferOnCreateUserSide.tryAllocateChunk(
        10U, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunkHeader = maybeChunkHeader.value();
    auto firstPayloadPtr = chunkHeader->userPayload();
    m_sutNoOfferOnCreateUserSide.sendChunk(chunkHeader);

    auto maybeLastChunkHeader = m_sutNoOfferOnCreateUserSide.tryGetPreviousChunk();

    ASSERT_TRUE(maybeLastChunkHeader.has_value());
    EXPECT_THAT(maybeLastChunkHeader.value()->userPayload(), Eq(firstPayloadPtr));
}

TEST_F(PublisherPort_test, cleanupReleasesAllChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a78f11a6-8d4e-4ab5-888a-a2706ff97ec1");
    // push some chunks to history
    for (size_t i = 0; i < iox::MAX_PUBLISHER_HISTORY; i++)
    {
        auto maybeChunkHeader = m_sutWithHistoryUserSide.tryAllocateChunk(
            sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        auto chunkHeader = maybeChunkHeader.value();
        m_sutWithHistoryUserSide.sendChunk(chunkHeader);
    }
    // allocate some samples
    auto maybeChunkHeader1 = m_sutWithHistoryUserSide.tryAllocateChunk(
        sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader1.has_error());
    auto maybeChunkHeader2 = m_sutWithHistoryUserSide.tryAllocateChunk(
        sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader2.has_error());
    auto maybeChunkHeader3 = m_sutWithHistoryUserSide.tryAllocateChunk(
        sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader3.has_error());

    m_sutWithHistoryRouDiSide.releaseAllChunks();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

} // namespace
