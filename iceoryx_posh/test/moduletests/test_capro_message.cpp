// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;

class CaproMessage_test : public Test
{
  public:
    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
};


TEST_F(CaproMessage_test, CTorSetsParametersCorrectly)
{
    IdString_t testServiceID{"1"};
    IdString_t testEventID{"2"};
    IdString_t testInstanceID{"3"};
    ServiceDescription sd(testServiceID, testEventID, testInstanceID);
    iox::popo::SubscriberPortData recData{
        sd, "foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer, iox::popo::SubscriberOptions()};

    CaproMessage testObj(CaproMessageType::OFFER, sd, CaproMessageSubType::SERVICE, &recData);

    EXPECT_EQ(&recData, testObj.m_chunkQueueData);
    EXPECT_EQ(CaproMessageType::OFFER, testObj.m_type);
    EXPECT_EQ(CaproMessageSubType::SERVICE, testObj.m_subType);
    EXPECT_EQ(0U, testObj.m_historyCapacity);
    EXPECT_EQ(sd, testObj.m_serviceDescription);
}


TEST_F(CaproMessage_test, DefaultArgsOfCtor)
{
    CaproMessage testObj(CaproMessageType::OFFER, ServiceDescription("1", "2", "3"));

    EXPECT_EQ(CaproMessageSubType::NOSUBTYPE, testObj.m_subType);
    EXPECT_EQ(nullptr, testObj.m_chunkQueueData);
}

} // namespace
