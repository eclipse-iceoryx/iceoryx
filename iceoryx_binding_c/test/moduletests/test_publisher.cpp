// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::cxx;
using namespace iox::posix;

extern "C" {
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/publisher.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_pub_test : public Test
{
  protected:
    struct DummySample
    {
        uint64_t dummy{42};
    };

    iox_pub_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~iox_pub_test()
    {
    }

    void SetUp()
    {
        m_sut.m_portData = &m_publisherPortData;
        ::testing::internal::CaptureStderr();
    }

    void TearDown()
    {
        std::string output = ::testing::internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    void Subscribe(popo::PublisherPortData* ptr)
    {
        PublisherPortUser userPort(ptr);
        PublisherPortRouDi roudiPort(ptr);

        roudiPort.tryGetCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    void Unsubscribe(popo::PublisherPortData* ptr)
    {
        PublisherPortRouDi roudiPort(ptr);

        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint32_t CHUNK_SIZE = 128;

    using ChunkQueueData_t = popo::ChunkQueueData<DefaultChunkQueueConfig, popo::ThreadSafePolicy>;
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    GenericRAII m_uniqueRouDiId{[] { popo::internal::setUniqueRouDiId(0); },
                                [] { popo::internal::unsetUniqueRouDiId(); }};

    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    // publisher port w/o history
    PublisherPortData m_publisherPortData{
        ServiceDescription("a", "b", "c"), "myApp", &m_memoryManager, PublisherOptions()};

    // publisher port w/ history
    PublisherOptions m_publisherOptions{MAX_PUBLISHER_HISTORY};
    PublisherPortData m_publisherPortDataHistory{
        capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, m_publisherOptions};
    cpp2c_Publisher m_sut;
};

TEST_F(iox_pub_test, initialStateOfIsOfferedIsAsExpected)
{
    PublisherOptions iGotOptions;
    auto expectedIsOffered = iGotOptions.offerOnCreate;
    EXPECT_EQ(expectedIsOffered, iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, is_offeredAfterOffer)
{
    iox_pub_offer(&m_sut);
    EXPECT_TRUE(iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, isNotOfferedAfterStopOffer)
{
    iox_pub_offer(&m_sut);
    iox_pub_stop_offer(&m_sut);
    EXPECT_FALSE(iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, initialStateIsNoSubscribers)
{
    EXPECT_FALSE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, has_subscribersAfterSubscription)
{
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    EXPECT_TRUE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, noSubscribersAfterUnsubscribe)
{
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    this->Unsubscribe(&m_publisherPortData);
    EXPECT_FALSE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, allocateChunkForOneChunkIsSuccessful)
{
    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
}

TEST_F(iox_pub_test, chunkHeaderCanBeObtainedFromChunk)
{
    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
    auto header = iox_chunk_payload_to_header(chunk);
    EXPECT_NE(header, nullptr);
}

TEST_F(iox_pub_test, chunkHeaderCanBeConvertedBackToPayload)
{
    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
    auto header = iox_chunk_payload_to_header(chunk);
    auto payload = iox_chunk_header_to_payload(header);
    EXPECT_EQ(payload, chunk);
}

TEST_F(iox_pub_test, allocate_chunkFailsWhenHoldingToManyChunksInParallel)
{
    void* chunk = nullptr;
    for (int i = 0; i < 8 /* ///@todo actually it should be MAX_CHUNKS_HELD_PER_RECEIVER but it does not work*/; ++i)
    {
        EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, 100));
    }

    EXPECT_EQ(AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL, iox_pub_loan_chunk(&m_sut, &chunk, 100));
}

TEST_F(iox_pub_test, allocate_chunkFailsWhenOutOfChunks)
{
    std::vector<SharedChunk> chunkBucket;
    while (true)
    {
        auto sharedChunk = m_memoryManager.getChunk(100);
        if (sharedChunk)
            chunkBucket.emplace_back(sharedChunk);
        else
            break;
    }

    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_RUNNING_OUT_OF_CHUNKS, iox_pub_loan_chunk(&m_sut, &chunk, 100));
}

TEST_F(iox_pub_test, allocatingChunkAcquiresMemory)
{
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(iox_pub_test, freeingAnAllocatedChunkReleasesTheMemory)
{
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    iox_pub_release_chunk(&m_sut, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(iox_pub_test, noLastChunkWhenNothingSent)
{
    EXPECT_EQ(iox_pub_loan_previous_chunk(&m_sut), nullptr);
}

TEST_F(iox_pub_test, lastChunkAvailableAfterSend)
{
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    iox_pub_publish_chunk(&m_sut, chunk);

    const void* lastChunk = iox_pub_loan_previous_chunk(&m_sut);

    EXPECT_EQ(chunk, lastChunk);
}

TEST_F(iox_pub_test, sendDeliversChunk)
{
    void* chunk = nullptr;
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    static_cast<DummySample*>(chunk)->dummy = 4711;
    iox_pub_publish_chunk(&m_sut, chunk);

    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);
    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    ASSERT_TRUE(maybeSharedChunk.has_value());
    EXPECT_TRUE(*maybeSharedChunk == chunk);
    EXPECT_TRUE(static_cast<DummySample*>(maybeSharedChunk->getPayload())->dummy == 4711);
}

TEST_F(iox_pub_test, correctServiceDescriptionReturned)
{
    auto serviceDescription = iox_pub_get_service_description(&m_sut);

    EXPECT_THAT(serviceDescription.serviceId, Eq(iox::capro::InvalidID));
    EXPECT_THAT(serviceDescription.instanceId, Eq(iox::capro::InvalidID));
    EXPECT_THAT(serviceDescription.eventId, Eq(iox::capro::InvalidID));
    EXPECT_THAT(std::string(serviceDescription.serviceString), Eq("a"));
    EXPECT_THAT(std::string(serviceDescription.instanceString), Eq("b"));
    EXPECT_THAT(std::string(serviceDescription.eventString), Eq("c"));
}

TEST(iox_pub_options_test, publisherOptionsAreInitializedCorrectly)
{
    iox_pub_options_t sut;
    sut.historyCapacity = 37;
    sut.nodeName = "Dr.Gonzo";
    sut.offerOnCreate = false;

    PublisherOptions options;
    // set offerOnCreate to the opposite of the expected default to check if it gets overwritten to default
    sut.offerOnCreate = (options.offerOnCreate == false) ? true : false;

    iox_pub_options_init(&sut);
    EXPECT_EQ(sut.historyCapacity, options.historyCapacity);
    EXPECT_EQ(sut.nodeName, nullptr);
    EXPECT_EQ(sut.offerOnCreate, options.offerOnCreate);
    EXPECT_TRUE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionsInitializationCheckReturnsTrueAfterDefaultInit)
{
    iox_pub_options_t sut;
    iox_pub_options_init(&sut);
    EXPECT_TRUE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionsInitializationCheckReturnsFalseWithoutDefaultInit)
{
    iox_pub_options_t sut;
    EXPECT_FALSE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionInitializationWithNullptrDoesNotCrash)
{
    EXPECT_EXIT(
        {
            iox_pub_options_init(nullptr);
            exit(0);
        },
        ::testing::ExitedWithCode(0),
        ".*");
}

TEST(iox_pub_options_test, publisherInitializationTerminatesIfOptionsAreNotInitialized)
{
    iox_pub_options_t options;
    iox_pub_storage_t storage;

    EXPECT_DEATH({ iox_pub_init(&storage, "a", "b", "c", &options); }, ".*");
}

