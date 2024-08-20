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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include "iox/atomic.hpp"
#include "test.hpp"

#include <random>
#include <set>
#include <type_traits>
#include <vector>


namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::runtime;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::roudi_env;
using iox::capro::IdString_t;
using iox::capro::ServiceDescription;
using iox::popo::MessagingPattern;

using ServiceContainer = std::vector<ServiceDescription>;

static iox::concurrent::Atomic<bool> callbackWasCalled;
ServiceContainer serviceContainer;

// We use various test fixtures to group the tests and
// allow usage of typed tests where appropriate.
// These test fixtures mostly build on each other by inheritance.

class ServiceDiscoveryBase_test : public RouDi_GTest
{
  public:
    ServiceDiscoveryBase_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }

    void findService(const optional<IdString_t>& service,
                     const optional<IdString_t>& instance,
                     const optional<IdString_t>& event,
                     const iox::popo::MessagingPattern pattern) noexcept
    {
        serviceContainer.clear();
        sut.findService(service, instance, event, findHandler, pattern);
    }

    static void findHandler(const ServiceDescription& s)
    {
        serviceContainer.emplace_back(s);
    };


    iox::runtime::PoshRuntime* runtime{&iox::runtime::PoshRuntime::initRuntime("Runtime")};
    ServiceDiscovery sut;
};

template <typename T>
class ServiceDiscovery_test : public ServiceDiscoveryBase_test
{
  public:
    using CommunicationKind = T;

    void SetUp() override
    {
        callbackWasCalled = false;
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
        m_waitset.attachEvent(sut, ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, 0U).or_else([](auto) {
            GTEST_FAIL() << "Could not attach ServiceDiscovery to WaitSet!";
        });
    }

    void findService(const optional<IdString_t>& service,
                     const optional<IdString_t>& instance,
                     const optional<IdString_t>& event) noexcept
    {
        ServiceDiscoveryBase_test::findService(service, instance, event, CommunicationKind::PATTERN);
    }

    void findService(const ServiceDescription& s) noexcept
    {
        findService(s.getServiceIDString(), s.getInstanceIDString(), s.getEventIDString());
    }

    // Used to avoid sleeps in tests if the system behaves correctly.
    // The watchdog ensures that the test will fail if it takes too long, due to
    // timeouts or potential deadlocks.
    iox::popo::WaitSet<1U> m_waitset;
    const iox::units::Duration m_fatalTimeout = 10_s;
    Watchdog m_watchdog{m_fatalTimeout};
};

class ServiceDiscoveryNotification_test : public ServiceDiscoveryBase_test
{
  public:
    void SetUp() override
    {
        callbackWasCalled = false;
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    static void dummyCallback(ServiceDiscovery* const)
    {
        callbackWasCalled = true;
    }

    static void searchForPublisher(ServiceDiscovery* const serviceDiscovery, ServiceDescription* service)
    {
        serviceContainer.clear();
        serviceDiscovery->findService(service->getServiceIDString(),
                                      service->getInstanceIDString(),
                                      service->getEventIDString(),
                                      findHandler,
                                      MessagingPattern::PUB_SUB);
        callbackWasCalled = true;
    }

    const iox::units::Duration m_fatalTimeout = 10_s;
    Watchdog m_watchdog{m_fatalTimeout};
};

struct PubSub
{
    using Producer = iox::popo::UntypedPublisher;
    static constexpr MessagingPattern PATTERN{MessagingPattern::PUB_SUB};
};

struct ReqRes
{
    using Producer = iox::popo::UntypedServer;
    static constexpr MessagingPattern PATTERN{MessagingPattern::REQ_RES};
};

using CommunicationKind = Types<PubSub, ReqRes>;

TYPED_TEST_SUITE(ServiceDiscovery_test, CommunicationKind, );

//
// Built-in topics can be found
// Only PUB/SUB (no REQ/RES built-in topics)
//

TEST_F(ServiceDiscoveryBase_test, FindServiceWithWildcardsReturnsOnlyIntrospectionServicesAndServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "d944f32c-edef-44f5-a6eb-c19ee73c98eb");
    findService(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, MessagingPattern::PUB_SUB);

