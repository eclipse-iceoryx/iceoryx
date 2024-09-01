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

#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/runtime.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::testing;
using namespace iox::roudi_env;
using namespace iox::capro;
using namespace iox::mepoo;

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
        m_memoryManager.configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);
    }

    ~iox_pub_test()
    {
    }

    void SetUp() override
    {
        m_sut.m_portData = &m_publisherPortData;
    }

    void TearDown() override
    {
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
    alignas(8) uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint64_t CHUNK_SIZE = 256;

    using ChunkQueueData_t = popo::ChunkQueueData<DefaultChunkQueueConfig, popo::ThreadSafePolicy>;
    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    BumpAllocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    // publisher port w/o history
    PublisherPortData m_publisherPortData{ServiceDescription("a", "b", "c"),
                                          "myApp",
                                          roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                          &m_memoryManager,
                                          PublisherOptions()};

    // publisher port w/ history
    PublisherOptions m_publisherOptions{MAX_PUBLISHER_HISTORY};
    PublisherPortData m_publisherPortDataHistory{capro::ServiceDescription("x", "y", "z"),
                                                 "myApp",
                                                 roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                 &m_memoryManager,
                                                 m_publisherOptions};
    cpp2c_Publisher m_sut;
};

TEST_F(iox_pub_test, initPublisherWithNullptrForStorageReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ac8921a-957a-4bd4-8943-7294defc78d3");
    iox_pub_options_t options;
    iox_pub_options_init(&options);

    EXPECT_EQ(iox_pub_init(nullptr, "all", "glory", "hypnotoad", &options), nullptr);
}

// this crashes if the fixture is used, therefore a test without a fixture
TEST(iox_pub_test_DeathTest, initPublisherWithNotInitializedPublisherOptionsTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "c8768fd8-e02c-4f87-99a0-32a2c36af4b1");
    iox_pub_options_t options;
    iox_pub_storage_t storage;

    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_init(&storage, "a", "b", "c", &options); },
                             iox::CBindingError::BINDING_C__PUBLISHER_OPTIONS_NOT_INITIALIZED);
}

TEST_F(iox_pub_test, initPublisherWithDefaultOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2e677cd-2fcc-47a2-80e6-2d08245b7c1a");
    iox::roudi_env::RouDiEnv roudiEnv;

    iox_runtime_init("hypnotoad");

    iox_pub_options_t options;
    iox_pub_options_init(&options);
    iox_pub_storage_t storage;

    auto sut = iox_pub_init(&storage, "a", "b", "c", &options);
    EXPECT_THAT(sut, Ne(nullptr));
    iox_pub_deinit(sut);
}

TEST_F(iox_pub_test, initialStateOfIsOfferedIsAsExpected)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa757a54-a8df-420e-b32d-a9d5724a7d20");
    PublisherOptions iGotOptions;
    auto expectedIsOffered = iGotOptions.offerOnCreate;
    EXPECT_EQ(expectedIsOffered, iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, is_offeredAfterOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "719c76be-e6b5-48a7-bd87-a7daea702983");
    iox_pub_offer(&m_sut);
    EXPECT_TRUE(iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, isNotOfferedAfterStopOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "0da3afd6-a8f5-489d-b44a-83c89a626e82");
    iox_pub_offer(&m_sut);
    iox_pub_stop_offer(&m_sut);
    EXPECT_FALSE(iox_pub_is_offered(&m_sut));
}

TEST_F(iox_pub_test, initialStateIsNoSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cd0f66b-cab8-4b2f-8e53-679b415dc745");
    EXPECT_FALSE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, has_subscribersAfterSubscription)
{
    ::testing::Test::RecordProperty("TEST_ID", "b970de58-c253-4e07-a6a0-61df061e8173");
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    EXPECT_TRUE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, noSubscribersAfterUnsubscribe)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3de1ea2-64ed-48b6-bc1c-967e010fee32");
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    this->Unsubscribe(&m_publisherPortData);
    EXPECT_FALSE(iox_pub_has_subscribers(&m_sut));
}

TEST_F(iox_pub_test, allocateChunkForOneChunkIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a8f2275-f153-469e-8699-f36a136608a1");
    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
}

