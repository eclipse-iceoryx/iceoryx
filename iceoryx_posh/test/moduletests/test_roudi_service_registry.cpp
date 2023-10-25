// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iox/optional.hpp"
#include "iox/std_string_support.hpp"
#include "iox/string.hpp"

#include "test.hpp"

#include <chrono>
#include <random>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::roudi;
using iox::optional;
using iox::capro::IdString_t;

using iox::capro::ServiceDescription;
using SearchResult_t = std::vector<ServiceRegistry::ServiceDescriptionEntry>;

struct PublisherTest
{
    auto add(const ServiceDescription& sd)
    {
        return registry.addPublisher(sd);
    }

    auto otherAdd(const ServiceDescription& sd)
    {
        return registry.addServer(sd);
    }

    auto remove(const ServiceDescription& sd)
    {
        return registry.removePublisher(sd);
    }

    auto count(ServiceRegistry::ServiceDescriptionEntry& entry)
    {
        return entry.publisherCount;
    }

    auto filter(const SearchResult_t& result)
    {
        SearchResult_t filtered;
        for (auto& entry : result)
        {
            if (entry.publisherCount > 0)
            {
                filtered.emplace_back(entry);
            }
        }
        return filtered;
    }

    auto operator->()
    {
        return &registry;
    }

    iox::roudi::ServiceRegistry registry;
};

struct ServerTest
{
    auto add(const ServiceDescription& sd)
    {
        return registry.addServer(sd);
    }

    auto otherAdd(const ServiceDescription& sd)
    {
        return registry.addPublisher(sd);
    }

    auto remove(const ServiceDescription& sd)
    {
        return registry.removeServer(sd);
    }

    auto count(ServiceRegistry::ServiceDescriptionEntry& entry)
    {
        return entry.serverCount;
    }

    auto filter(const SearchResult_t& result)
    {
        SearchResult_t filtered;
        for (auto& entry : result)
        {
            if (entry.serverCount > 0)
            {
                filtered.emplace_back(entry);
            }
        }
        return filtered;
    }

    auto operator->()
    {
        return &registry;
    }

    iox::roudi::ServiceRegistry registry;
};

template <typename Sut>
class ServiceRegistry_test : public Test
{
  public:
    using ServiceDescription = iox::capro::ServiceDescription;
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    uint64_t countServices()
    {
        uint64_t count = 0;
        sut->forEach([&](const ServiceRegistry::ServiceDescriptionEntry&) { ++count; });
        return count;
    }

    void
    find(const optional<IdString_t>& service, const optional<IdString_t>& instance, const optional<IdString_t>& event)
    {
        searchResult.clear();
        auto findHandler = [&](const ServiceRegistry::ServiceDescriptionEntry& entry) {
            searchResult.push_back(entry);
        };
        sut->find(service, instance, event, findHandler);
    }

    SearchResult_t searchResult;
    Sut sut;
};

constexpr auto CAPACITY = ServiceRegistry::CAPACITY;

typedef ::testing::Types<PublisherTest, ServerTest> TestTypes;

TYPED_TEST_SUITE(ServiceRegistry_test, TestTypes, );

TYPED_TEST(ServiceRegistry_test, AddNoServiceDescriptionsAndWildcardSearchReturnsNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a050209-01d8-4d0e-9e70-c0662b9dbe76");
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, AddMaximumNumberOfServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa7d6416-4183-4942-a323-01f78c1bb6c1");
    iox::vector<ServiceDescription, CAPACITY> services;

    for (uint64_t i = 0U; i < CAPACITY; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::into<iox::lossy<iox::capro::IdString_t>>(iox::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = this->sut.add(service);
        ASSERT_FALSE(result.has_error());
    }
}

TYPED_TEST(ServiceRegistry_test, AddMoreThanMaximumNumberOfServiceDescriptionsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a911f654-8314-4ea3-b9b2-1afa121a2b21");
    iox::vector<ServiceDescription, CAPACITY> services;

    for (uint64_t i = 0U; i < CAPACITY; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::into<iox::lossy<iox::capro::IdString_t>>(iox::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = this->sut.add(service);
        ASSERT_FALSE(result.has_error());
    }

    auto result = this->sut.add(iox::capro::ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ServiceRegistry::Error::SERVICE_REGISTRY_FULL));
}

TYPED_TEST(ServiceRegistry_test, AddServiceDescriptionsWhichWasAlreadyAddedAndReturnsOneResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "79234425-98ce-49eb-bf04-82883ee22a92");
    auto result1 = this->sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = this->sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result2.has_error());

    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaer")));
    EXPECT_THAT(this->sut.count(this->searchResult[0]), Eq(2));
}

