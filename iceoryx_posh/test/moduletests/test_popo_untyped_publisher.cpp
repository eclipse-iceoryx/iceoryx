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

#include "iceoryx_posh/popo/modern_api/untyped_publisher.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using TestUntypedPublisher = iox::popo::UntypedPublisherImpl<MockBasePublisher<void>>;

class UntypedPublisherTest : public Test
{
  public:
    UntypedPublisherTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    TestUntypedPublisher sut{{"", "", ""}};
};

TEST_F(UntypedPublisherTest, GetsUIDViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getUid).Times(1);
    // ===== Test ===== //
    sut.getUid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedPublisherTest, LoansViaBasePublisher)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::Sample<void>(
        iox::cxx::unique_ptr<void>(reinterpret_cast<void*>(chunk->payload()), [](void* const) {} // Placeholder deleter.
                                   ),
        sut);
    EXPECT_CALL(sut, loan(42)).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<void>>(std::move(*sample)))));
    // ===== Test ===== //
    sut.loan(42);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(UntypedPublisherTest, PublishesSampleViaBasePublisher)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::Sample<void>(
        iox::cxx::unique_ptr<void>(reinterpret_cast<void*>(chunk->payload()), [](void* const) {} // Placeholder deleter.
                                   ),
        sut);
    EXPECT_CALL(sut, loan(42)).WillOnce(Return(ByMove(iox::cxx::success<iox::popo::Sample<void>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto loanResult = sut.loan(42);
    sut.publish(std::move(loanResult.get_value()));
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(UntypedPublisherTest, PublishesVoidPointerViaUnderlyingPort)
{
    // ===== Setup ===== //
    void* chunk = iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader));
    EXPECT_CALL(sut.m_port, sendChunk).Times(1); // m_port is mocked.
    // ===== Test ===== //
    sut.publish(chunk);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(UntypedPublisherTest, GetsPreviousSampleViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, loanPreviousSample).Times(1);
    // ===== Test ===== //
    sut.loanPreviousSample();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedPublisherTest, OffersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, offer).Times(1);
    // ===== Test ===== //
    sut.offer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedPublisherTest, StopsOffersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, stopOffer).Times(1);
    // ===== Test ===== //
    sut.stopOffer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedPublisherTest, checksIfOfferedViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, isOffered).Times(1);
    // ===== Test ===== //
    sut.isOffered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedPublisherTest, ChecksIfHasSubscribersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasSubscribers).Times(1);
    // ===== Test ===== //
    sut.hasSubscribers();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
