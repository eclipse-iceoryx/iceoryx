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

#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/experimental/popo/subscriber.hpp"

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ========================= Simulated Shared Memory ========================= //
static constexpr uint32_t NUM_CHUNKS_IN_POOL = 3 * iox::MAX_SUBSCRIBER_QUEUE_CAPACITY;
static constexpr uint32_t SMALL_CHUNK = 128;
static constexpr uint32_t CHUNK_META_INFO_SIZE = 256;
static constexpr size_t MEMORY_SIZE = NUM_CHUNKS_IN_POOL * (SMALL_CHUNK + CHUNK_META_INFO_SIZE);
alignas(64) static uint8_t s_memory[MEMORY_SIZE];


// ========================= Helpers ========================= //
struct Position {
    Position(double_t x, double_t y, double_t z) : x(x), y(y), z(z)
    {};
    double_t x = 0.0;
    double_t y = 0.0;
    double_t z = 0.0;
};

// ========================= Test Setup ========================= //

class PubSubExperimentalIntegrationTest : public Test {

public:

    PubSubExperimentalIntegrationTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

protected:
    iox::posix::Allocator m_memoryAllocator{s_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolConfig;
    iox::mepoo::MemoryManager m_memoryManager;

};

// ========================= Tests ========================= //

TEST_F(PubSubExperimentalIntegrationTest, DataTransferFromPublisherToSubscriber)
{

    iox::popo::TypedPublisher<Position> typedPublisher{{"Odometry", "Position", "Vehicle"}};
    typedPublisher.offer();

    iox::popo::TypedSubscriber<Position> typedSubscriber{{"Odometry", "Position", "Vehicle"}};
    typedSubscriber.subscribe(10);

}
