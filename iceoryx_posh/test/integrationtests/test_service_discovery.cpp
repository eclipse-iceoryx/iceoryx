// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include "mocks/posh_runtime_mock.hpp"
#include "test.hpp"

#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;
using iox::capro::IdString_t;
using iox::capro::ServiceDescription;
using iox::roudi::RouDiEnvironment;
using iox::runtime::ServiceContainer;

struct Publisher
{
    using Producer = iox::popo::UntypedPublisher;
    static constexpr MessagingPattern KIND{MessagingPattern::PUB_SUB};
};

struct Server
{
    /// @todo #27 Replace with iox::popo::UntypedServer once available
    using Producer = iox::popo::UntypedPublisher;
    static constexpr MessagingPattern KIND{MessagingPattern::REQ_RES};
};


class ServiceDiscoveryPubSub_test : public RouDi_GTest
{
  public:
    void SetUp() override
    {
        searchResultOfFindServiceWithFindHandler.clear();
        callbackWasCalled = false;
        serviceContainer.clear();
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
    }

    iox::runtime::PoshRuntime* runtime{&iox::runtime::PoshRuntime::initRuntime("Runtime")};
    ServiceDiscovery sut;
    static std::atomic_bool callbackWasCalled;
    static ServiceContainer serviceContainer;
    static ServiceContainer searchResultOfFindServiceWithFindHandler;

    static void findHandler(const ServiceDescription& s)
    {
        searchResultOfFindServiceWithFindHandler.emplace_back(s);
    };

    static void testCallback(ServiceDiscovery* const)
    {
        callbackWasCalled = true;
    }

    static void searchForPublisher(ServiceDiscovery* const serviceDiscovery, ServiceDescription* service)
    {
        serviceContainer = serviceDiscovery->findService(service->getServiceIDString(),
                                                         service->getInstanceIDString(),
                                                         service->getEventIDString(),
                                                         MessagingPattern::PUB_SUB);
        callbackWasCalled = true;
    }

    const iox::units::Duration m_fatalTimeout = 10_s;
    Watchdog m_watchdog{m_fatalTimeout};
};

std::atomic_bool ServiceDiscoveryPubSub_test::callbackWasCalled{false};
ServiceContainer ServiceDiscoveryPubSub_test::serviceContainer;
ServiceContainer ServiceDiscoveryPubSub_test::searchResultOfFindServiceWithFindHandler;

template <typename T>
class ServiceDiscovery_test : public ServiceDiscoveryPubSub_test
{
  public:
    using CommunicationKind = T;
};


void compareAndClearServiceContainers(ServiceContainer& lhs, ServiceContainer& rhs)
{
    const auto size = lhs.size();
    ASSERT_THAT(size, Eq(rhs.size()));
    for (auto i = 0U; i < size; i++)
    {
        EXPECT_THAT(lhs[i], Eq(rhs[i]));
    }
    lhs.clear();
    rhs.clear();
}

///
/// Publish-Subscribe & Request-Response
///

using CommunicationKind = Types<Publisher, Server>;
TYPED_TEST_SUITE(ServiceDiscovery_test, CommunicationKind);

