// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_discovery.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include "mocks/posh_runtime_mock.hpp"
#include "test.hpp"


#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::cxx;
using iox::roudi::RouDiEnvironment;
using iox::capro::IdString_t;
using iox::capro::ServiceDescription;
using iox::runtime::ServiceContainer;

class PoshDiscovery_test : public RouDi_GTest
{
  public:
    PoshDiscovery_test()
    {
    }

    virtual ~PoshDiscovery_test()
    {
    }

    virtual void SetUp()
    {
        testing::internal::CaptureStdout();
    };

    virtual void TearDown()
    {
        std::string output = testing::internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    };

    void InterOpWait()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    iox::runtime::PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime("Runtime")};
    iox::runtime::PoshRuntime* m_runtimeBob{&iox::runtime::PoshRuntime::initRuntime("Bob")}; // move to 2nd thread
    PoshDiscovery m_sut;
};

TIMING_TEST_F(PoshDiscovery_test, GetServiceRegistryChangeCounterOfferStopOfferService, Repeat(5), [&] {
    auto serviceCounter = m_sut.getServiceRegistryChangeCounter();
    auto initialCout = serviceCounter->load();

    m_sut.offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 1 == serviceCounter->load());

    m_sut.stopOfferService({"service1", "instance1", "event1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 2 == serviceCounter->load());
});

TEST_F(PoshDiscovery_test, OfferEmptyServiceIsInvalid)
{
    auto isServiceOffered = m_sut.offerService(iox::capro::ServiceDescription());

    EXPECT_FALSE(isServiceOffered);
}

TEST_F(PoshDiscovery_test, FindServiceWithWildcardsReturnsOnlyIntrospectionServices)
{
    EXPECT_FALSE(m_sut.offerService(iox::capro::ServiceDescription()));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(iox::runtime::Wildcard_t(), iox::runtime::Wildcard_t());
    ASSERT_FALSE(serviceContainer.has_error());

    auto searchResult = serviceContainer.value();

    for (auto& service : searchResult)
    {
        EXPECT_THAT(service.getServiceIDString().c_str(), StrEq("Introspection"));
    }
}

TEST_F(PoshDiscovery_test, OfferSingleMethodServiceSingleInstance)
{
    auto isServiceOffered = m_sut.offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
    ASSERT_EQ(true, isServiceOffered);
}


TEST_F(PoshDiscovery_test, OfferServiceWithDefaultServiceDescriptionFails)
{
    auto isServiceOffered = m_sut.offerService(iox::capro::ServiceDescription());
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(PoshDiscovery_test, OfferServiceWithValidEventIdSucessfull)
{
    auto isServiceOffered = m_sut.offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    ASSERT_EQ(true, isServiceOffered);
}

TEST_F(PoshDiscovery_test, OfferServiceWithInvalidEventIdFails)
{
    auto isServiceOffered =
        m_sut.offerService({"service1", iox::capro::InvalidIdString, iox::capro::InvalidIdString});
    this->InterOpWait();

    ASSERT_EQ(false, isServiceOffered);
}

TEST_F(PoshDiscovery_test, ReofferedServiceWithValidServiceDescriptionCanBeFound)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(PoshDiscovery_test, OfferExsistingServiceMultipleTimesIsRedundant)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));

    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(PoshDiscovery_test, FindSameServiceMultipleTimesReturnsSingleInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(PoshDiscovery_test, OfferMultiMethodServiceSingleInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service3"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service3", "instance1", "event1"}));
}

TEST_F(PoshDiscovery_test, OfferMultiMethodServiceWithDistinctSingleInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance2", "event2"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance2", "event2"}));
}

TEST_F(PoshDiscovery_test, SubscribeAnyInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance3", "event3"}));
    this->InterOpWait();
    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back({"service1", "instance1", "event1"});
    serviceContainerExp.push_back({"service1", "instance2", "event2"});
    serviceContainerExp.push_back({"service1", "instance3", "event3"});

    auto serviceContainer = m_sut.findService(IdString_t("service1"), iox::runtime::Wildcard_t());
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(3u));
    EXPECT_TRUE(serviceContainer.value() == serviceContainerExp);
}

TEST_F(PoshDiscovery_test, OfferSingleMethodServiceMultiInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance3", "event3"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance2", "event2"}));

    serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance3", "event3"}));
}

