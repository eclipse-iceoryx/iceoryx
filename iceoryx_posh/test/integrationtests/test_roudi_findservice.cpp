// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "roudi_gtest.hpp"

using IdString = iox::capro::ServiceDescription::IdString;

class RoudiFindService_test : public RouDi_GTest
{
  public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    iox::runtime::PoshRuntime* senderRuntime{&iox::runtime::PoshRuntime::getInstance("/sender")};
    iox::runtime::PoshRuntime* receiverRuntime{&iox::runtime::PoshRuntime::getInstance("/receiver")};
};

TEST_F(RoudiFindService_test, OfferSingleMethodServiceSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);

    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));
}

TEST_F(RoudiFindService_test, DISABLED_OfferMultiMethodServiceSingleInstance_PERFORMANCETEST42)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service3", "instance1"});

    this->InterOpWait();
    iox::runtime::InstanceContainer instanceContainer;

    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service2", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service3", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));
}

TEST_F(RoudiFindService_test, OfferSingleMethodServiceMultiInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service1", "instance2"});
    senderRuntime->offerService({"service1", "instance3"});
    this->InterOpWait();

    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service1", "instance2"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance2")));

    instanceContainer.clear();
    receiverRuntime->findService({"service1", "instance3"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance3")));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceMultiInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service1", "instance2"});
    senderRuntime->offerService({"service1", "instance3"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service2", "instance2"});
    senderRuntime->offerService({"service2", "instance3"});
    this->InterOpWait();

    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service1", "instance2"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance2")));

    instanceContainer.clear();
    receiverRuntime->findService({"service1", "instance3"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance3")));

    instanceContainer.clear();
    receiverRuntime->findService({"service2", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service2", "instance2"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance2")));

    instanceContainer.clear();
    receiverRuntime->findService({"service2", "instance3"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance3")));
}

TEST_F(RoudiFindService_test, StopOfferSingleMethodServiceSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));
}

TEST_F(RoudiFindService_test, StopOfferMultiMethodServiceSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service3", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    senderRuntime->stopOfferService({"service3", "instance1"});
    this->InterOpWait();

    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));

    instanceContainer.clear();
    receiverRuntime->findService({"service2", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.begin(), Eq(IdString("instance1")));

    instanceContainer.clear();
    receiverRuntime->findService({"service3", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));
}
TEST_F(RoudiFindService_test, FindNonExistingServices)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service3", "instance1"});
    this->InterOpWait();

    iox::runtime::InstanceContainer instanceContainer;
    receiverRuntime->findService({"service1", "schlomo"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));

    receiverRuntime->findService({"ignatz", "instance1"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));

    receiverRuntime->findService({"ignatz", "schlomo"}, instanceContainer);
    ASSERT_THAT(instanceContainer.size(), Eq(0u));
}
TEST_F(RoudiFindService_test, InterfacePort)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto interfacePortData = receiverRuntime->getMiddlewareInterface(iox::Interfaces::SOMEIP);
    iox::popo::InterfacePort interfacePort(interfacePortData);

    iox::capro::CaproMessage caproMessage;
    this->InterOpWait();

    bool serviceFound = false;

    while (interfacePort.getCaProMessage(caproMessage))
    {
        if ((caproMessage.m_serviceDescription.getServiceIDString() == "service1")
            && (caproMessage.m_serviceDescription.getInstanceIDString() == "instance1")
            && ((caproMessage.m_serviceDescription.getEventIDString() == iox::capro::AnyEventString)))
        {
            serviceFound = true;
            break;
        }
    }
    EXPECT_THAT(serviceFound, Eq(true));
}