TYPED_TEST(ServiceDiscovery_test, FindServiceReturnsOfferedService)
{
    ::testing::Test::RecordProperty("TEST_ID", "30f0e255-3584-4ab2-b7a6-85c16026852d");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");

    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);

    auto serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                                  SERVICE_DESCRIPTION.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));

    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, ReofferedServiceWithServiceDescriptionCanBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "b67b4990-e2fd-4efa-ab5d-e53c4ee55972");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");
    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);

    auto serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                                  SERVICE_DESCRIPTION.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));
    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    producer.stopOffer();
    this->InterOpWait();
    serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                             SERVICE_DESCRIPTION.getInstanceIDString(),
                                             SERVICE_DESCRIPTION.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(serviceContainer.empty());
    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    producer.offer();
    this->InterOpWait();

    serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                             SERVICE_DESCRIPTION.getInstanceIDString(),
                                             SERVICE_DESCRIPTION.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));
    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, OfferExistingServiceMultipleTimesIsRedundant)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae0790ed-4e1b-4f12-94b3-c9e56433c935");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");
    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);
    producer.offer();
    this->InterOpWait();

    auto serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                                  SERVICE_DESCRIPTION.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));

    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, FindSameServiceMultipleTimesReturnsSingleInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "21948bcf-fe7e-44b4-b93b-f46303e3e050");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");
    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);

    auto serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                                  SERVICE_DESCRIPTION.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));
    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                                             SERVICE_DESCRIPTION.getInstanceIDString(),
                                             SERVICE_DESCRIPTION.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));
    this->sut.findService(SERVICE_DESCRIPTION.getServiceIDString(),
                          SERVICE_DESCRIPTION.getInstanceIDString(),
                          SERVICE_DESCRIPTION.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, OfferDifferentServicesWithSameInstanceAndEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "25bf794d-450e-47ce-a920-ab2ea479af39");

    const IdString_t INSTANCE = "instance";
    const IdString_t EVENT = "event";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1("service1", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("service2", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3("service3", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);

    auto serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION1.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION1));
    this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION2.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION2));

    this->sut.findService(SERVICE_DESCRIPTION2.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION3.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION3));
    this->sut.findService(SERVICE_DESCRIPTION3.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceDoesNotReturnServiceWhenStringsDoNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "1984e907-e990-48b2-8cbd-eab3f67cd162");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1("service1", "instance1", "event1");
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("service2", "instance2", "event2");
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);

    auto serviceContainer = this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                                                  SERVICE_DESCRIPTION1.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION2.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                          SERVICE_DESCRIPTION1.getInstanceIDString(),
                          SERVICE_DESCRIPTION2.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer = this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                                             SERVICE_DESCRIPTION2.getInstanceIDString(),
                                             SERVICE_DESCRIPTION1.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                          SERVICE_DESCRIPTION2.getInstanceIDString(),
                          SERVICE_DESCRIPTION1.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer = this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                                             SERVICE_DESCRIPTION2.getInstanceIDString(),
                                             SERVICE_DESCRIPTION2.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                          SERVICE_DESCRIPTION2.getInstanceIDString(),
                          SERVICE_DESCRIPTION2.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());
}

