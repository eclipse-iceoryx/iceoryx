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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include "test.hpp"

#include <iostream>

using namespace ::testing;
using ::testing::_;

// ========================= Test Helpers ========================= //

class MockPublisherPortUser
{
public:
    MockPublisherPortUser(std::nullptr_t)
    {}
    MOCK_METHOD1(allocateChunk, iox::cxx::expected<iox::mepoo::ChunkHeader*, iox::popo::AllocationError>(const uint32_t));
    MOCK_METHOD1(freeChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD1(sendChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD0(getLastChunk, iox::cxx::optional<iox::mepoo::ChunkHeader*>());
    MOCK_METHOD0(offer, void());
    MOCK_METHOD0(stopOffer, void());
    MOCK_METHOD0(isOffered, bool());
    MOCK_METHOD0(hasSubscribers, bool());
};

struct DummyData{
    uint64_t val = 42;
};

#define protected public
#define private public

template<typename T, typename port_t>
class StubbedBasePublisher : public iox::popo::BasePublisher<T, port_t>
{
public:
    StubbedBasePublisher(iox::capro::ServiceDescription sd)
        : iox::popo::BasePublisher<T, port_t>::BasePublisher(sd)
    {};
    uid_t uid() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::uid();
    }
    iox::cxx::expected<iox::popo::Sample<T>, iox::popo::AllocationError> loan(uint64_t size) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::loan(size);
    }
    void release(iox::popo::Sample<T>& sample) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::release(sample);
    }
    void publish(iox::popo::Sample<T>& sample) noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::publish(sample);
    }
    iox::cxx::expected<iox::popo::SampleRecallError> previousSample() const noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::previousSample();
    }
    void offer() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::offer();
    }
    void stopOffer() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::stopOffer();
    }
    bool isOffered() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::isOffered();
    }
    bool hasSubscribers() noexcept
    {
        return iox::popo::BasePublisher<T, port_t>::hasSubscribers();
    }
    port_t& getPort()
    {
        return iox::popo::BasePublisher<T, port_t>::m_port;
    }
};

using TestBasePublisher = StubbedBasePublisher<DummyData, MockPublisherPortUser>;

// ========================= Test Setup ========================= //

class ExperimentalPublisherTest : public Test {

public:
    ExperimentalPublisherTest()
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

// ========================= Tests ========================= //

TEST_F(ExperimentalPublisherTest, OffersIfTryingToPublishBeforeOffering)
{
    ON_CALL(sut.getPort(), allocateChunk).WillByDefault(Return(ByMove(iox::cxx::success<iox::mepoo::ChunkHeader*>())));
    EXPECT_CALL(sut.getPort(), offer).Times(1);

    sut.loan(sizeof(DummyData)).and_then([](iox::popo::Sample<DummyData>& sample){
        sample.publish();
    });
}

TEST_F(ExperimentalPublisherTest, LoanReturnsAllocationErrorIfMempoolExhausted)
{
}

TEST_F(ExperimentalPublisherTest, LoanReturnsAllocationErrorIfNoAppropriatelySizedMemchunks)
{
}

TEST_F(ExperimentalPublisherTest, LoanReturnsAllocatedSampleOnSuccess)
{
}
