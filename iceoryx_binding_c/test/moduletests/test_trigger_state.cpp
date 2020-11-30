// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/trigger_state.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::cxx;
using namespace iox::posix;


extern "C" {
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/trigger_state.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_trigger_state_test : public Test
{
  public:
    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

    void SetUp()
    {
        m_lastTriggerCallbackArgument = nullptr;
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
        m_subscriber.m_portData = &m_portPtr;
    }

    void TearDown()
    {
    }

    void Subscribe(SubscriberPortData* ptr)
    {
        uint64_t queueCapacity = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY;
        iox_sub_subscribe(m_subscriberHandle, queueCapacity);

        SubscriberPortSingleProducer(ptr).tryGetCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
        SubscriberPortSingleProducer(ptr).dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    static void triggerCallback(UserTrigger* arg)
    {
        m_lastTriggerCallbackArgument = arg;
    }


    static UserTrigger* m_lastTriggerCallbackArgument;
    ConditionVariableData m_condVar;
    WaitSetMock m_waitSet{&m_condVar};
    UserTrigger m_userTrigger;

    static constexpr uint32_t NUM_CHUNKS_IN_POOL = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + 2;
    static constexpr uint32_t CHUNK_SIZE = 128;
    static constexpr size_t MEMORY_SIZE = 1024 * 1024 * 100;
    uint8_t m_memory[MEMORY_SIZE];
    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;
    iox::popo::SubscriberPortData m_portPtr{
        TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePusher<SubscriberPortData::ChunkQueueData_t> m_chunkPusher{&m_portPtr.m_chunkReceiverData};
    cpp2c_Subscriber m_subscriber;
    iox_sub_t m_subscriberHandle = &m_subscriber;
};

UserTrigger* iox_trigger_state_test::m_lastTriggerCallbackArgument = nullptr;

TEST_F(iox_trigger_state_test, triggerStateHasCorrectId)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, nullptr);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    ASSERT_THAT(triggerStateVector.size(), Eq(1));
    EXPECT_EQ(iox_trigger_state_get_trigger_id(&triggerStateVector[0]), 123);
}

TEST_F(iox_trigger_state_test, triggerOriginIsUserTriggerPointerWhenItsOriginatingFromThem)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, nullptr);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_does_originate_from_user_trigger(&triggerStateVector[0], &m_userTrigger), true);
}

TEST_F(iox_trigger_state_test, triggerOriginIsNotUserTriggerPointerWhenItsNotOriginatingFromThem)
{
    iox_sub_attach_to_ws(m_subscriberHandle, &m_waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 587, NULL);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.tryPush(m_memoryManager.getChunk(100));

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_does_originate_from_user_trigger(&triggerStateVector[0], &m_userTrigger), false);
}

TEST_F(iox_trigger_state_test, triggerOriginIsSubscriberPointerWhenItsOriginatingFromThem)
{
    iox_sub_attach_to_ws(m_subscriberHandle, &m_waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 587, NULL);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.tryPush(m_memoryManager.getChunk(100));

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_does_originate_from_subscriber(&triggerStateVector[0], m_subscriberHandle), true);
}

TEST_F(iox_trigger_state_test, triggerOriginIsNotSubscriberPointerWhenItsOriginatingFromThem)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, nullptr);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_does_originate_from_subscriber(&triggerStateVector[0], m_subscriberHandle), false);
}


TEST_F(iox_trigger_state_test, getOriginReturnsPointerToUserTriggerWhenOriginatingFromThem)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, nullptr);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_get_user_trigger_origin(&triggerStateVector[0]), &m_userTrigger);
}

TEST_F(iox_trigger_state_test, getOriginReturnsNullptrUserTriggerWhenNotOriginatingFromThem)
{
    iox_sub_attach_to_ws(m_subscriberHandle, &m_waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 587, NULL);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.tryPush(m_memoryManager.getChunk(100));

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_get_user_trigger_origin(&triggerStateVector[0]), nullptr);
}


TEST_F(iox_trigger_state_test, getOriginReturnsPointerToSubscriberWhenOriginatingFromThem)
{
    iox_sub_attach_to_ws(m_subscriberHandle, &m_waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 587, NULL);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.tryPush(m_memoryManager.getChunk(100));

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_get_subscriber_origin(&triggerStateVector[0]), m_subscriberHandle);
}

TEST_F(iox_trigger_state_test, getOriginReturnsNullptrSubscriberWhenNotOriginatingFromThem)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, iox_trigger_state_test::triggerCallback);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    EXPECT_EQ(iox_trigger_state_get_subscriber_origin(&triggerStateVector[0]), nullptr);
}

TEST_F(iox_trigger_state_test, callbackCanBeCalledOnce)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, iox_trigger_state_test::triggerCallback);
    m_userTrigger.trigger();

    auto triggerStateVector = m_waitSet.wait();

    iox_trigger_state_call(&triggerStateVector[0]);
    EXPECT_EQ(m_lastTriggerCallbackArgument, &m_userTrigger);
}

TEST_F(iox_trigger_state_test, callbackCanBeCalledMultipleTimes)
{
    m_userTrigger.attachToWaitset(m_waitSet, 123, iox_trigger_state_test::triggerCallback);
    m_userTrigger.trigger();
    auto triggerStateVector = m_waitSet.wait();

    iox_trigger_state_call(&triggerStateVector[0]);
    m_lastTriggerCallbackArgument = nullptr;
    iox_trigger_state_call(&triggerStateVector[0]);
    m_lastTriggerCallbackArgument = nullptr;
    iox_trigger_state_call(&triggerStateVector[0]);
    m_lastTriggerCallbackArgument = nullptr;
    iox_trigger_state_call(&triggerStateVector[0]);

    EXPECT_EQ(m_lastTriggerCallbackArgument, &m_userTrigger);
}