TYPED_TEST(ServiceDiscovery_test, FindServiceWithInstanceAndEventWildcardReturnsAllMatchingServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e0b1a12-6995-45f4-8fd8-59acbca9bfa8");

    const IdString_t SERVICE = "service";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1(SERVICE, "instance1", "event1");
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2(SERVICE, "instance2", "event2");
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3(SERVICE, "instance3", "event3");
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);

    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back(SERVICE_DESCRIPTION1);
    serviceContainerExp.push_back(SERVICE_DESCRIPTION2);
    serviceContainerExp.push_back(SERVICE_DESCRIPTION3);

    auto serviceContainer = this->sut.findService(
        SERVICE, iox::capro::Wildcard, iox::capro::Wildcard, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(3U));
    EXPECT_TRUE(serviceContainer == serviceContainerExp);

    this->sut.findService(
        SERVICE, iox::capro::Wildcard, iox::capro::Wildcard, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler == serviceContainerExp);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceWithServiceWildcardReturnsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "f4731a52-39d8-4d49-b247-008a2e9181f9");
    const IdString_t INSTANCE = "instance";
    const IdString_t EVENT = "event";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1("service1", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("service2", "another_instance", EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3("service3", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION4("service4", INSTANCE, "another_event");
    typename TestFixture::CommunicationKind::Producer producer_sd4(SERVICE_DESCRIPTION4);

    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back(SERVICE_DESCRIPTION1);
    serviceContainerExp.push_back(SERVICE_DESCRIPTION3);

    auto serviceContainer =
        this->sut.findService(iox::capro::Wildcard, INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(2U));
    EXPECT_TRUE(serviceContainer == serviceContainerExp);

    this->sut.findService(
        iox::capro::Wildcard, INSTANCE, EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler == serviceContainerExp);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceWithEventWildcardReturnsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "e78b35f4-b3c3-4c39-b10a-67712c72e28a");
    const IdString_t SERVICE = "service";
    const IdString_t INSTANCE = "instance";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1(SERVICE, INSTANCE, "event1");
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("another_service", INSTANCE, "event2");
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3(SERVICE, INSTANCE, "event3");
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION4(SERVICE, "another_instance", "event4");
    typename TestFixture::CommunicationKind::Producer producer_sd4(SERVICE_DESCRIPTION4);

    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back(SERVICE_DESCRIPTION1);
    serviceContainerExp.push_back(SERVICE_DESCRIPTION3);

    auto serviceContainer =
        this->sut.findService(SERVICE, INSTANCE, iox::capro::Wildcard, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(2U));
    EXPECT_TRUE(serviceContainer == serviceContainerExp);

    this->sut.findService(
        SERVICE, INSTANCE, iox::capro::Wildcard, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler == serviceContainerExp);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceWithInstanceWildcardReturnsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ec4b422-3ded-4af3-9e72-3b870c55031c");
    const IdString_t SERVICE = "service";
    const IdString_t EVENT = "event";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1(SERVICE, "instance1", EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("another_service", "instance2", EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3(SERVICE, "instance3", EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION4(SERVICE, "instance4", "another_event");
    typename TestFixture::CommunicationKind::Producer producer_sd4(SERVICE_DESCRIPTION4);

    ServiceContainer serviceContainerExp;
    serviceContainerExp.push_back(SERVICE_DESCRIPTION1);
    serviceContainerExp.push_back(SERVICE_DESCRIPTION3);

    auto serviceContainer =
        this->sut.findService(SERVICE, iox::capro::Wildcard, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(2U));
    EXPECT_TRUE(serviceContainer == serviceContainerExp);

    this->sut.findService(
        SERVICE, iox::capro::Wildcard, EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler == serviceContainerExp);
}

TYPED_TEST(ServiceDiscovery_test, OfferSingleServiceMultiInstance)
{
    ::testing::Test::RecordProperty("TEST_ID", "538bec69-ea02-400e-8643-c833d6e84972");

    const IdString_t SERVICE = "service";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1(SERVICE, "instance1", "event1");
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2(SERVICE, "instance2", "event2");
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3(SERVICE, "instance3", "event3");
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);

    auto serviceContainer = this->sut.findService(SERVICE,
                                                  SERVICE_DESCRIPTION1.getInstanceIDString(),
                                                  SERVICE_DESCRIPTION1.getEventIDString(),
                                                  TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION1));
    this->sut.findService(SERVICE,
                          SERVICE_DESCRIPTION1.getInstanceIDString(),
                          SERVICE_DESCRIPTION1.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE,
                                             SERVICE_DESCRIPTION2.getInstanceIDString(),
                                             SERVICE_DESCRIPTION2.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION2));
    this->sut.findService(SERVICE,
                          SERVICE_DESCRIPTION2.getInstanceIDString(),
                          SERVICE_DESCRIPTION2.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE,
                                             SERVICE_DESCRIPTION3.getInstanceIDString(),
                                             SERVICE_DESCRIPTION3.getEventIDString(),
                                             TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION3));
    this->sut.findService(SERVICE,
                          SERVICE_DESCRIPTION3.getInstanceIDString(),
                          SERVICE_DESCRIPTION3.getEventIDString(),
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceReturnsCorrectServiceInstanceCombinations)
{
    ::testing::Test::RecordProperty("TEST_ID", "360839a7-9309-4e7e-8e89-892097a87f7a");

    const IdString_t SERVICE1 = "service1";
    const IdString_t INSTANCE1 = "instance1";
    const IdString_t INSTANCE2 = "instance2";
    const IdString_t EVENT1 = "event1";
    const IdString_t EVENT2 = "event2";
    const IdString_t EVENT3 = "event3";

    const iox::capro::ServiceDescription SERVICE_DESCRIPTION_1_1_1(SERVICE1, INSTANCE1, EVENT1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION_1_1_2(SERVICE1, INSTANCE1, EVENT2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION_1_2_1(SERVICE1, INSTANCE2, EVENT1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION_1_2_2(SERVICE1, INSTANCE2, EVENT2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION_1_2_3(SERVICE1, INSTANCE2, EVENT3);

    typename TestFixture::CommunicationKind::Producer producer_sd_1_1_1(SERVICE_DESCRIPTION_1_1_1);
    typename TestFixture::CommunicationKind::Producer producer_sd_1_1_2(SERVICE_DESCRIPTION_1_1_2);
    typename TestFixture::CommunicationKind::Producer producer_sd_1_2_1(SERVICE_DESCRIPTION_1_2_1);
    typename TestFixture::CommunicationKind::Producer producer_sd_1_2_2(SERVICE_DESCRIPTION_1_2_2);
    typename TestFixture::CommunicationKind::Producer producer_sd_1_2_3(SERVICE_DESCRIPTION_1_2_3);

    auto serviceContainer = this->sut.findService(SERVICE1, INSTANCE1, EVENT1, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION_1_1_1));
    this->sut.findService(SERVICE1, INSTANCE1, EVENT1, this->findHandler, TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE1, INSTANCE1, EVENT2, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));

    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION_1_1_2));
    this->sut.findService(SERVICE1, INSTANCE1, EVENT2, this->findHandler, TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE1, INSTANCE2, EVENT1, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION_1_2_1));
    this->sut.findService(SERVICE1, INSTANCE2, EVENT1, this->findHandler, TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE1, INSTANCE2, EVENT2, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION_1_2_2));
    this->sut.findService(SERVICE1, INSTANCE2, EVENT2, this->findHandler, TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(SERVICE1, INSTANCE2, EVENT3, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION_1_2_3));
    this->sut.findService(SERVICE1, INSTANCE2, EVENT3, this->findHandler, TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

TYPED_TEST(ServiceDiscovery_test, FindServiceDoesNotReturnNotOfferedServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4f99eb1-7496-4a1e-bbd1-ebdb07e1ec9b");

    const IdString_t INSTANCE = "instance";
    const IdString_t EVENT = "event";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION1("service1", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd1(SERVICE_DESCRIPTION1);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION2("service2", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd2(SERVICE_DESCRIPTION2);
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION3("service3", INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd3(SERVICE_DESCRIPTION3);

    producer_sd1.stopOffer();
    producer_sd3.stopOffer();
    this->InterOpWait();

    auto serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION1.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(SERVICE_DESCRIPTION1.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION2.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION2));
    this->sut.findService(SERVICE_DESCRIPTION2.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);

    serviceContainer = this->sut.findService(
        SERVICE_DESCRIPTION3.getServiceIDString(), INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(SERVICE_DESCRIPTION3.getServiceIDString(),
                          INSTANCE,
                          EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());
}

TYPED_TEST(ServiceDiscovery_test, NonExistingServicesAreNotFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "86b87264-4df4-4d20-9357-06391ca1d57f");
    const IdString_t SERVICE = "service";
    const IdString_t NONEXISTENT_SERVICE = "ignatz";
    const IdString_t INSTANCE = "instance";
    const IdString_t NONEXISTENT_INSTANCE = "schlomo";
    const IdString_t EVENT = "event";
    const IdString_t NONEXISTENT_EVENT = "hypnotoad";
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION(SERVICE, INSTANCE, EVENT);
    typename TestFixture::CommunicationKind::Producer producer_sd(SERVICE_DESCRIPTION);

    auto serviceContainer = this->sut.findService(
        NONEXISTENT_SERVICE, NONEXISTENT_INSTANCE, NONEXISTENT_EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(NONEXISTENT_SERVICE,
                          NONEXISTENT_INSTANCE,
                          NONEXISTENT_EVENT,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(NONEXISTENT_SERVICE, NONEXISTENT_INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        NONEXISTENT_SERVICE, NONEXISTENT_INSTANCE, EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(NONEXISTENT_SERVICE, INSTANCE, NONEXISTENT_EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        NONEXISTENT_SERVICE, INSTANCE, NONEXISTENT_EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(NONEXISTENT_SERVICE, INSTANCE, EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        NONEXISTENT_SERVICE, INSTANCE, EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(SERVICE, NONEXISTENT_INSTANCE, NONEXISTENT_EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        SERVICE, NONEXISTENT_INSTANCE, NONEXISTENT_EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(SERVICE, NONEXISTENT_INSTANCE, NONEXISTENT_EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        SERVICE, NONEXISTENT_INSTANCE, NONEXISTENT_EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());

    serviceContainer =
        this->sut.findService(SERVICE, INSTANCE, NONEXISTENT_EVENT, TestFixture::CommunicationKind::KIND);
    EXPECT_THAT(serviceContainer.size(), Eq(0U));
    this->sut.findService(
        SERVICE, INSTANCE, NONEXISTENT_EVENT, this->findHandler, TestFixture::CommunicationKind::KIND);
    EXPECT_TRUE(this->searchResultOfFindServiceWithFindHandler.empty());
}

TYPED_TEST(ServiceDiscovery_test, FindServiceReturnsMaxPublisherServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "68628cc2-df6d-46e4-8586-7563f43bf10c");
    const IdString_t SERVICE = "s";
    constexpr auto MAX_USER_PUBLISHERS = iox::MAX_PUBLISHERS - iox::NUMBER_OF_INTERNAL_PUBLISHERS;

    // if the result size is limited to be lower than the number of publishers in the registry,
    // there is no way to retrieve them all
    constexpr auto NUM_PRODUCERS = std::min(MAX_USER_PUBLISHERS, iox::MAX_FINDSERVICE_RESULT_SIZE);
    iox::cxx::vector<typename TestFixture::CommunicationKind::Producer, NUM_PRODUCERS> producers;

    // we want to check whether we find all the services, including internal ones like introspection,
    // so we first search for them without creating other services to obtain the internal ones
    ServiceContainer serviceContainerExp = this->sut.findService(
        iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, TestFixture::CommunicationKind::KIND);

    for (size_t i = 0; i < NUM_PRODUCERS; i++)
    {
        std::string instance = "i" + iox::cxx::convert::toString(i);
        iox::capro::ServiceDescription SERVICE_DESCRIPTION(
            SERVICE, IdString_t(iox::cxx::TruncateToCapacity, instance), "foo");
        producers.emplace_back(SERVICE_DESCRIPTION);
        serviceContainerExp.push_back(SERVICE_DESCRIPTION);
    }

    // now we should find the maximum number of services we can search for,
    // i.e. internal services and those we just created (iox::MAX_PUBLISHERS combined)
    auto serviceContainer = this->sut.findService(
        iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, TestFixture::CommunicationKind::KIND);

    constexpr auto EXPECTED_NUM_PUBLISHERS = std::min(iox::MAX_PUBLISHERS, iox::MAX_FINDSERVICE_RESULT_SIZE);
    EXPECT_EQ(serviceContainer.size(), EXPECTED_NUM_PUBLISHERS);
    EXPECT_TRUE(serviceContainer == serviceContainerExp);

    this->sut.findService(iox::capro::Wildcard,
                          iox::capro::Wildcard,
                          iox::capro::Wildcard,
                          this->findHandler,
                          TestFixture::CommunicationKind::KIND);
    compareAndClearServiceContainers(serviceContainer, this->searchResultOfFindServiceWithFindHandler);
}

///
/// Publisher-Subscriber
///

TEST_F(ServiceDiscoveryPubSub_test, FindServiceWithWildcardsReturnsOnlyIntrospectionServicesAndServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "d944f32c-edef-44f5-a6eb-c19ee73c98eb");
    auto searchResult = this->sut.findService(
        iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, MessagingPattern::PUB_SUB);


    for (auto& service : searchResult)
    {
        EXPECT_THAT(service.getInstanceIDString().c_str(), StrEq("RouDi_ID"));
    }

    this->sut.findService(
        iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, findHandler, MessagingPattern::PUB_SUB);
    compareAndClearServiceContainers(searchResult, searchResultOfFindServiceWithFindHandler);
}

TEST_F(ServiceDiscoveryPubSub_test, FindServiceWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e1bf253-ce81-47cc-9b4a-605de7e49b64");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("ninjababy", "pow", "pow");
    iox::popo::UntypedPublisher publisher(SERVICE_DESCRIPTION);
    iox::cxx::function_ref<void(const ServiceDescription&)> searchFunction;
    this->sut.findService(
        iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, searchFunction, MessagingPattern::PUB_SUB);
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryIsAttachableToWaitSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc0eeb7a-6f2a-481f-ae8a-1e17460e261f");
    iox::popo::WaitSet<10U> waitSet;

    waitSet
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     0U,
                     iox::popo::createNotificationCallback(testCallback))
        .and_then([]() { GTEST_SUCCEED(); })
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to wait set"; });
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryIsNotifiedbyWaitSetAboutSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1cf36b5-3db2-4e6f-8e05-e7e449530ec0");
    iox::popo::WaitSet<1U> waitSet;

    waitSet
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     0U,
                     iox::popo::createNotificationCallback(testCallback))
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to wait set"; });

    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("Moep", "Fluepp", "Shoezzel");
    iox::popo::UntypedPublisher publisher(SERVICE_DESCRIPTION);

    auto notificationVector = waitSet.timedWait(1_s);

    for (auto& notification : notificationVector)
    {
        (*notification)();
    }

    EXPECT_TRUE(callbackWasCalled);
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryNotifiedbyWaitSetFindsSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ecde7e0-f5b2-4721-b309-66f32f40a7bf");
    iox::popo::WaitSet<1U> waitSet;
    iox::capro::ServiceDescription serviceDescriptionToSearchFor("Soep", "Moemi", "Luela");

    waitSet
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     0U,
                     iox::popo::createNotificationCallback(searchForPublisher, serviceDescriptionToSearchFor))
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to wait set"; });

    iox::popo::UntypedPublisher publisher(serviceDescriptionToSearchFor);

    auto notificationVector = waitSet.timedWait(1_s);

    for (auto& notification : notificationVector)
    {
        (*notification)();
    }

    ASSERT_FALSE(serviceContainer.empty());
    EXPECT_THAT(serviceContainer.front(), Eq(serviceDescriptionToSearchFor));
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryIsAttachableToListener)
{
    ::testing::Test::RecordProperty("TEST_ID", "def201f7-d1bf-4031-8e50-a2ad22ee303c");
    iox::popo::Listener listener;

    listener
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     iox::popo::createNotificationCallback(testCallback))
        .and_then([]() { GTEST_SUCCEED(); })
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to listener"; });
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryIsNotifiedByListenerAboutSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "305107fc-41dd-431c-8032-ed5e82f93038");
    iox::popo::Listener listener;

    listener
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     iox::popo::createNotificationCallback(testCallback))
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to listener"; });

    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("Moep", "Fluepp", "Shoezzel");
    iox::popo::UntypedPublisher publisher(SERVICE_DESCRIPTION);

    while (!callbackWasCalled.load())
    {
        std::this_thread::yield();
    }

    EXPECT_TRUE(callbackWasCalled.load());
}

TEST_F(ServiceDiscoveryPubSub_test, ServiceDiscoveryNotifiedbyListenerFindsSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "b38ba8a4-ff27-437a-b376-13125cb419cb");
    iox::popo::Listener listener;
    iox::capro::ServiceDescription serviceDescriptionToSearchFor("Gimbel", "Seggel", "Doedel");

    listener
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     iox::popo::createNotificationCallback(searchForPublisher, serviceDescriptionToSearchFor))
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to listener"; });

    iox::popo::UntypedPublisher publisher(serviceDescriptionToSearchFor);

    while (!callbackWasCalled.load())
    {
        std::this_thread::yield();
    }

    ASSERT_FALSE(serviceContainer.empty());
    EXPECT_THAT(serviceContainer.front(), Eq(serviceDescriptionToSearchFor));
}

} // namespace