    constexpr uint32_t NUM_INTERNAL_SERVICES = 6U;
    EXPECT_EQ(serviceContainer.size(), NUM_INTERNAL_SERVICES);
    for (auto& service : serviceContainer)
    {
        EXPECT_THAT(service.getInstanceIDString().c_str(), StrEq("RouDi_ID"));
    }
}

//
// Offer, StopOffer, Reoffer
// Variation of PUB/SUB and REQ/RES
//

// tests with multiple offer stages, we do not vary the search kind here
// but check whether we find the service exactly

TYPED_TEST(ServiceDiscovery_test, ReofferedServiceCanBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "b67b4990-e2fd-4efa-ab5d-e53c4ee55972");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");
    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->findService(SERVICE_DESCRIPTION);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(*serviceContainer.begin(), Eq(SERVICE_DESCRIPTION));

    producer.stopOffer();

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->findService(SERVICE_DESCRIPTION);

    EXPECT_TRUE(serviceContainer.empty());

    producer.offer();

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->findService(SERVICE_DESCRIPTION);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(serviceContainer[0], Eq(SERVICE_DESCRIPTION));
}


TYPED_TEST(ServiceDiscovery_test, ServiceOfferedMultipleTimesCanBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae0790ed-4e1b-4f12-94b3-c9e56433c935");
    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("service", "instance", "event");
    typename TestFixture::CommunicationKind::Producer producer(SERVICE_DESCRIPTION);

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->findService(SERVICE_DESCRIPTION);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(serviceContainer[0], Eq(SERVICE_DESCRIPTION));

    producer.offer();

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->findService(SERVICE_DESCRIPTION);

    ASSERT_THAT(serviceContainer.size(), Eq(1U));
    EXPECT_THAT(serviceContainer[0], Eq(SERVICE_DESCRIPTION));
}

//
// Notification Tests
// Check whether attaching, notification and detaching of waitset and listener works
// Only PUB/SUB
//

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryIsAttachableToWaitSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc0eeb7a-6f2a-481f-ae8a-1e17460e261f");
    iox::popo::WaitSet<10U> waitSet;

    waitSet
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     0U,
                     iox::popo::createNotificationCallback(dummyCallback))
        .and_then([]() { GTEST_SUCCEED(); })
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to wait set"; });
}

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryIsNotifiedbyWaitSetAboutSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1cf36b5-3db2-4e6f-8e05-e7e449530ec0");
    iox::popo::WaitSet<1U> waitSet;

    waitSet
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     0U,
                     iox::popo::createNotificationCallback(dummyCallback))
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

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryNotifiedbyWaitSetFindsSingleService)
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

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryIsAttachableToListener)
{
    ::testing::Test::RecordProperty("TEST_ID", "def201f7-d1bf-4031-8e50-a2ad22ee303c");
    iox::popo::Listener listener;

    listener
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     iox::popo::createNotificationCallback(dummyCallback))
        .and_then([]() { GTEST_SUCCEED(); })
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to listener"; });
}

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryIsNotifiedByListenerAboutSingleService)
{
    ::testing::Test::RecordProperty("TEST_ID", "305107fc-41dd-431c-8032-ed5e82f93038");
    iox::popo::Listener listener;

    listener
        .attachEvent(this->sut,
                     ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                     iox::popo::createNotificationCallback(dummyCallback))
        .or_else([](auto) { GTEST_FAIL() << "Could not attach to listener"; });

    const iox::capro::ServiceDescription SERVICE_DESCRIPTION("Moep", "Fluepp", "Shoezzel");
    iox::popo::UntypedPublisher publisher(SERVICE_DESCRIPTION);

    while (!callbackWasCalled.load())
    {
        std::this_thread::yield();
    }

    EXPECT_TRUE(callbackWasCalled.load());
}

TEST_F(ServiceDiscoveryNotification_test, ServiceDiscoveryNotifiedbyListenerFindsSingleService)
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

//
// FindService Tests
// Check whether findService works in all variations
// Variations: PUB/SUB, REQ/RES, all search scenarios including (partial) wildcards
//

