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

#include "iceoryx_posh/experimental/popo/typed_publisher.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

using TestTypedPublisher = iox::popo::TypedPublisher<DummyData, MockBasePublisher<DummyData>>;

class ExperimentalTypedPublisherTest : public Test
{
  public:
    ExperimentalTypedPublisherTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    TestTypedPublisher sut{{"", "", ""}};
};

TEST_F(ExperimentalTypedPublisherTest, LoansSamplesLargeEnoughForTheType)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan(sizeof(DummyData)))
        .WillOnce(Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));

    // ===== Test ===== //
    auto result = sut.loan();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, GetsUIDViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, uid).Times(1);
    // ===== Test ===== //
    sut.uid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalTypedPublisherTest, PublishesSampleViaBasePublisher)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto loanResult = sut.loan();
    sut.publish(std::move(loanResult.get_value()));
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithAdditionalArguments)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(
        [](DummyData* allocation, int, float) {
            auto data = new (allocation) DummyData();
            data->val = 777;
        },
        42,
        77.77);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfALambdaWithNoAdditionalArguments)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf([](DummyData* allocation) {
        auto data = new (allocation) DummyData();
        data->val = 777;
    });
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithNoAdditionalArguments)
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
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{});
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfACallableStructWithAdditionalArguments)
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
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(CallableStruct{}, 42, 77.77);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
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

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithNoAdditionalArguments)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionNoAdditionalArgs);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishTheResultOfFunctionPointerWithAdditionalArguments)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishResultOf(freeFunctionWithAdditionalArgs, 42, 77.77);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, CanLoanSamplesAndPublishCopiesOfProvidedValues)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    auto sample = new iox::popo::PublishableSample<DummyData>(
        iox::cxx::unique_ptr<DummyData>(reinterpret_cast<DummyData*>(reinterpret_cast<DummyData*>(chunk->payload())),
                                        [](DummyData* const) {} // Placeholder deleter.
                                        ),
        sut);
    auto data = DummyData();
    data.val = 777;
    EXPECT_CALL(sut, loan).WillOnce(
        Return(ByMove(iox::cxx::success<iox::popo::PublishableSample<DummyData>>(std::move(*sample)))));
    EXPECT_CALL(sut, publishMocked).Times(1);
    // ===== Test ===== //
    auto result = sut.publishCopyOf(data);
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    // ===== Cleanup ===== //
    delete sample;
    iox::cxx::alignedFree(chunk);
}

TEST_F(ExperimentalTypedPublisherTest, GetsPreviousSampleViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, loanPreviousSample).Times(1);
    // ===== Test ===== //
    sut.loanPreviousSample();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalTypedPublisherTest, OffersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, offer).Times(1);
    // ===== Test ===== //
    sut.offer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalTypedPublisherTest, StopsOffersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, stopOffer).Times(1);
    // ===== Test ===== //
    sut.stopOffer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalTypedPublisherTest, checksIfOfferedViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, isOffered).Times(1);
    // ===== Test ===== //
    sut.isOffered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalTypedPublisherTest, ChecksIfHasSubscribersViaBasePublisher)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasSubscribers).Times(1);
    // ===== Test ===== //
    sut.hasSubscribers();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