TYPED_TEST(ServiceRegistry_test, AddServiceDescriptionsTwiceAndRemoveOnceAndReturnsOneResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f8193ea-2d36-423f-a658-1dba30c1868d");
    auto result1 = this->sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = this->sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result2.has_error());

    this->sut.remove(ServiceDescription("Li", "La", "Launebaerli"));

    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaerli")));
    EXPECT_THAT(this->sut.count(this->searchResult[0]), Eq(1));
}

TYPED_TEST(ServiceRegistry_test, AddServiceDescriptionsTwiceAndPurgeReturnsNoResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "3185b67f-b891-4a82-8f91-047e059ed68f");
    auto result1 = this->sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = this->sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result2.has_error());

    this->sut->purge(ServiceDescription("Li", "La", "Launebaerli"));

    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, AddEmptyServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cff55b0-d12f-48f5-8f0c-6501d0c2bf79");
    auto result = this->sut.add(ServiceDescription());
    ASSERT_FALSE(result.has_error());
}

TYPED_TEST(ServiceRegistry_test, RemovingServiceDescriptionsWhichWasntAddedFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf3e39f5-c29d-4b5f-8e01-af69681b2ea8");
    this->sut.remove(ServiceDescription("Sim", "Sa", "Lambim"));
    EXPECT_THAT(this->countServices(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, RemovingEmptyServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "523f2320-c0e5-4590-a1b0-604e756ecaa5");
    ASSERT_FALSE(this->sut.add(ServiceDescription()).has_error());
    this->sut.remove(ServiceDescription());
    EXPECT_THAT(this->countServices(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, SingleEmptyServiceDescriptionsCanBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "be3e4b13-d930-47b2-aeaa-95c65f06deed");
    ASSERT_FALSE(this->sut.add(ServiceDescription()).has_error());
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription()));
}

TYPED_TEST(ServiceRegistry_test, SingleEmptyServiceDescriptionsCanBeFoundWithEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "1af0137f-6bf5-422e-a6ec-513f7d3f6191");
    ASSERT_FALSE(this->sut.add(ServiceDescription()).has_error());
    this->find(iox::capro::IdString_t(""), iox::capro::IdString_t(""), iox::capro::IdString_t(""));

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription()));
}

TYPED_TEST(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "a12c9d39-6379-4296-8987-df56a87169f7");
    auto result = this->sut.add(ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_FALSE(result.has_error());
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription("Foo", "Bar", "Baz")));
}

TYPED_TEST(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithEventName)
{
    ::testing::Test::RecordProperty("TEST_ID", "6df8fd7d-e4d1-4c51-8ad7-2fbe82e4ed09");
    iox::capro::ServiceDescription service("a", "b", "c");
    ASSERT_FALSE(this->sut.add(service).has_error());
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::IdString_t("c"));

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service));
}

