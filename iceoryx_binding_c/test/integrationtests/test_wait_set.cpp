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
    void SetUp() override
    {
        iox_wait_set_init(sut, &m_condVar);
    }

    void TearDown() override
    {
        iox_wait_set_deinit(sut);
    }

    sub_t CreateSubscriber()
    {
        const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

        SubscriberPortData* m_portPtr = new SubscriberPortData{
            TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
        return m_portPtr;
    }

    void RemoveSubscriber(sub_t subscriber)
    {
        delete subscriber;
    }

    ConditionVariableData m_condVar;
    struct wait_set_storage_t sutStorage;
    wait_set_t sut = (wait_set_t)&sutStorage;
};

TEST_F(iox_wait_set_test, AttachSingleConditionSuccessfully)
{
    sub_t subscriber = CreateSubscriber();
    //    iox_wait_set_attach_condition(sut, subscriber);
    RemoveSubscriber(subscriber);
}