// required to sort and compare result containers
struct Comparator
{
    bool operator()(const ServiceDescription& a, const ServiceDescription& b)
    {
        if (a.getServiceIDString() < b.getServiceIDString())
        {
            return true;
        }
        if (b.getServiceIDString() < a.getServiceIDString())
        {
            return false;
        }

        if (a.getInstanceIDString() < b.getInstanceIDString())
        {
            return true;
        }
        if (b.getInstanceIDString() < a.getInstanceIDString())
        {
            return false;
        }

        if (a.getEventIDString() < b.getEventIDString())
        {
            return true;
        }
        return false;
    }
};

bool sortAndCompare(ServiceContainer& a, ServiceContainer& b)
{
    std::sort(a.begin(), a.end(), Comparator());
    std::sort(b.begin(), b.end(), Comparator());
    return a == b;
}

// Implements a brute force reference discovery algorithm we check against.
// It is fairly trivial and we avoid generating result sets for individual tests
// as it is automated by this class.
//
// An advantage we do not fully explore here is that it can be used in combination with random tests
// 1) generate a random scenario (i.e. register random services)
// 2) run the SUT algorithm and the reference algorithm
// 3) compare results of both algorithms, fail if they are not equal
//
// We mainly do this with handpicked test cases, except for the limit tests where we fill the registry
// to the max.
//
// Note that this depends on correctness of the reference algorithm (which is trivial).
// This is no real restriction since otherwise to check hand-crafted expectations of tests
// (now it suffices to check the reference algorithm).

class ReferenceDiscovery
{
  public:
    std::set<ServiceDescription> services;

    ReferenceDiscovery(const MessagingPattern pattern = MessagingPattern::PUB_SUB)
    {
        if (pattern == MessagingPattern::PUB_SUB)
        {
            services.emplace(iox::roudi::IntrospectionMempoolService);
            services.emplace(iox::roudi::IntrospectionPortService);
            services.emplace(iox::roudi::IntrospectionPortThroughputService);
            services.emplace(iox::roudi::IntrospectionSubscriberPortChangingDataService);
            services.emplace(iox::roudi::IntrospectionProcessService);
            services.emplace(iox::SERVICE_DISCOVERY_SERVICE_NAME,
                             iox::SERVICE_DISCOVERY_INSTANCE_NAME,
                             iox::SERVICE_DISCOVERY_EVENT_NAME);
        }
    }

    bool contains(const ServiceDescription& s)
    {
        return services.find(s) != services.end();
    }

    void add(const ServiceDescription& s)
    {
        services.emplace(s);
    }

    // brute force reference implementation
    ServiceContainer findService(const optional<IdString_t>& service,
                                 const optional<IdString_t>& instance,
                                 const optional<IdString_t>& event)
    {
        ServiceContainer result;
        for (auto& s : services)
        {
            bool match = (service) ? (s.getServiceIDString() == *service) : true;
            match &= (instance) ? (s.getInstanceIDString() == *instance) : true;
            match &= (event) ? (s.getEventIDString() == *event) : true;

            if (match)
            {
                result.emplace_back(s);
            }
        }
        return result;
    }

    auto size()
    {
        return services.size();
    }
};

template <typename T>
class ServiceDiscoveryFindService_test : public ServiceDiscoveryBase_test
{
  public:
    using Variation = T;

    static constexpr uint32_t MAX_PUBLISHERS = iox::MAX_PUBLISHERS;
    static constexpr uint32_t MAX_SERVERS = iox::MAX_SERVERS;

    iox::vector<iox::popo::UntypedPublisher, MAX_PUBLISHERS> publishers;
    iox::vector<iox::popo::UntypedServer, MAX_SERVERS> servers;

    ReferenceDiscovery publisherDiscovery{MessagingPattern::PUB_SUB};
    ReferenceDiscovery serverDiscovery{MessagingPattern::REQ_RES};

    ServiceContainer expectedResult;

    void addPublisher(const ServiceDescription& s)
    {
        publishers.emplace_back(s);
        publisherDiscovery.add(s);
    }

    void addServer(const ServiceDescription& s)
    {
        // servers are unique (contrary to publishers)
        if (!serverDiscovery.contains(s))
        {
            servers.emplace_back(s);
            serverDiscovery.add(s);
        }
    }

