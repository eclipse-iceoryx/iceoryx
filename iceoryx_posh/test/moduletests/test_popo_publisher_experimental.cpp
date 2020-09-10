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

#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include "test.hpp"

#include <iostream>

using namespace ::testing;
using ::testing::_;

// ========================= Test Helpers ========================= //

class MockSenderPort
{
public:
    MOCK_METHOD2(reserveChunk, void(const uint32_t, bool));
    MOCK_METHOD1(deliverChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD1(freeChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD0(activate, void());
    MOCK_METHOD0(deactivate, void());
    MOCK_METHOD0(hasSubscribers, bool());
    MOCK_METHOD0(enableDoDeliverOnSubscription, void());
    MOCK_METHOD0(doesDeliverOnSubscribe, bool());
    MOCK_METHOD0(isPortActive, bool());
    MOCK_METHOD0(isUnique, bool());

};

///
/// @brief The TestBasePublisher class exposes protected methods of BasePublisher so they can be tested.
///
template<typename T, typename port_t>
class TestBasePublisher : protected iox::popo::BasePublisher<T, port_t>
{
public:
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
    iox::cxx::expected<iox::popo::AllocationError> publish(iox::popo::Sample<T>& sample) noexcept
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
};

template<typename T>
using TestPublisher = TestBasePublisher<T, MockSenderPort>;

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

};

// ========================= Tests ========================= //

TEST_F(ExperimentalPublisherTest, OffersIfTryingToPublishBeforeOffer)
{

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





