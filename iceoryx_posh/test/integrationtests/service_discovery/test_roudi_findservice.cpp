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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "test_roudi_service_discovery.hpp"

class RoudiFindService_test : public RouDiServiceDiscoveryTest
{
  public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    iox::runtime::PoshRuntime* senderRuntime{&iox::runtime::PoshRuntime::initRuntime("sender")};
    iox::runtime::PoshRuntime* receiverRuntime{&iox::runtime::PoshRuntime::initRuntime("receiver")};
};

TEST_F(RoudiFindService_test, OfferSingleMethodServiceSingleInstance)
{
    auto isServiceOffered = senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});

    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
    ASSERT_EQ(true, isServiceOffered);
}


TEST_F(RoudiFindService_test, OfferServiceWithDefaultServiceDescriptionFails)
{
    auto isServiceOffered = senderRuntime->offerService(iox::capro::ServiceDescription());
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithAnyServiceIdStringDescriptionFails)
{
    auto isServiceOffered = senderRuntime->offerService(
        iox::capro::ServiceDescription(iox::capro::AnyServiceString, iox::capro::AnyInstanceString));
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithAnyServiceIdDescriptionFails)
{
    auto isServiceOffered =
        senderRuntime->offerService(iox::capro::ServiceDescription(iox::capro::AnyService, iox::capro::AnyInstance));
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithValidEventIdSucessfull)
{
    auto isServiceOffered =
        senderRuntime->offerService(iox::capro::ServiceDescription({"service1", "instance1", "event1"}));
    this->InterOpWait();

    ASSERT_EQ(true, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithInvalidEventIdFails)
{
    auto isServiceOffered = senderRuntime->offerService(
        iox::capro::ServiceDescription({"service1", "instance1", iox::capro::InvalidIDString}));
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithAnyEventIdFails)
{
    auto isServiceOffered =
        senderRuntime->offerService(iox::capro::ServiceDescription(123u, 456u, iox::capro::AnyEvent));
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithAnyEventIdStringFails)
{
    auto isServiceOffered = senderRuntime->offerService(
        iox::capro::ServiceDescription({"service1", "instance1", iox::capro::AnyEventString}));
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, ReofferedServiceWithValidServiceDescriptionCanBeFound)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});

    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
}

TEST_F(RoudiFindService_test, OfferExsistingServiceMultipleTimesIsRedundant)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});

    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
}

TEST_F(RoudiFindService_test, FindSameServiceMultipleTimesReturnsSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service3", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service2", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service3", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceWithDistinctSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance2"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service2", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));

    instanceContainer = receiverRuntime->findService({"service2", "instance2"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance2")));
}

TEST_F(RoudiFindService_test, SubscribeAnyInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service1", "instance2"});
    senderRuntime->offerService({"service1", "instance3"});
    this->InterOpWait();
    InstanceContainer instanceContainerExp;
    InitContainer(instanceContainerExp, {"instance1", "instance2", "instance3"});

    auto instanceContainer = receiverRuntime->findService({"service1", iox::capro::AnyServiceString});

    ASSERT_THAT(instanceContainer.value().size(), Eq(3u));
    EXPECT_TRUE(instanceContainer.value() == instanceContainerExp);
}

TEST_F(RoudiFindService_test, OfferSingleMethodServiceMultiInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service1", "instance2"});
    senderRuntime->offerService({"service1", "instance3"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service1", "instance2"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance2")));

    instanceContainer = receiverRuntime->findService({"service1", "instance3"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance3")));
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

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service1", "instance2"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance2")));

    instanceContainer = receiverRuntime->findService({"service1", "instance3"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance3")));

    instanceContainer = receiverRuntime->findService({"service2", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service2", "instance2"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance2")));

    instanceContainer = receiverRuntime->findService({"service2", "instance3"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance3")));
}

TEST_F(RoudiFindService_test, StopOfferSingleMethodServiceSingleInstance)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));
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

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));

    instanceContainer = receiverRuntime->findService({"service2", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));

    instanceContainer = receiverRuntime->findService({"service3", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));
}

TEST_F(RoudiFindService_test, StopOfferServiceRedundantCall)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});

    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));
}


TEST_F(RoudiFindService_test, StopNonExistingService)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    senderRuntime->stopOfferService({"service2", "instance2"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "instance1"});

    ASSERT_THAT(instanceContainer.value().size(), Eq(1));
    ASSERT_THAT(*instanceContainer.value().begin(), Eq(IdString_t("instance1")));
}

TEST_F(RoudiFindService_test, FindNonExistingServices)
{
    senderRuntime->offerService({"service1", "instance1"});
    senderRuntime->offerService({"service2", "instance1"});
    senderRuntime->offerService({"service3", "instance1"});
    this->InterOpWait();

    auto instanceContainer = receiverRuntime->findService({"service1", "schlomo"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));

    instanceContainer = receiverRuntime->findService({"ignatz", "instance1"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));

    instanceContainer = receiverRuntime->findService({"ignatz", "schlomo"});
    ASSERT_THAT(instanceContainer.value().size(), Eq(0u));
}

TEST_F(RoudiFindService_test, InterfacePort)
{
    senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    auto interfacePortData = receiverRuntime->getMiddlewareInterface(iox::capro::Interfaces::SOMEIP);
    iox::popo::InterfacePort interfacePort(interfacePortData);
    this->InterOpWait();
    bool serviceFound = false;

    while (auto maybeCaProMessage = interfacePort.tryGetCaProMessage())
    {
        auto caproMessage = maybeCaProMessage.value();
        if ((caproMessage.m_serviceDescription.getServiceIDString() == IdString_t("service1"))
            && (caproMessage.m_serviceDescription.getInstanceIDString() == IdString_t("instance1"))
            && ((caproMessage.m_serviceDescription.getEventIDString() == IdString_t(iox::capro::AnyEventString))))
        {
            serviceFound = true;
            break;
        }
    }

    EXPECT_THAT(serviceFound, Eq(true));
}

TEST_F(RoudiFindService_test, findServiceMaxInstances)
{
    size_t noOfInstances = iox::MAX_NUMBER_OF_INSTANCES;
    InstanceContainer instanceContainerExp;
    for (size_t i = 0; i < noOfInstances; i++)
    {
        // Service & Instance string is kept short , to reduce the response size in find service request ,
        // (message queue has a limit of 512)
        std::string instance = "i" + std::to_string(i);
        senderRuntime->offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance)});
        instanceContainerExp.push_back(IdString_t(iox::cxx::TruncateToCapacity, instance));
        this->InterOpWait();
    }

    auto instanceContainer = receiverRuntime->findService({"s", "65535"});

    EXPECT_THAT(instanceContainer.value().size(), Eq(iox::MAX_NUMBER_OF_INSTANCES));
    EXPECT_TRUE(instanceContainer.value() == instanceContainerExp);
    ASSERT_THAT(instanceContainer.has_error(), Eq(false));
}

TEST_F(RoudiFindService_test, findServiceInstanceContainerOverflowError)
{
    size_t noOfInstances = (iox::MAX_NUMBER_OF_INSTANCES + 1);
    InstanceContainer instanceContainerExp;
    for (size_t i = 0; i < noOfInstances; i++)
    {
        std::string instance = "i" + std::to_string(i);
        senderRuntime->offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance)});
        instanceContainerExp.push_back(IdString_t(iox::cxx::TruncateToCapacity, instance));
        this->InterOpWait();
    }

    auto instanceContainer = receiverRuntime->findService({"s", "65535"});

    ASSERT_THAT(instanceContainer.has_error(), Eq(true));
}