    void add(const ServiceDescription& s, MessagingPattern pattern)
    {
        if (pattern == MessagingPattern::PUB_SUB)
        {
            addPublisher(s);
        }
        else
        {
            addServer(s);
        }
    }

    void add(const ServiceDescription& s)
    {
        add(s, Variation::PATTERN);
    }

    void addOther(const ServiceDescription& s)
    {
        add(s, otherPattern());
    }

    void testFindService(const optional<IdString_t>& service,
                         const optional<IdString_t>& instance,
                         const optional<IdString_t>& event) noexcept
    {
        optional<IdString_t> s(service);
        optional<IdString_t> i(instance);
        optional<IdString_t> e(event);

        Variation::setSearchArgs(s, i, e); // modify inputs with (partial) wildcards

        ServiceDiscoveryBase_test::findService(s, i, e, Variation::PATTERN);

        if (Variation::PATTERN == MessagingPattern::PUB_SUB)
        {
            expectedResult = publisherDiscovery.findService(s, i, e);
        }
        else
        {
            expectedResult = serverDiscovery.findService(s, i, e);
        }

        // verify equality of expected (i.e. reference) result and actual result
        EXPECT_TRUE(sortAndCompare(serviceContainer, expectedResult));
    }

    void testFindService(const ServiceDescription& s)
    {
        testFindService(s.getServiceIDString(), s.getInstanceIDString(), s.getEventIDString());
    }

    // assumes we have only two pattern which holds for now
    static constexpr MessagingPattern otherPattern()
    {
        if (Variation::PATTERN == MessagingPattern::PUB_SUB)
        {
            return MessagingPattern::REQ_RES;
        }
        else
        {
            return MessagingPattern::PUB_SUB;
        }
    }

    static constexpr uint32_t maxProducers(MessagingPattern pattern)
    {
        if (pattern == MessagingPattern::PUB_SUB)
        {
            return iox::MAX_PUBLISHERS - iox::NUMBER_OF_INTERNAL_PUBLISHERS;
        }
        else
        {
            return iox::MAX_SERVERS;
        }
    }
};

template <typename T>
T uniform(T n)
{
    static auto seed = std::random_device()();
    static std::mt19937 mt(seed);
    std::uniform_int_distribution<T> dist(0, n);
    return dist(mt);
}

// - create random strings/ids to fill the registry in limit tests
// - has the advantage of automatically generating cases
// - test results should NOT depend on randomness (if they do, we have a bug)
using string_t = iox::capro::IdString_t;

string_t randomString(uint64_t size = string_t::capacity())
{
    // deliberately contains no '0' (need to exclude some char)
    static const char chars[] = "123456789"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz";

    constexpr auto N = string_t::capacity();
    size = std::min(N, size);

    char a[N + 1U];
    for (uint64_t i = 0U; i < size; ++i)
    {
        // -2 is needed to avoid generating the '\0' terminator of chars
        a[i] = chars[uniform(sizeof(chars) - 2U)];
    }
    a[size] = '\0';

    return string_t(a);
}

ServiceDescription randomService()
{
    auto service = randomString();
    auto instance = randomString();
    auto event = randomString();
    return {service, instance, event};
}

ServiceDescription randomService(const string_t& service)
{
    auto instance = randomString();
    auto event = randomString();
    return {service, instance, event};
}

ServiceDescription randomService(const string_t& service, const string_t& instance)
{
    auto event = randomString();
    return {service, instance, event};
}

// define all 8 test variations to search for a service
// (S)ervice, (I)nstance, (E)vent, (W)ildcard

struct SIE
{
    static void setSearchArgs(optional<IdString_t>&, optional<IdString_t>&, optional<IdString_t>&)
    {
    }
};

struct WIE
{
    static void setSearchArgs(optional<IdString_t>& service, optional<IdString_t>&, optional<IdString_t>&)
    {
        service.reset();
    }
};

struct SWE
{
    static void setSearchArgs(optional<IdString_t>&, optional<IdString_t>& instance, optional<IdString_t>&)
    {
        instance.reset();
    }
};

