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

#include "iceoryx_posh/popo/modern_api/base_publisher.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

template <typename T, typename port_t>
class StubbedBasePublisher : public iox::popo::BasePublisher<T, port_t>
{
  public:
    StubbedBasePublisher(iox::capro::ServiceDescription)
        : iox::popo::BasePublisher<T, port_t>::BasePublisher(){};
    uid_t getUid() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::getUid();
    }
    iox::capro::ServiceDescription getServiceDescription() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::getServiceDescription();
    }
    iox::cxx::expected<iox::popo::Sample<T>, iox::popo::AllocationError> loan(uint64_t size) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::loan(size);
    }
    void release(iox::popo::Sample<T>& sample) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::release(sample);
    }
    void publish(iox::popo::Sample<T>&& sample) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::publish(std::move(sample));
    }
    iox::cxx::optional<iox::popo::Sample<T>> loanPreviousSample() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::loanPreviousSample();
    }
    void offer() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::offer();
    }
    void stopOffer() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::stopOffer();
    }
    bool isOffered() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::isOffered();
    }
    bool hasSubscribers() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::hasSubscribers();
    }
    port_t& getMockedPort()
    {
        return iox::popo::BasePublisher<T, port_t>::m_port;
    }
};

using TestBasePublisher = StubbedBasePublisher<DummyData, MockPublisherPortUser>;

class BasePublisherTest : public Test
{
  public:
    BasePublisherTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    TestBasePublisher sut{{"", "", ""}};
};

TEST_F(BasePublisherTest, LoanForwardsAllocationErrorsToCaller)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(
            ByMove(iox::cxx::error<iox::popo::AllocationError>(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS))));
    // ===== Test ===== //
    auto result = sut.loan(sizeof(DummyData));
    // ===== Verify ===== //
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS, result.get_error());
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, LoanReturnsAllocatedTypedSampleOnSuccess)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk))));
    // ===== Test ===== //
    auto result = sut.loan(sizeof(DummyData));
    // ===== Verify ===== //
    // The memory location of the sample should be the same as the chunk payload.
    EXPECT_EQ(chunk->payload(), result.get_value().get());
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(BasePublisherTest, LoanedSamplesContainPointerToChunkHeader)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk))));
    // ===== Test ===== //
    auto result = sut.loan(sizeof(DummyData));
    // ===== Verify ===== //
    EXPECT_EQ(chunk, result.get_value().getHeader());
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(BasePublisherTest, LoanedSamplesAreAutomaticallyReleasedWhenOutOfScope)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));

    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>(chunk))));

    EXPECT_CALL(sut.getMockedPort(), freeChunk(chunk)).Times(AtLeast(1));

    // ===== Test ===== //
    {
        auto result = sut.loan(sizeof(DummyData));
    }
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(BasePublisherTest, PublishingSendsUnderlyingMemoryChunkOnPublisherPort)
{
    // ===== Setup ===== //
    ON_CALL(sut.getMockedPort(), tryAllocateChunk)
        .WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>())));
    EXPECT_CALL(sut.getMockedPort(), sendChunk).Times(1);
    // ===== Test ===== //
    sut.loan(sizeof(DummyData)).and_then([](iox::popo::Sample<DummyData>& sample) { sample.publish(); });
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, PreviousSampleReturnsSampleWhenPreviousChunkIsRetrievable)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetPreviousChunk)
        .WillOnce(Return(ByMove(iox::cxx::make_optional<iox::mepoo::ChunkHeader*>())));
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

TEST_F(BasePublisherTest, OfferDoesOfferServiceOnUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), offer).Times(1);
    // ===== Test ===== //
    sut.offer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, StopOfferDoesStopOfferServiceOnUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), stopOffer).Times(1);
    // ===== Test ===== //
    sut.stopOffer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, isOfferedDoesCheckIfPortIsOfferedOnUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), isOffered).Times(1);
    // ===== Test ===== //
    sut.isOffered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, isOfferedDoesCheckIfUnderylingPortHasSubscribers)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasSubscribers).Times(1);
    // ===== Test ===== //
    sut.hasSubscribers();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, GetServiceDescriptionCallForwardedToUnderlyingPublisherPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