TEST_F(iox_pub_test, allocateChunkUserPayloadAlignmentIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6e7e2ea-fa76-43e7-b217-5bfc4b94837c");
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{128U};
    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS,
              iox_pub_loan_aligned_chunk(&m_sut, &chunk, sizeof(DummySample), USER_PAYLOAD_ALIGNMENT));

    EXPECT_TRUE(reinterpret_cast<uint64_t>(chunk) % USER_PAYLOAD_ALIGNMENT == 0U);
}

TEST_F(iox_pub_test, allocateChunkWithUserHeaderIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "53c07218-2ac9-4e2b-8ca3-7df856cd1cc5");
    constexpr uint32_t USER_HEADER_SIZE = 4U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 2U;

    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS,
              iox_pub_loan_aligned_chunk_with_user_header(
                  &m_sut, &chunk, sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT));

    auto chunkHeader = iox_chunk_header_from_user_payload(chunk);
    auto spaceBetweenChunkHeaderAndUserPaylod =
        reinterpret_cast<uint64_t>(chunk) - reinterpret_cast<uint64_t>(chunkHeader);
    EXPECT_GT(spaceBetweenChunkHeaderAndUserPaylod, sizeof(iox::mepoo::ChunkHeader));
}

TEST_F(iox_pub_test, allocateChunkWithUserHeaderAndUserPayloadAlignmentFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2bcb0d4-b207-436f-9909-9e67815f525a");
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{128U};
    constexpr uint32_t USER_HEADER_SIZE = 4U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 3U;

    void* chunk = nullptr;
    ASSERT_EQ(
        AllocationResult_INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER,
        iox_pub_loan_aligned_chunk_with_user_header(
            &m_sut, &chunk, sizeof(DummySample), USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT));
}

TEST_F(iox_pub_test, chunkHeaderCanBeObtainedFromChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4b620af-4ec8-4638-b81b-6759ed048d36");
    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
    auto chunkHeader = iox_chunk_header_from_user_payload(chunk);
    EXPECT_NE(chunkHeader, nullptr);
}

TEST_F(iox_pub_test, chunkHeaderCanBeConvertedBackToUserPayload)
{
    ::testing::Test::RecordProperty("TEST_ID", "01dbfd4c-7345-4291-aa2c-2404f15c4bda");
    void* chunk = nullptr;
    ASSERT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, sizeof(DummySample)));
    auto chunkHeader = iox_chunk_header_from_user_payload(chunk);
    auto userPayloadFromRoundtrip = iox_chunk_header_to_user_payload(chunkHeader);
    EXPECT_EQ(userPayloadFromRoundtrip, chunk);
}

TEST_F(iox_pub_test, allocate_chunkFailsWhenHoldingToManyChunksInParallel)
{
    ::testing::Test::RecordProperty("TEST_ID", "77271a1c-242d-438b-8f52-e4819fdf1033");
    void* chunk = nullptr;
    for (uint32_t i = 0U; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; ++i)
    {
        EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_loan_chunk(&m_sut, &chunk, 100));
    }

    EXPECT_EQ(AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL, iox_pub_loan_chunk(&m_sut, &chunk, 100));
}

TEST_F(iox_pub_test, allocate_chunkFailsWhenOutOfChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7563ed4c-a6d5-487d-9c6c-937d8e8c3d1d");
    constexpr uint64_t USER_PAYLOAD_SIZE{100U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    std::vector<SharedChunk> chunkStore;
    while (!m_memoryManager.getChunk(chunkSettings)
                .and_then([&](auto& chunk) { chunkStore.emplace_back(chunk); })
                .has_error())
    {
    }

    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_RUNNING_OUT_OF_CHUNKS, iox_pub_loan_chunk(&m_sut, &chunk, 100));
}

