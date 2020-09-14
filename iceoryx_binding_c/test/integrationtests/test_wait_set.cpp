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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_wait_set_test : public Test
{
  public:
    class WaitSetMock : public WaitSet
    {
      public:
        WaitSetMock(ConditionVariableData* data)
            : WaitSet(data)
        {
        }
    };

    void SetUp() override
    {
    }

    void TearDown() override
    {
        delete m_sut;
        for (auto s : m_subscriber)
        {
            delete s;
        }
    }

    iox_sub_t CreateSubscriber()
    {
        const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

        cpp2c_Subscriber* subscriber = new cpp2c_Subscriber();
        subscriber->m_portData = new SubscriberPortData{
            TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

        m_subscriber.emplace_back(subscriber);
        return subscriber;
    }

    void RemoveSubscriber(iox_sub_t subscriber)
    {
        delete subscriber->m_portData;
        delete subscriber;
    }

    ConditionVariableData m_condVar;
    iox_wait_set_storage_t m_sutStorage;
    WaitSetMock* m_sut = new WaitSetMock{&m_condVar};
    std::vector<iox_sub_t> m_subscriber;
};

TEST_F(iox_wait_set_test, AttachSingleConditionSuccessfully)
{
    iox_sub_t subscriber = CreateSubscriber();
    EXPECT_THAT(iox_wait_set_attach_condition(m_sut, subscriber), Eq(WaitSetResult_SUCCESS));
}

TEST_F(iox_wait_set_test, AttachSingleConditionTwiceResultsInFailure)
{
    iox_sub_t subscriber = CreateSubscriber();
    iox_wait_set_attach_condition(m_sut, subscriber), Eq(WaitSetResult_SUCCESS);

    EXPECT_THAT(iox_wait_set_attach_condition(m_sut, subscriber), Eq(WaitSetResult_CONDITION_VARIABLE_ALREADY_SET));
}
