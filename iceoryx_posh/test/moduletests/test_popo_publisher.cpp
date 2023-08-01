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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using ::testing::_;

struct DummyData
{
    DummyData() = default;
    DummyData(uint64_t initialVal)
        : val(initialVal)
    {
    }
    // constexpr variable cannot be used instead in tests (linker optimization - no address)
    static constexpr uint64_t defaultVal()
    {
        return 42U;
    };
    uint64_t val{defaultVal()};
};

using TestPublisher = iox::popo::PublisherImpl<DummyData, iox::mepoo::NoUserHeader, MockBasePublisher<DummyData>>;

class PublisherTest : public Test
{
  public:
    PublisherTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    ChunkMock<DummyData> chunkMock;
    TestPublisher sut{{"", "", ""}};
    MockPublisherPortUser& portMock{sut.mockPort()};
};


TEST_F(PublisherTest, LoansChunkLargeEnoughForTheType)
{
    ::testing::Test::RecordProperty("TEST_ID", "38d0779a-1fd5-407d-95aa-2cf24fcf3a09");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    EXPECT_CALL(portMock, releaseChunk(chunkMock.chunkHeader()));
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, LoanedSampleIsDefaultInitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "52b5de5e-be1b-4815-8ac6-45b8dd3e9814");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value()->val, DummyData::defaultVal());
    EXPECT_CALL(portMock, releaseChunk(chunkMock.chunkHeader()));
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, LoanWithArgumentsCallsCustomCtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "1fd165ed-73a2-4465-a740-6d7b502b0d95");
    constexpr uint64_t CUSTOM_VALUE{73};
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loan(CUSTOM_VALUE);
    // ===== Verify ===== //
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(result.value()->val, CUSTOM_VALUE);
    EXPECT_CALL(portMock, releaseChunk(chunkMock.chunkHeader()));
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e341963-5917-440b-b01a-2fc8fff64def");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf(
        [](DummyData* allocation, uint64_t intVal) {
            auto data = new (allocation) DummyData();
            data->val = intVal;
        },
        42U);
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithNoAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "98bf5461-58c6-401d-a599-8e8f4dc5f806");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf([](DummyData* allocation) {
        auto data = new (allocation) DummyData();
        data->val = 777;
    });
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithNoAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5decb3d-3080-4d32-8b5a-2a6e673e17c2");
    struct CallableStruct
    {
        void operator()(DummyData* allocation)
        {
            auto data = new (allocation) DummyData();
            data->val = 777;
        };
    };
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{});
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a14a270-118d-4067-9906-3a608fe046cd");
    struct CallableStruct
    {
        void operator()(DummyData* allocation, uint64_t, float)
        {
            auto data = new (allocation) DummyData();
            data->val = 777;
        };
    };
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{}, 42U, 77.77F);
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

void freeFunctionNoAdditionalArgs(DummyData* allocation)
{
    auto data = new (allocation) DummyData();
    data->val = 777;
}
void freeFunctionWithAdditionalArgs(DummyData* allocation, uint64_t, float)
{
    auto data = new (allocation) DummyData();
    data->val = 777;
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithNoAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "eae5694a-25c3-48ec-b1ac-518321730773");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionNoAdditionalArgs);
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithAdditionalArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "5696d415-1278-4bfe-891f-9c994cd0025e");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionWithAdditionalArgs, 42U, 77.77F);
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, CanLoanSamplesAndPublishCopiesOfProvidedValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "84d2599b-f6b2-497d-b2e9-029b58738552");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    DummyData data(73);
    // ===== Test ===== //
    auto result = sut.publishCopyOf(data);
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, LoanFailsAndForwardsAllocationErrorsToCaller)
{
    ::testing::Test::RecordProperty("TEST_ID", "257750cd-3a1b-4363-a6d2-4318590528bb");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::err(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS))));
    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS, result.error());
    // ===== Cleanup ===== //
}


TEST_F(PublisherTest, LoanedSamplesContainPointerToChunkHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "935108d7-bf2f-4557-8722-f7f474f413a3");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(chunkMock.chunkHeader(), result.value().getChunkHeader());
    EXPECT_CALL(portMock, releaseChunk(chunkMock.chunkHeader()));
    // ===== Cleanup ===== //
}

TEST_F(PublisherTest, PublishingSendsUnderlyingMemoryChunkOnPublisherPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "743183e2-76cb-4d51-9643-a962d933fdac");
    EXPECT_CALL(portMock, tryAllocateChunk(sizeof(DummyData), _, _, _))
        .WillOnce(Return(ByMove(iox::ok(chunkMock.chunkHeader()))));
    EXPECT_CALL(portMock, sendChunk(chunkMock.chunkHeader()));
    // ===== Test ===== //
    sut.loan().and_then([](auto& sample) { sample.publish(); });
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

// test whether the BasePublisher methods are called

TEST_F(PublisherTest, OfferDoesOfferServiceOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "047b39f4-f35d-4b1f-9b27-d58229a89820");
    EXPECT_CALL(sut, offer).Times(1);
    // ===== Test ===== //
    sut.offer();
}
TEST_F(PublisherTest, StopOfferDoesStopOfferServiceOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4bfdd25-3f41-4768-8a6e-2d99dff41257");
    EXPECT_CALL(sut, stopOffer).Times(1);
    sut.stopOffer();
}

TEST_F(PublisherTest, isOfferedDoesCheckIfPortIsOfferedOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "df690425-77ea-43ea-8b4a-2fe0ad6288e7");
    EXPECT_CALL(sut, isOffered).Times(1);
    // ===== Test ===== //
    sut.isOffered();
}

TEST_F(PublisherTest, isOfferedDoesCheckIfUnderylingPortHasSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "aeaa8b55-f814-4073-a173-835df8c84be5");
    EXPECT_CALL(sut, hasSubscribers).Times(1);
    // ===== Test ===== //
    sut.hasSubscribers();
}

TEST_F(PublisherTest, GetServiceDescriptionCallForwardedToUnderlyingPublisherPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "55c87b44-715c-41d6-8004-677e284089c8");
    EXPECT_CALL(sut, getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
}

} // namespace