TEST_F(iox_pub_test, allocatingChunkAcquiresMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a779236-0302-48b5-add7-0a2c33a9d86b");
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(iox_pub_test, freeingAnAllocatedChunkReleasesTheMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d3901bc-28a1-4a6d-9d81-d3a456e1f806");
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    iox_pub_release_chunk(&m_sut, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(iox_pub_test, sendDeliversChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "187d552a-6903-40cd-88a0-7722eb2a40a8");
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
    EXPECT_TRUE(static_cast<DummySample*>(maybeSharedChunk->getUserPayload())->dummy == 4711);
}

TEST_F(iox_pub_test, correctServiceDescriptionReturned)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f91cb12-fbfa-4bad-ad59-ab2579f83fbe");
    auto serviceDescription = iox_pub_get_service_description(&m_sut);

    EXPECT_THAT(std::string(serviceDescription.serviceString), Eq("a"));
    EXPECT_THAT(std::string(serviceDescription.instanceString), Eq("b"));
    EXPECT_THAT(std::string(serviceDescription.eventString), Eq("c"));
}

TEST_F(iox_pub_test, pubReleaseChunkWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "002fa1a4-e364-41a5-be59-6dbfa6543b98");
    void* chunk = nullptr;
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_release_chunk(nullptr, chunk); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_release_chunk(&m_sut, nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubPublishChunckWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f4db395-fb0b-45e2-9736-9831892ba314");
    void* chunk = nullptr;
    iox_pub_offer(&m_sut);
    this->Subscribe(&m_publisherPortData);
    iox_pub_loan_chunk(&m_sut, &chunk, 100);
    static_cast<DummySample*>(chunk)->dummy = 4711;
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_publish_chunk(nullptr, chunk); }, iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_publish_chunk(&m_sut, nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubOfferWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "5588dacf-6e6c-44c6-835d-1dfeb03ff2c1");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_offer(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubStopOfferWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "db267aa8-071e-402d-887e-1a81fd5b40ca");
    iox_pub_offer(&m_sut);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_stop_offer(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, isPubOfferedWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b637e0c-c544-45e8-9723-9c54e78df0b0");
    iox_pub_offer(&m_sut);

    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_is_offered(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubHasSubscribersWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f9c319e-0fa5-454d-b416-62c5f7125562");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_has_subscribers(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubGetServiceDescriptionWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "82113c0b-910b-41c3-b22a-f93e756eecd9");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_get_service_description(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_pub_test, pubDeinitWithNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a2ef759-4b29-40d1-bef1-9e2cd4c8ad75");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_pub_deinit(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST(iox_pub_options_test, publisherOptionsAreInitializedCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "dfc9e8b7-efb8-4fe6-a5a3-ebd9c2ad45ac");
    iox_pub_options_t sut;
    sut.historyCapacity = 37;
    sut.nodeName = "Dr.Gonzo";
    sut.offerOnCreate = false;
    sut.subscriberTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;

    PublisherOptions options;
    // set offerOnCreate to the opposite of the expected default to check if it gets overwritten to default
    sut.offerOnCreate = (options.offerOnCreate == false) ? true : false;

    iox_pub_options_init(&sut);
    EXPECT_EQ(sut.historyCapacity, options.historyCapacity);
    EXPECT_EQ(sut.nodeName, nullptr);
    EXPECT_EQ(sut.offerOnCreate, options.offerOnCreate);
    EXPECT_EQ(sut.subscriberTooSlowPolicy, cpp2c::consumerTooSlowPolicy(options.subscriberTooSlowPolicy));
    EXPECT_TRUE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionsInitializationCheckReturnsTrueAfterDefaultInit)
{
    ::testing::Test::RecordProperty("TEST_ID", "e95c8f2b-c15a-4fb5-9de8-719b2c61a926");
    iox_pub_options_t sut;
    iox_pub_options_init(&sut);
    EXPECT_TRUE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionsInitializationCheckReturnsFalseWithoutDefaultInit)
{
    ::testing::Test::RecordProperty("TEST_ID", "f3c7c69e-6946-4da4-9c9e-93129cae9d61");
    iox_pub_options_t sut;
    memset(&sut, 0, sizeof(sut));
    EXPECT_FALSE(iox_pub_options_is_initialized(&sut));
}

TEST(iox_pub_options_test, publisherOptionInitializationWithNullptrDoesNotCrash)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe415d38-eaaf-466e-b7d8-d220612cb344");

    IOX_EXPECT_NO_FATAL_FAILURE([&] { iox_pub_options_init(nullptr); });
}

} // namespace