struct SIW
{
    static void setSearchArgs(optional<IdString_t>&, optional<IdString_t>&, optional<IdString_t>& event)
    {
        event.reset();
    }
};

struct WWE
{
    static void setSearchArgs(optional<IdString_t>& service, optional<IdString_t>& instance, optional<IdString_t>&)
    {
        service.reset();
        instance.reset();
    }
};

struct WIW
{
    static void setSearchArgs(optional<IdString_t>& service, optional<IdString_t>&, optional<IdString_t>& event)
    {
        service.reset();
        event.reset();
    }
};

struct SWW
{
    static void setSearchArgs(optional<IdString_t>&, optional<IdString_t>& instance, optional<IdString_t>& event)
    {
        instance.reset();
        event.reset();
    }
};

struct WWW
{
    static void
    setSearchArgs(optional<IdString_t>& service, optional<IdString_t>& instance, optional<IdString_t>& event)
    {
        service.reset();
        instance.reset();
        event.reset();
    }
};

template <typename S, typename T>
struct Variation : public S, public T
{
};


// We could also have the variation of search directly in (a) testFindService method,
// but this would add test interference and make it impossible to compare
// subsequent test results with each other

// define all variations we test
using PS_SIE = Variation<SIE, PubSub>;
using PS_WIE = Variation<WIE, PubSub>;
using PS_SWE = Variation<SWE, PubSub>;
using PS_SIW = Variation<SIW, PubSub>;
using PS_WWE = Variation<WWE, PubSub>;
using PS_WIW = Variation<WIW, PubSub>;
using PS_SWW = Variation<SWW, PubSub>;
using PS_WWW = Variation<WWW, PubSub>;

using RR_SIE = Variation<SIE, ReqRes>;
using RR_WIE = Variation<WIE, ReqRes>;
using RR_SWE = Variation<SWE, ReqRes>;
using RR_SIW = Variation<SIW, ReqRes>;
using RR_WWE = Variation<WWE, ReqRes>;
using RR_WIW = Variation<WIW, ReqRes>;
using RR_SWW = Variation<SWW, ReqRes>;
using RR_WWW = Variation<WWW, ReqRes>;

using TestVariations = Types<PS_SIE,
                             PS_WIE,
                             PS_SWE,
                             PS_SIW,
                             PS_WWE,
                             PS_WIW,
                             PS_SWW,
                             PS_WWW,
                             RR_SIE,
                             RR_WIE,
                             RR_SWE,
                             RR_SIW,
                             RR_WWE,
                             RR_WIW,
                             RR_SWW,
                             RR_WWW>;

// note that testing all variations is costly but ensures coverage of every
// combination of wildcards, communication type and many equivalence classes
// of a search setup, i.e. the existing services in the system

TYPED_TEST_SUITE(ServiceDiscoveryFindService_test, TestVariations, );

// *************************************************************************************************
// All tests run for publishers and servers as well as all 8 search variations.

// Each test works as follows:
// 1) Set up the discovery state by adding services.
// 2) Run the search for specific services using discovery.
// 3) Run the search for the same services using the simple brute force reference implementation.
// 4) Compare results of 2) and 3) (set comparison).
//    If the result sets differ, the entire test fails.

// For brevity, the steps 2), 3) and 4) are always performed by
// calling this->testFindService(SERVICE, INSTANCE, EVENT);
// It is important that for each setup in 1) (i.e. test case)
// each testFindService call is performed with each possible combination of wildcards.

// There is some overlap/redundancy in the tests due to the generative scheme
// that can be considered the price for elegance and structure.
//**************************************************************************************************

