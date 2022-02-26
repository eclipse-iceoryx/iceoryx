// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;
using ::testing::_;

struct DummyData
{
    DummyData() = default;
    uint64_t val{42};
};
struct DummyHeader
{
    DummyHeader() = default;
    uint64_t counter = 0;
};

using SutProducerType = Sample<DummyData, DummyHeader>;
using SutConsumerType = Sample<const DummyData, const DummyHeader>;

class MockPublisherInterface : public PublisherInterface<DummyData, DummyHeader>
{
  public:
    void publish(SutProducerType&& sample) noexcept override
    {
        auto s = std::move(sample); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(s));
    }

    MOCK_METHOD(void, mockSend, (SutProducerType &&), (noexcept));
};

class Sample_test : public Test
{
  public:
    MockPublisherInterface mockInterface;
    ChunkMock<DummyData, DummyHeader> sampleMock;
    SutProducerType sutProducer{iox::cxx::unique_ptr<DummyData>(sampleMock.sample(), [](DummyData*) {}), mockInterface};
    SutConsumerType sutConsumer{iox::cxx::unique_ptr<const DummyData>(sampleMock.sample(), [](const DummyData*) {})};
};

TEST_F(Sample_test, SendCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ddbbcad-704f-4f0a-849c-5db8ac339668");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    sutProducer.publish();

    EXPECT_THAT(sutProducer.get(), Eq(nullptr));
}

TEST_F(Sample_test, SendOnMoveDestinationCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "74a62eae-d36f-47bf-9df9-695e50fcdd88");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    auto movedSut = std::move(sutProducer);
    movedSut.publish();

    EXPECT_THAT(sutProducer.get(), Eq(nullptr));
}

TEST_F(Sample_test, SendCallsInterfaceMockWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e0e7b2b-c417-4f77-b999-22c7bd4342ea");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    sutProducer.publish();

    EXPECT_THAT(sutProducer.get(), Eq(nullptr));
}

TEST_F(Sample_test, PublishingAlreadyPublishedSampleCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b0302c9-814a-4b03-813a-fd5586fc987c");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    sutProducer.publish();

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const auto errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });

    sutProducer.publish();

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

TEST_F(Sample_test, PublishingMovedSampleCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c3a9a19-0581-4e47-aed7-f55892bef7fa");

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const auto errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });

    auto movedSut = std::move(sutProducer);
    sutProducer.publish();

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

} // namespace