TYPED_TEST(ServiceRegistry_test, ServiceDescriptionNotFoundWhenEventDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba7785d1-08ec-4f7c-b341-dff033dae2c7");
    iox::capro::ServiceDescription service("Besser", "Wisser", "Girl");
    ASSERT_FALSE(this->sut.add(service).has_error());
    this->find(iox::capro::IdString_t("Besser"), iox::capro::IdString_t("Wisser"), iox::capro::IdString_t("Boy"));

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithInstanceName)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dfaa836-f1da-466f-a794-a7ac1b599ce3");
    auto result = this->sut.add(ServiceDescription("Baz", "Bar", "Foo"));
    ASSERT_FALSE(result.has_error());
    this->find(iox::capro::Wildcard, iox::capro::IdString_t("Bar"), iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(ServiceDescription("Baz", "Bar", "Foo")));
}

TYPED_TEST(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithServiceName)
{
    ::testing::Test::RecordProperty("TEST_ID", "0890013c-e14b-4ae2-89cb-757624c12b4e");
    iox::capro::ServiceDescription service("a", "b", "c");
    ASSERT_FALSE(this->sut.add(service).has_error());
    this->find(iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service));
}

TYPED_TEST(ServiceRegistry_test, EmptyAndNotEmptyServiceDescriptionsCanAllBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f34e604-5217-4e47-9c6f-26c5cbdcd3ec");
    ServiceDescription service1;
    ServiceDescription service2("alpha", "bravo", "charlie");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(2));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service1));
    EXPECT_THAT(this->searchResult[1].serviceDescription, Eq(service2));
}

TYPED_TEST(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameCanAllBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "251fe262-2e8f-4e32-a5f1-a4b1aaa812fd");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());
    this->find(iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(3));

    bool hasFoundB = false;
    bool hasFoundC = false;
    bool hasFoundD = false;

    for (auto& e : this->searchResult)
    {
        if (e.serviceDescription == service1)
            hasFoundB = true;
        if (e.serviceDescription == service2)
            hasFoundC = true;
        if (e.serviceDescription == service3)
            hasFoundD = true;
    }

    EXPECT_THAT(hasFoundB && hasFoundC && hasFoundD, Eq(true));
}

TYPED_TEST(ServiceRegistry_test, MultipleServiceDescriptionWithDifferentServiceNameCanAllBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b7a0897-46a7-457e-8746-0c3899e96653");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("c", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    this->find(iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service1));
    this->searchResult.clear();

    this->find(iox::capro::IdString_t("c"), iox::capro::Wildcard, iox::capro::Wildcard);
    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service2));
}

TYPED_TEST(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameFindsSpecificService)
{
    ::testing::Test::RecordProperty("TEST_ID", "557d6533-25a3-4c0c-adc0-e8ebb74785a0");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());
    this->find(iox::capro::IdString_t("a"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));

    ASSERT_THAT(this->searchResult.size(), Eq(1));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service2));
}

TYPED_TEST(ServiceRegistry_test, MultipleServiceDescriptionAddedInNonLinearOrderFindsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "26187db1-299a-494c-bd85-eb646b8cf67b");
    iox::capro::ServiceDescription service1("a", "1", "moep");
    iox::capro::ServiceDescription service2("b", "2", "moep");
    iox::capro::ServiceDescription service3("c", "3", "moep");
    iox::capro::ServiceDescription service4("d", "4", "moep");
    iox::capro::ServiceDescription service5("e", "5", "moep");

    ASSERT_FALSE(this->sut.add(service5).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());
    ASSERT_FALSE(this->sut.add(service4).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service1).has_error());

    this->sut.remove(service5);
    this->sut.remove(service1);
    EXPECT_THAT(this->countServices(), Eq(3));
    this->find(iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, FindSpecificNonExistingServiceDescriptionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "183600f6-3d2d-4c0f-8c1d-d74acfb7fc50");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());
    this->find(iox::capro::IdString_t("a"), iox::capro::IdString_t("g"), iox::capro::IdString_t("f"));

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, AddingMultipleServiceDescriptionWithSameServicesAndRemovingSpecificDoesNotFindSpecific)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8230904-c14e-4e9f-a324-f92c67522271");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());

    this->sut.remove(service2);
    EXPECT_THAT(this->countServices(), Eq(2));

    this->find(iox::capro::IdString_t("a"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));
    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, ServiceNotFoundAfterAddingAndRemovingToServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5e67b1f-65f2-4973-9f76-73479dd1de9e");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("c", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());

    this->sut.remove(service2);
    EXPECT_THAT(this->countServices(), Eq(2));

    this->find(iox::capro::IdString_t("b"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));
    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, AddingMultipleServiceDescriptionAndRemovingAllDoesNotFindAnything)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7a7e160-813c-4daf-8c55-d0a85bb5f642");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());

    this->sut.remove(service1);
    this->sut.remove(service2);
    this->sut.remove(service3);

    this->find(iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);
    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

