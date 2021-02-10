// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/typed_publisher.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

using TestTypedPublisher = iox::popo::TypedPublisher<DummyData, MockBasePublisher<DummyData>>;

class TypedPublisherTest : public Test
{
  public:
    TypedPublisherTest()
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
    // std::function<void(DummyData* const)> placeholderDeleter = [](DummyData* const) {};
    TestTypedPublisher sut{{"", "", ""}};
    MockPublisherPortUser& mockPort{sut.mockPort()};
};


TEST_F(TypedPublisherTest, LoansChunksLargeEnoughForTheType)
{
    EXPECT_CALL(mockPort, tryAllocateChunk(sizeof(DummyData)))
        .WillOnce(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    EXPECT_FALSE(result.has_error());
    // ===== Cleanup ===== //
    EXPECT_CALL(mockPort, releaseChunk(chunkMock.chunkHeader()));
}

#if 0

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithAdditionalArguments)
{
    // ===== Setup ===== //
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(
        [](DummyData* allocation, int intVal) {
            auto data = new (allocation) DummyData();
            data->val = intVal;
        },
        42);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithNoAdditionalArguments)
{
    // ===== Setup ===== //
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf([](DummyData* allocation) {
        auto data = new (allocation) DummyData();
        data->val = 777;
    });
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithNoAdditionalArguments)
{
    // ===== Setup ===== //
    struct CallableStruct
    {
        void operator()(DummyData* allocation)
        {
            auto data = new (allocation) DummyData();
            data->val = 777;
        };
    };
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{});
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithAdditionalArguments)
{
    // ===== Setup ===== //
    struct CallableStruct
    {
        void operator()(DummyData* allocation, int, float)
        {
            auto data = new (allocation) DummyData();
            data->val = 777;
        };
    };
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{}, 42, 77.77);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

void freeFunctionNoAdditionalArgs(DummyData* allocation)
{
    auto data = new (allocation) DummyData();
    data->val = 777;
}
void freeFunctionWithAdditionalArgs(DummyData* allocation, int, float)
{
    auto data = new (allocation) DummyData();
    data->val = 777;
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithNoAdditionalArguments)
{
    // ===== Setup ===== //
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionNoAdditionalArgs);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithAdditionalArguments)
{
    // ===== Setup ===== //
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionWithAdditionalArgs, 42, 77.77);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedPublisherTest, CanLoanSamplesAndPublishCopiesOfProvidedValues)
{
    // ===== Setup ===== //
    auto sample = iox::popo::Sample<DummyData>(
        {static_cast<DummyData*>(chunkMock.chunkHeader()->payload()), placeholderDeleter}, sut);
    auto data = DummyData();
    data.val = 777;
    EXPECT_CALL(sut, loan).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<DummyData>>(std::move(sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishCopyOf(data);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
}



TEST_F(BasePublisherTest, LoanForwardsAllocationErrorsToCaller)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(
            ByMove(iox::cxx::error<iox::popo::AllocationError>(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS))));
    // ===== Test ===== //
    auto result = sut.loanSample(sizeof(DummyData));
    // ===== Verify ===== //
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS, result.get_error());
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, LoanReturnsAllocatedTypedSampleOnSuccess)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loanSample(sizeof(DummyData));
    // ===== Verify ===== //
    // The memory location of the sample should be the same as the chunk payload.
    EXPECT_EQ(chunkMock.chunkHeader()->payload(), result.value().get());
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, LoanedSamplesContainPointerToChunkHeader)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loanSample(sizeof(DummyData));
    // ===== Verify ===== //
    EXPECT_EQ(chunkMock.chunkHeader(), result.value().getHeader());
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, LoanedSamplesAreAutomaticallyReleasedWhenOutOfScope)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));

    EXPECT_CALL(sut.getMockedPort(), releaseChunk(chunkMock.chunkHeader())).Times(AtLeast(1));

    // ===== Test ===== //
    {
        auto result = sut.loanSample(sizeof(DummyData));
    }
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, PublishingSendsUnderlyingMemoryChunkOnPublisherPort)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));
    EXPECT_CALL(sut.getMockedPort(), sendChunk).Times(1);
    // ===== Test ===== //
    sut.loanSample(sizeof(DummyData)).and_then([](auto& sample) { sample.publish(); });
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, PreviousSampleReturnsSampleWhenPreviousChunkIsRetrievable)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetPreviousChunk)
        .WillOnce(Return(ByMove(iox::cxx::make_optional<iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader()))));
    // ===== Test ===== //
    auto result = sut.loanPreviousSample();
    // ===== Verify ===== //
    EXPECT_EQ(true, result.has_value());
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, PreviousSampleReturnsEmptyOptionalWhenChunkNotRetrievable)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetPreviousChunk).WillOnce(Return(ByMove(iox::cxx::nullopt)));
    // ===== Test ===== //
    auto result = sut.loanPreviousSample();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_value());
    // ===== Cleanup ===== //
}

#endif