TEST_F(PoshDiscovery_test, OfferMultiMethodServiceMultiInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance2", "event2"}));
    EXPECT_TRUE(m_sut.offerService({"service1", "instance3", "event3"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance2", "event2"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance3", "event3"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance2", "event2"}));

    serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance3", "event3"}));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance2"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance2", "event2"}));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance3"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance3", "event3"}));
}

TEST_F(PoshDiscovery_test, StopOfferWithInvalidServiceDescriptionFails)
{
    EXPECT_FALSE(m_sut.stopOfferService(
        {iox::capro::InvalidIdString, iox::capro::InvalidIdString, iox::capro::InvalidIdString}));
}

TEST_F(PoshDiscovery_test, StopOfferSingleMethodServiceSingleInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(PoshDiscovery_test, StopOfferMultiMethodServiceSingleInstance)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.stopOfferService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = m_sut.findService(IdString_t("service2"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1u));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service2", "instance1", "event1"}));

    serviceContainer = m_sut.findService(IdString_t("service3"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(PoshDiscovery_test, StopOfferServiceRedundantCall)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}


TEST_F(PoshDiscovery_test, StopNonExistingService)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();
    EXPECT_TRUE(m_sut.stopOfferService({"service2", "instance2", "event2"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(1));
    ASSERT_THAT(*serviceContainer.value().begin(), Eq(ServiceDescription{"service1", "instance1", "event1"}));
}

TEST_F(PoshDiscovery_test, FindNonExistingServices)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service2", "instance1", "event1"}));
    EXPECT_TRUE(m_sut.offerService({"service3", "instance1", "event1"}));
    this->InterOpWait();

    auto serviceContainer = m_sut.findService(IdString_t("service1"), IdString_t("schlomo"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = m_sut.findService(IdString_t("ignatz"), IdString_t("instance1"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));

    serviceContainer = m_sut.findService(IdString_t("ignatz"), IdString_t("schlomo"));
    ASSERT_FALSE(serviceContainer.has_error());
    ASSERT_THAT(serviceContainer.value().size(), Eq(0u));
}

TEST_F(PoshDiscovery_test, InterfacePort)
{
    EXPECT_TRUE(m_sut.offerService({"service1", "instance1", "event1"}));
    this->InterOpWait();

    /// @todo call the c'tor of PoshRuntime from a different thread
    auto interfacePortData = m_runtimeBob->getMiddlewareInterface(iox::capro::Interfaces::SOMEIP);
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

TEST_F(PoshDiscovery_test, findServiceMaxServices)
{
    ServiceContainer serviceContainerExp;
    for (size_t i = 0; i < iox::MAX_NUMBER_OF_SERVICES; i++)
    {
        // Service & Instance string is kept short , to reduce the response size in find service request ,
        // (message queue has a limit of 512)
        std::string instance = "i" + iox::cxx::convert::toString(i);
        EXPECT_TRUE(
            m_sut.offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"}));
        serviceContainerExp.push_back({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"});
        this->InterOpWait();
    }

    auto serviceContainer = m_sut.findService(IdString_t("s"), iox::runtime::Wildcard_t());

    ASSERT_FALSE(serviceContainer.has_error());
    EXPECT_THAT(serviceContainer.value().size(), Eq(iox::MAX_NUMBER_OF_SERVICES));
    EXPECT_TRUE(serviceContainer.value() == serviceContainerExp);
}

TEST_F(PoshDiscovery_test, findServiceserviceContainerOverflowError)
{
    size_t noOfInstances = (iox::MAX_NUMBER_OF_SERVICES + 1);
    ServiceContainer serviceContainerExp;
    for (size_t i = 0; i < noOfInstances; i++)
    {
        std::string instance = "i" + iox::cxx::convert::toString(i);
        EXPECT_TRUE(
            m_sut.offerService({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"}));
        serviceContainerExp.push_back({"s", IdString_t(iox::cxx::TruncateToCapacity, instance), "foo"});
        this->InterOpWait();
    }

    auto serviceContainer = m_sut.findService(IdString_t("s"), iox::runtime::Wildcard_t());

    ASSERT_THAT(serviceContainer.has_error(), Eq(true));
}

} // namespace