template <typename T>
T uniform(T max)
{
    static auto seed = std::random_device()();
    static std::mt19937 mt(seed);
    std::uniform_int_distribution<T> dist(0, max);
    return dist(mt);
}

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

TYPED_TEST(ServiceRegistry_test, CanAddMaximumNumberOfDifferentServiceDescriptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "76aef6cb-7886-4d64-9188-09bd1be2d335");
    uint32_t numEntriesAdded = 0U;
    do
    {
        // may (rarely) generate duplicates to be counted internally
        auto id = randomString();
        ServiceDescription sd(id, id, id);
        auto result = this->sut.add(sd);
        if (result.has_error())
        {
            break;
        }
        numEntriesAdded++;
    } while (true);

    // duplicates do not count to the max and may be generated randomly,
    // but for the contract we only need to guarantee that we can at least addPublisher
    // the configured max
    constexpr auto MAX = ServiceRegistry::CAPACITY;
    EXPECT_GE(numEntriesAdded, MAX);
}

TYPED_TEST(ServiceRegistry_test, SearchInFullRegistryWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c4519d4-1873-4837-a3eb-0367106fb9b5");

    constexpr auto CAP = string_t::capacity();

    string_t fixedId = iox::into<iox::lossy<string_t>>(std::string(CAP, '0'));

    ServiceDescription lastAdded;
    do
    {
        auto id = randomString();
        ServiceDescription sd(fixedId, fixedId, id);

        auto result = this->sut.add(sd);
        if (result.has_error())
        {
            break;
        }
        lastAdded = sd;
    } while (true);

    // removePublisher the last and replace it with a unique service description
    this->sut->purge(lastAdded);

    // is unique (random does not generate 0s) and last if a vector is used internally
    // for almost worst case search (search on last string will terminate early whp)
    auto id = randomString(CAP - 1);
    id.unsafe_append("0");

    ServiceDescription uniqueSd(fixedId, fixedId, id);
    auto result = this->sut.add(uniqueSd);
    EXPECT_FALSE(result.has_error());

    auto& service = uniqueSd.getServiceIDString();
    auto& instance = uniqueSd.getInstanceIDString();
    auto& event = uniqueSd.getEventIDString();

    // This is close to a worst case search but not quite due to randomness in the last string.
    // Different strings are required as we need different strings to create a full registry,
    // and randomness is the easiest way to achieve this.
    // It could also be achieved with determinstic string enumeration instead of randomness,
    // but it is more cumbersome and not required here.

    this->find(service, instance, event);
    ASSERT_EQ(this->searchResult.size(), 1);
}

TYPED_TEST(ServiceRegistry_test, FunctionIsAppliedToAllEntriesInSearchResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7828085-d879-43b7-9fee-e5e88cf36995");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("a", "b", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());

    this->find(iox::capro::IdString_t("a"), iox::capro::IdString_t("b"), iox::capro::Wildcard);

    ASSERT_THAT(this->searchResult.size(), Eq(2));
    EXPECT_THAT(this->searchResult[0].serviceDescription, Eq(service1));
    EXPECT_THAT(this->searchResult[1].serviceDescription, Eq(service3));
}

