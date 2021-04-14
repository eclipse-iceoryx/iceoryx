// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"


#include "test.hpp"

#include <chrono>
#include <stdlib.h>
#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::roudi;
using namespace iox::capro;
using namespace iox::runtime;
using ::testing::Return;

void onSampleReceivedCallback(Subscriber<int>* subscriber)
{
    static_cast<void>(subscriber);
}

class PubSubListener_IntegrationTest : public Test
{
  public:
    PubSubListener_IntegrationTest()
        : m_runtime(PoshRuntime::initRuntime("foo"))
    {
        m_listener = std::make_unique<Listener>();
        m_subscriber = std::make_unique<Subscriber<int>>(m_serviceDescr);
    }
    virtual ~PubSubListener_IntegrationTest()
    {
    }

    void SetUp()
    {
    }
    void TearDown(){};

    ServiceDescription m_serviceDescr{"Radar", "FrontLeft", "Counter"};
    RouDiEnvironment m_roudiEnv;
    PoshRuntime& m_runtime;
    std::unique_ptr<Listener> m_listener;
    std::unique_ptr<Subscriber<int>> m_subscriber;
};

TEST_F(PubSubListener_IntegrationTest, SubscriberGoesOutOfScopeAndDeatchingWorks)
{
    
    m_listener
        ->attachEvent(*m_subscriber,
                      iox::popo::SubscriberEvent::DATA_RECEIVED,
                      iox::popo::createEventCallback(onSampleReceivedCallback))
        .or_else([](auto) { ASSERT_TRUE(false); });

    m_subscriber.reset();

    EXPECT_TRUE(m_listener);
}