TYPED_TEST(ServiceDiscoveryFindService_test, FindWhenNothingOffered)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f0bf2c0-5e96-4da6-b282-f84917bb5243");

    // ensure the discovery loop ran at least once
    this->triggerDiscoveryLoopAndWaitToFinish();

    // Checks whether the reference implementation result matches the sut result if we call
    // findService({"a"}, {"b"}, {"c"}).
    // In this case they both should find nothing (with any parameterization of findService)
    // since nothing was offered.
    this->testFindService({"a"}, {"b"}, {"c"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindWhenSingleServiceOffered)
{
    ::testing::Test::RecordProperty("TEST_ID", "aab09c10-8b1e-4f25-8f72-bd762b69f2cb");
    this->add({"a", "b", "c"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"a"}, {"b"}, {"c"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindWhenSingleServiceIsOfferedMultipleTimes)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c5625d3-49f6-4b8b-a118-6ad850d181ed");
    this->add({"a", "b", "c"});
    this->add({"a", "b", "c"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"a"}, {"b"}, {"c"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindWhenMultipleServicesAreOffered)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea19b805-8572-4891-b3b8-aa988358418e");
    this->add({"a", "b", "c"});
    this->add({"a", "b", "aa"});
    this->add({"aa", "a", "c"});
    this->add({"a", "ab", "a"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"aa"}, {"a"}, {"c"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindWhenMultipleInstancesOfTheSameServiceAreOffered)
{
    ::testing::Test::RecordProperty("TEST_ID", "b026a02b-25b5-481c-9958-57c22bbc20c0");
    this->add({"a", "b", "c"});
    this->add({"a", "d", "c"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"a"}, {"d"}, {"c"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, RepeatedSearchYieldsSameResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1ee22fb-dc40-4011-b054-35a31d7ee5ee");
    this->add({"a", "b", "c"});
    this->add({"a", "b", "aa"});
    this->add({"aa", "a", "c"});
    this->add({"a", "ab", "a"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"a"}, {"b"}, {"aa"});
    auto previousResult = serviceContainer;

    this->testFindService({"a"}, {"b"}, {"aa"});
    sortAndCompare(previousResult, serviceContainer);
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindNonExistingService)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f953d0d-bae3-45a1-82e7-c78a32b6d365");
    this->add({"a", "b", "c"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    // those are all representatives of equivalence classes of mismatches
    // that hould not be found
    this->testFindService({"x"}, {"b"}, {"c"});
    this->testFindService({"a"}, {"x"}, {"c"});
    this->testFindService({"a"}, {"b"}, {"x"});
    this->testFindService({"x"}, {"x"}, {"c"});
    this->testFindService({"a"}, {"x"}, {"x"});
    this->testFindService({"x"}, {"b"}, {"x"});
    this->testFindService({"x"}, {"x"}, {"x"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, FindNonExistingServiceAmongMultipleNearMatches)
{
    ::testing::Test::RecordProperty("TEST_ID", "86b4977f-53c6-495c-add3-524c9f3e0f27");
    // add multiple near matches
    this->add({"x", "b", "c"});
    this->add({"a", "x", "c"});
    this->add({"a", "b", "x"});
    this->add({"x", "x", "c"});
    this->add({"a", "x", "x"});
    this->add({"x", "b", "x"});
    this->add({"x", "x", "x"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"a"}, {"b"}, {"c"});
}

// Limit test of one kind (MessagingPattern) of service:
// Fill the registry to the maximum (limit) and search for specific services.
TYPED_TEST(ServiceDiscoveryFindService_test, FindInMaximumServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "ecdcf06a-c5fa-4e9e-956d-ec40f3d8eaae");
    // maximum number of producers to generate
    auto constexpr MAX = TestFixture::maxProducers(TestFixture::Variation::PATTERN);
    // first threshold to change generation strategy at (about 1/3 of max)
    auto constexpr N1 = MAX / 3;
    // second threshold to change generation strategy at (about 2/3 of max)
    auto constexpr N2 = 2 * N1;

    // phase 1 : generate random services (no Umlaut)
    auto s1 = randomService();
    this->add(s1);
    uint32_t created = 1;

    for (; created < N1; ++created)
    {
        this->add(randomService());
    }

    // create a unique service presented by Umlaut (hence unique)
    ServiceDescription s2{"Ferdinand", "Spitz", "Schnüffler"};
    this->add(s2);
    ++created;

    // phase 2 : fix service, generate random instances (no Umlaut)
    for (; created < N2; ++created)
    {
        this->add(randomService("Ferdinand"));
    }

    // phase 3 : fix service and instance, generate random events (no Umlaut)
    for (; created < MAX - 1; ++created)
    {
        this->add(randomService("Ferdinand", "Spitz"));
    }

    auto s3 = randomService("Ferdinand", "Spitz");
    this->add(s3);
    ++created;

    EXPECT_EQ(created, MAX);

    this->triggerDiscoveryLoopAndWaitToFinish();

    // search for specific services we inserted at various times (includes wildcard searches etc.):
    // find first offered service, last offered service and some service offered inbetween
    this->testFindService(s1);
    this->testFindService(s2);
    this->testFindService(s3);
}

//
// test mixed operation of PUB/SUB and REQ/RES
//

TYPED_TEST(ServiceDiscoveryFindService_test, SameServerAndPublisherCanBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "d75ce3c2-861e-4ae6-8d05-244950a99e8c");
    this->add({"Ferdinand", "Schnüffel", "Spitz"});
    this->addOther({"Ferdinand", "Schnüffel", "Spitz"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"Ferdinand"}, {"Schnüffel"}, {"Spitz"});
}

TYPED_TEST(ServiceDiscoveryFindService_test, OtherServiceKindWithMatchingNameIsNotFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "e182e255-b17e-445a-ad1b-a05b643b30fd");
    this->add({"Schnüffel", "Ferdinand", "Spitz"});
    this->addOther({"Ferdinand", "Schnüffel", "Spitz"});

    this->triggerDiscoveryLoopAndWaitToFinish();

    this->testFindService({"Ferdinand"}, {"Schnüffel"}, {"Spitz"});
}

// Limit test of both kinds (MessagingPattern) of service
//
// 1) fill the discovery to the max with PUB/SUB and REQ/RES services
// 2) vary services instances and events create a general scenario that can occur in practice:
//    e.g. same service with many instances, same instance with many events etc.
// As such, the tests check whether service discovery works at system limits,
// i.e. findService works for a full service registry in a general case scenario
TYPED_TEST(ServiceDiscoveryFindService_test, FindInMaximumMixedServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0b292c2-49ce-47b4-b38e-456732240c41");
    // maximum number of producers to generate
    auto constexpr MAX = TestFixture::maxProducers(TestFixture::Variation::PATTERN);
    // first threshold to change generation strategy at (about 1/3 of max)
    auto constexpr N1 = MAX / 3;
    // second threshold to change generation strategy at (about 2/3 of max)
    auto constexpr N2 = 2 * N1;

    // phase 1 : generate random services (no Umlaut)
    auto s1 = randomService();
    this->add(s1);

    uint32_t created = 1;

    for (; created < N1; ++created)
    {
        this->add(randomService());
    }

    // create a unique service presented by Umlaut (hence unique)
    ServiceDescription s2{"Ferdinand", "Spitz", "Schnüffler"};
    this->add(s2);
    ++created;

    // phase 2 : fix service, generate random instances (no Umlaut)
    for (; created < N2; ++created)
    {
        this->add(randomService("Ferdinand"));
    }

    // phase 3 : fix service and instance, generate random events (no Umlaut)
    for (; created < MAX - 1; ++created)
    {
        this->add(randomService("Ferdinand", "Spitz"));
    }

    auto s3 = randomService("Ferdinand", "Spitz");
    this->add(s3);
    ++created;

    EXPECT_EQ(created, MAX);

    // create some services of the other kind
    created = 0;

    auto constexpr OTHER_MAX = TestFixture::maxProducers(TestFixture::otherPattern());

    // same phases, but now for the other service type
    // note that thresholds N1, N2 are chosen in a way that we can reuse them
    for (; created < N1; ++created)
    {
        this->addOther(randomService());
    }

    for (; created < N2; ++created)
    {
        this->addOther(randomService("Spitz"));
    }

    for (; created < OTHER_MAX; ++created)
    {
        this->addOther(randomService("Ferdinand", "Spitz"));
    }

    EXPECT_EQ(created, OTHER_MAX);

    this->triggerDiscoveryLoopAndWaitToFinish();

    // now we have the maximum of services of both kinds with semi-random services

    // search for specific services we inserted at various times (includes wildcard searches etc.):
    // find first offered service, last offered service and some service offered inbetween
    this->testFindService(s1);
    this->testFindService(s2);
    this->testFindService(s3);
}

} // namespace