TYPED_TEST(ServiceRegistry_test, NoFunctionIsAppliedToEmptySearchResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "220213cb-8fdf-4fd0-b8e2-24a96f11bfbc");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("a", "b", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.add(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());

    this->find(iox::capro::Wildcard, iox::capro::IdString_t("a"), iox::capro::Wildcard);

    EXPECT_THAT(this->searchResult.size(), Eq(0));
}

TYPED_TEST(ServiceRegistry_test, FindWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ac27528-8650-4d8a-8440-4c9bbcbee4fb");
    iox::capro::ServiceDescription service("ninjababy", "pow", "pow");
    ASSERT_FALSE(this->sut.add(service).has_error());
    this->find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);
}

TYPED_TEST(ServiceRegistry_test, FindWithMixOfPublishersAndServersWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e8a9647-69cc-4cad-afb1-9188927aff04");
    iox::capro::ServiceDescription service1("a", "a", "a");
    iox::capro::ServiceDescription service2("a", "b", "b");
    iox::capro::ServiceDescription service3("a", "a", "c");
    iox::capro::ServiceDescription service4("a", "a", "d");
    iox::capro::ServiceDescription service5("b", "b", "d");

    ASSERT_FALSE(this->sut.add(service1).has_error());
    ASSERT_FALSE(this->sut.otherAdd(service2).has_error());
    ASSERT_FALSE(this->sut.add(service3).has_error());
    ASSERT_FALSE(this->sut.otherAdd(service4).has_error());
    ASSERT_FALSE(this->sut.add(service5).has_error());

    this->find(iox::capro::Wildcard, iox::capro::IdString_t("a"), iox::capro::Wildcard);

    EXPECT_THAT(this->searchResult.size(), Eq(3U));

    auto filtered = this->sut.filter(this->searchResult);

    // only service1 and service3 match the category (server or publisher),
    // the other match (service 4) is of a different category
    ASSERT_EQ(filtered.size(), 2U);
    EXPECT_EQ(filtered[0].serviceDescription, service1);
    EXPECT_EQ(filtered[1].serviceDescription, service3);
}

TYPED_TEST(ServiceRegistry_test, HasDataChangedSinceLastCallReturnsTrueOnInitialCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "51398abb-53b2-4dce-9267-73f02f9d7574");

    EXPECT_TRUE(this->sut.registry.hasDataChangedSinceLastCall());
}

TYPED_TEST(ServiceRegistry_test, HasDataChangedSinceLastCallReturnsFalseOnSubsequentCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "a8a8d286-01ba-4084-94c4-fd0866e0e5d0");

    EXPECT_TRUE(this->sut.registry.hasDataChangedSinceLastCall());
    EXPECT_FALSE(this->sut.registry.hasDataChangedSinceLastCall());
}

TYPED_TEST(ServiceRegistry_test, HasDataChangedSinceLastCallReturnsTrueAfterAddingService)
{
    ::testing::Test::RecordProperty("TEST_ID", "17d5b84a-abe0-46e0-aa06-2d049c716b22");

    iox::capro::ServiceDescription service("a", "a", "a");

    this->sut.registry.hasDataChangedSinceLastCall();

    ASSERT_FALSE(this->sut.add(service).has_error());

    EXPECT_TRUE(this->sut.registry.hasDataChangedSinceLastCall());
}

TYPED_TEST(ServiceRegistry_test, HasDataChangedSinceLastCallReturnsTrueAfterRemovingService)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4f0c9e2-2549-4fa0-88d4-75a2ef8714b8");

    iox::capro::ServiceDescription service("a", "a", "a");

    ASSERT_FALSE(this->sut.add(service).has_error());
    this->sut.registry.hasDataChangedSinceLastCall();

    this->sut.remove(service);

    EXPECT_TRUE(this->sut.registry.hasDataChangedSinceLastCall());
}

} // namespace
