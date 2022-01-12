// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

namespace
{
using iox::capro::IdString_t;
using iox::capro::ServiceDescription;
using iox::runtime::ServiceContainer;

class RoudiFindService_test : public RouDi_GTest
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
    ::testing::Test::RecordProperty("TEST_ID", "30f0e255-3584-4ab2-b7a6-85c16026852d");
    auto isServiceOffered = senderRuntime->offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
    ASSERT_EQ(true, isServiceOffered);
}


TEST_F(RoudiFindService_test, OfferServiceWithDefaultServiceDescriptionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1db1ce50-4e95-46f3-8682-9cc90576dbc0");
    auto isServiceOffered = senderRuntime->offerService(iox::capro::ServiceDescription());
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithValidEventIdSucessfull)
{
    ::testing::Test::RecordProperty("TEST_ID", "1107d0e3-42e1-4b24-9a4d-cef8badb7154");
    auto isServiceOffered = senderRuntime->offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    ASSERT_EQ(true, isServiceOffered);
}

TEST_F(RoudiFindService_test, OfferServiceWithInvalidEventIdFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b67b4990-e2fd-4efa-ab5d-e53c4ee55972");
    auto isServiceOffered =
        senderRuntime->offerService({"service1", iox::capro::InvalidIdString, iox::capro::InvalidIdString});
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(RoudiFindService_test, ReofferedServiceWithValidServiceDescriptionCanBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e3af6f8-7798-4887-8526-f797068492ba");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(RoudiFindService_test, OfferExsistingServiceMultipleTimesIsRedundant)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae0790ed-4e1b-4f12-94b3-c9e56433c935");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(RoudiFindService_test, FindSameServiceMultipleTimesReturnsSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "21948bcf-fe7e-44b4-b93b-f46303e3e050");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "25bf794d-450e-47ce-a920-ab2ea479af39");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service3"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service3", "instance1", "event1"}));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceWithDistinctSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "1984e907-e990-48b2-8cbd-eab3f67cd162");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance2", "event2"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance2", "event2"}));
}

TEST_F(RoudiFindService_test, SubscribeAnyInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e0b1a12-6995-45f4-8fd8-59acbca9bfa8");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance3", "event3"}));
    this->InterOpWait();
    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back({"service1", "instance1", "event1"});
    serviceContainerExp.push_back({"service1", "instance2", "event2"});
    serviceContainerExp.push_back({"service1", "instance3", "event3"});

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), iox::runtime::Wildcard_t());
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(3u));
    EXPECT_TRUE(serviceContainer.value() == serviceContainerExp);
}

TEST_F(RoudiFindService_test, OfferSingleMethodServiceMultiInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "538bec69-ea02-400e-8643-c833d6e84972");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance3", "event3"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance2", "event2"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance3", "event3"}));
}

TEST_F(RoudiFindService_test, OfferMultiMethodServiceMultiInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "360839a7-9309-4e7e-8e89-892097a87f7a");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance3", "event3"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance2", "event2"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance3", "event3"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance2", "event2"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance3", "event3"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance2", "event2"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance3", "event3"}));
}

TEST_F(RoudiFindService_test, StopOfferWithInvalidServiceDescriptionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f758831-674b-4ea2-b5ee-1be0b22d8292");
    EXPECT_FALSE(senderRuntime->stopOfferService(
        {iox::capro::InvalidIdString, iox::capro::InvalidIdString, iox::capro::InvalidIdString}));
}

TEST_F(RoudiFindService_test, StopOfferSingleMethodServiceSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "84676338-d7ea-409e-88c3-22155bababed");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(RoudiFindService_test, StopOfferMultiMethodServiceSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4f99eb1-7496-4a1e-bbd1-ebdb07e1ec9b");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->stopOfferService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = receiverRuntime->findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = receiverRuntime->findService(IdString_t("service3"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(RoudiFindService_test, StopOfferServiceRedundantCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "c41f0a85-5774-45ab-8618-5ea45675e8b2");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}


TEST_F(RoudiFindService_test, StopNonExistingService)
{
    ::testing::Test::RecordProperty("TEST_ID", "de76c8d3-8090-4247-b5d3-d57fb27f2d32");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(senderRuntime->stopOfferService({"service2", "instance2", "event2"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(RoudiFindService_test, FindNonExistingServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "86b87264-4df4-4d20-9357-06391ca1d57f");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(senderRuntime->offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = receiverRuntime->findService(IdString_t("service1"), IdString_t("schlomo"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = receiverRuntime->findService(IdString_t("ignatz"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = receiverRuntime->findService(IdString_t("ignatz"), IdString_t("schlomo"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(RoudiFindService_test, InterfacePort)
{
    ::testing::Test::RecordProperty("TEST_ID", "b455c123-3290-4a72-83ec-6b12da95181e");
    EXPECT_TRUE(senderRuntime->offerService({"service1", "instance1", "event1"}));
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
            && ((caproMessage.m_serviceDescription.getEventIDString() == IdString_t("event1"))))
        {
            serviceFound = true;
            break;
        }
    }

    EXPECT_THAT(serviceFound, Eq(true));
}

TEST_F(RoudiFindService_test, findServiceMaxServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "68628cc2-df6d-46e4-8586-7563f43bf10c");
    ServiceContainer serviceContainerExp;
    for (size_t i = 0; i < iox::MAX_NUMBER_OF_SERVICES; i++)
    {
        // Service & Instance string is kept short , to reduce the response size in find service request ,
        // (message queue has a limit of 512)
        std::string instance = "i" + iox::cxx::convert::toString(i);
        EXPECT_TRUE(senderRuntime->offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"}));
        serviceContainerExp.push_back({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"});
        this->InterOpWait();
    }

    auto serviceContainer = receiverRuntime->findService(IdString_t("s"), iox::runtime::Wildcard_t());

    ASSERT_FALSE(serviceContainer.has_error());
    EXPECT_THAT(serviceContainer.value().size(), Eq(iox::MAX_NUMBER_OF_SERVICES));
    EXPECT_TRUE(serviceContainer.value() == serviceContainerExp);
}

TEST_F(RoudiFindService_test, findServiceserviceContainerOverflowError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2f8d8c0-8712-4e7a-9e33-2b2a918f8a71");
    size_t noOfInstances = (iox::MAX_NUMBER_OF_SERVICES + 1);
    ServiceContainer serviceContainerExp;
    for (size_t i = 0; i < noOfInstances; i++)
    {
        std::string instance = "i" + iox::cxx::convert::toString(i);
        EXPECT_TRUE(senderRuntime->offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"}));
        serviceContainerExp.push_back({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"});
        this->InterOpWait();
    }

    auto serviceContainer = receiverRuntime->findService(IdString_t("s"), iox::runtime::Wildcard_t());

    ASSERT_THAT(serviceContainer.has_error(), Eq(true));
}

} // namespace
