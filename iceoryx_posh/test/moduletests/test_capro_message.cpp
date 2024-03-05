// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;

class CaproMessage_test : public Test
{
};


TEST_F(CaproMessage_test, CTorSetsParametersCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "76ac087b-c931-4c96-8e6e-0490c97d4994");
    IdString_t testServiceID{"1"};
    IdString_t testEventID{"2"};
    IdString_t testInstanceID{"3"};
    ServiceDescription sd(testServiceID, testEventID, testInstanceID);
    iox::popo::SubscriberPortData recData{sd,
                                          "foo",
                                          iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                          iox::popo::VariantQueueTypes::FiFo_MultiProducerSingleConsumer,
                                          iox::popo::SubscriberOptions()};

    CaproMessage testObj(CaproMessageType::OFFER, sd, CaproServiceType::PUBLISHER, &recData);

    EXPECT_EQ(&recData, testObj.m_chunkQueueData);
    EXPECT_EQ(CaproMessageType::OFFER, testObj.m_type);
    EXPECT_EQ(CaproServiceType::PUBLISHER, testObj.m_serviceType);
    EXPECT_EQ(0U, testObj.m_historyCapacity);
    EXPECT_EQ(sd, testObj.m_serviceDescription);
}


TEST_F(CaproMessage_test, DefaultArgsOfCtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "9192864e-3713-402e-9d92-1a5e803a93ee");
    CaproMessage testObj(CaproMessageType::OFFER, ServiceDescription("1", "2", "3"));

    EXPECT_EQ(CaproServiceType::NONE, testObj.m_serviceType);
    EXPECT_EQ(nullptr, testObj.m_chunkQueueData);
}

} // namespace
