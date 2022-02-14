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

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"

#include "test.hpp"

#include <chrono>
#include <random>

namespace
{
using namespace ::testing;
using namespace iox::roudi;

class ServiceRegistry_test : public Test
{
  public:
    using ServiceDescription = iox::capro::ServiceDescription;
    virtual void SetUp()
    {
        internal::CaptureStdout();
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
    iox::roudi::ServiceRegistry sut;

    iox::roudi::ServiceRegistry::ServiceDescriptionVector_t searchResults;
};

TEST_F(ServiceRegistry_test, AddNoServiceDescriptionsAndWildcardSearchReturnsNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a050209-01d8-4d0e-9e70-c0662b9dbe76");
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddMaximumNumberOfServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa7d6416-4183-4942-a323-01f78c1bb6c1");
    iox::cxx::vector<ServiceDescription, ServiceRegistry::MAX_SERVICE_DESCRIPTIONS> services;

    for (uint64_t i = 0U; i < ServiceRegistry::MAX_SERVICE_DESCRIPTIONS; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::capro::IdString_t(iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = sut.add(service);
        ASSERT_FALSE(result.has_error());
    }
}

TEST_F(ServiceRegistry_test, AddMoreThanMaximumNumberOfServiceDescriptionsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a911f654-8314-4ea3-b9b2-1afa121a2b21");
    iox::cxx::vector<ServiceDescription, ServiceRegistry::MAX_SERVICE_DESCRIPTIONS> services;

    for (uint64_t i = 0U; i < ServiceRegistry::MAX_SERVICE_DESCRIPTIONS; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::capro::IdString_t(iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = sut.add(service);
        ASSERT_FALSE(result.has_error());
    }

    auto result = sut.add(iox::capro::ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ServiceRegistry::Error::SERVICE_REGISTRY_FULL));
}

TEST_F(ServiceRegistry_test, AddServiceDescriptionsWhichWasAlreadyAddedAndReturnsOneResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8f61eb2-d082-4c26-9970-461427c3d200");
    auto result1 = sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result2.has_error());

    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaer")));
    EXPECT_THAT(searchResults[0].count, Eq(2));
}

TEST_F(ServiceRegistry_test, AddServiceDescriptionsTwiceAndRemoveOnceAndReturnsOneResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f8193ea-2d36-423f-a658-1dba30c1868d");
    auto result1 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result2.has_error());

    sut.remove(ServiceDescription("Li", "La", "Launebaerli"));

    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaerli")));
    EXPECT_THAT(searchResults[0].count, Eq(1));
}

TEST_F(ServiceRegistry_test, AddServiceDescriptionsTwiceAndPurgeReturnsNoResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "3185b67f-b891-4a82-8f91-047e059ed68f");
    auto result1 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result2.has_error());

    sut.purge(ServiceDescription("Li", "La", "Launebaerli"));

    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddEmptyServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cff55b0-d12f-48f5-8f0c-6501d0c2bf79");
    auto result = sut.add(ServiceDescription());
    ASSERT_FALSE(result.has_error());
}

TEST_F(ServiceRegistry_test, RemovingServiceDescriptionsWhichWasntAddedFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf3e39f5-c29d-4b5f-8e01-af69681b2ea8");
    sut.remove(ServiceDescription("Sim", "Sa", "Lambim"));
    EXPECT_THAT(sut.getServices().size(), Eq(0));
}

TEST_F(ServiceRegistry_test, RemovingEmptyServiceDescriptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "523f2320-c0e5-4590-a1b0-604e756ecaa5");
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.remove(ServiceDescription());
    EXPECT_THAT(sut.getServices().size(), Eq(0));
}

TEST_F(ServiceRegistry_test, SingleEmptyServiceDescriptionsCanBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "be3e4b13-d930-47b2-aeaa-95c65f06deed");
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription()));
}

TEST_F(ServiceRegistry_test, SingleEmptyServiceDescriptionsCanBeFoundWithEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "1af0137f-6bf5-422e-a6ec-513f7d3f6191");
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.find(searchResults, iox::capro::IdString_t(""), iox::capro::IdString_t(""), iox::capro::IdString_t(""));

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription()));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "a12c9d39-6379-4296-8987-df56a87169f7");
    auto result = sut.add(ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_FALSE(result.has_error());
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Foo", "Bar", "Baz")));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithEventName)
{
    ::testing::Test::RecordProperty("TEST_ID", "6df8fd7d-e4d1-4c51-8ad7-2fbe82e4ed09");
    iox::capro::ServiceDescription service("a", "b", "c");
    ASSERT_FALSE(sut.add(service).has_error());
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::IdString_t("c"));

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service));
}

TEST_F(ServiceRegistry_test, ServiceDescriptionNotFoundWhenEventDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba7785d1-08ec-4f7c-b341-dff033dae2c7");
    iox::capro::ServiceDescription service("Besser", "Wisser", "Girl");
    ASSERT_FALSE(sut.add(service).has_error());
    sut.find(searchResults,
             iox::capro::IdString_t("Besser"),
             iox::capro::IdString_t("Wisser"),
             iox::capro::IdString_t("Boy"));

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithInstanceName)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dfaa836-f1da-466f-a794-a7ac1b599ce3");
    auto result = sut.add(ServiceDescription("Baz", "Bar", "Foo"));
    ASSERT_FALSE(result.has_error());
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::IdString_t("Bar"), iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Baz", "Bar", "Foo")));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithServiceName)
{
    ::testing::Test::RecordProperty("TEST_ID", "0890013c-e14b-4ae2-89cb-757624c12b4e");
    iox::capro::ServiceDescription service("a", "b", "c");
    ASSERT_FALSE(sut.add(service).has_error());
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service));
}

TEST_F(ServiceRegistry_test, EmptyAndNotEmptyServiceDescriptionsCanAllBeFoundWithWildcardSearch)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f34e604-5217-4e47-9c6f-26c5cbdcd3ec");
    ServiceDescription service1;
    ServiceDescription service2("alpha", "bravo", "charlie");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    sut.find(searchResults, iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(2));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
    EXPECT_THAT(searchResults[1].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameCanAllBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "251fe262-2e8f-4e32-a5f1-a4b1aaa812fd");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(3));

    bool hasFoundB = false;
    bool hasFoundC = false;
    bool hasFoundD = false;

    for (auto& e : searchResults)
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

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithDifferentServiceNameCanAllBeFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b7a0897-46a7-457e-8746-0c3899e96653");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("c", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
    searchResults.clear();

    sut.find(searchResults, iox::capro::IdString_t("c"), iox::capro::Wildcard, iox::capro::Wildcard);
    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameFindsSpecificService)
{
    ::testing::Test::RecordProperty("TEST_ID", "557d6533-25a3-4c0c-adc0-e8ebb74785a0");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));

    ASSERT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionAddedInNonLinearOrderFindsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "26187db1-299a-494c-bd85-eb646b8cf67b");
    iox::capro::ServiceDescription service1("a", "1", "moep");
    iox::capro::ServiceDescription service2("b", "2", "moep");
    iox::capro::ServiceDescription service3("c", "3", "moep");
    iox::capro::ServiceDescription service4("d", "4", "moep");
    iox::capro::ServiceDescription service5("e", "5", "moep");

    ASSERT_FALSE(sut.add(service5).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    ASSERT_FALSE(sut.add(service4).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service1).has_error());

    sut.remove(service5);
    sut.remove(service1);
    EXPECT_THAT(sut.getServices().size(), Eq(3));
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, FindSpecificNonExistingServiceDescriptionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "183600f6-3d2d-4c0f-8c1d-d74acfb7fc50");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::IdString_t("g"), iox::capro::IdString_t("f"));

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionWithSameServicesAndRemovingSpecificDoesNotFindSpecific)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8230904-c14e-4e9f-a324-f92c67522271");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service2);
    EXPECT_THAT(sut.getServices().size(), Eq(2));

    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, ServiceNotFoundAfterAddingAndRemovingToServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5e67b1f-65f2-4973-9f76-73479dd1de9e");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("c", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service2);
    EXPECT_THAT(sut.getServices().size(), Eq(2));

    sut.find(searchResults, iox::capro::IdString_t("b"), iox::capro::IdString_t("c"), iox::capro::IdString_t("c"));
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionAndRemovingAllDoesNotFindAnything)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7a7e160-813c-4daf-8c55-d0a85bb5f642");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service1);
    sut.remove(service2);
    sut.remove(service3);

    sut.find(searchResults, iox::capro::IdString_t("a"), iox::capro::Wildcard, iox::capro::Wildcard);
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingVariousServiceDescriptionAndGetServicesDoesNotReturnDuplicate)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0348a73-4dbe-4464-b190-9274c2bd5031");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");
    iox::capro::ServiceDescription service4("e", "f", "f");

    ASSERT_FALSE(sut.add(service1).has_error());
    // add same service a, instance c to check if in sut only one entry is created
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    ASSERT_FALSE(sut.add(service4).has_error());

    auto serviceDescriptionVector = sut.getServices();

    bool service1Found = false;
    bool service2Found = false;
    bool service4Found = false;

    for (auto const& element : serviceDescriptionVector)
    {
        if (element.serviceDescription == service1)
        {
            service1Found = true;
        }

        if (element.serviceDescription == service2)
        {
            service2Found = true;
        }

        if (element.serviceDescription == service4)
        {
            service4Found = true;
        }
    }
    EXPECT_THAT(serviceDescriptionVector.size(), Eq(4));
    EXPECT_THAT(service1Found && service2Found && service4Found, Eq(true));
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
    // deliberately contains no `0` (need to exclude some char)
    static const char chars[] = "123456789"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz";

    constexpr auto N = string_t::capacity();
    size = std::min(N, size);

    char a[N + 1];
    for (uint64_t i = 0U; i < size; ++i)
    {
        a[i] = chars[uniform(sizeof(chars) - 1)];
    }
    a[size] = '\0';

    string_t s(a);
    return s;
}

TEST_F(ServiceRegistry_test, CanAddMaximumNumberOfDifferentServiceDescriptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "76aef6cb-7886-4d64-9188-09bd1be2d335");
    uint32_t numEntriesAdded = 0U;
    do
    {
        // may (rarely) generate duplicates to be counted internally
        auto id = randomString();
        ServiceDescription sd(id, id, id);
        auto result = sut.add(sd);
        if (result.has_error())
        {
            break;
        }
        numEntriesAdded++;
    } while (true);

    // duplicates do not count to the max and may be generated randomly,
    // but for the contract we only need to guarantee that we can at least add
    // the configured max
    constexpr auto MAX = ServiceRegistry::MAX_SERVICE_DESCRIPTIONS;
    EXPECT_GE(numEntriesAdded, MAX);
}

TEST_F(ServiceRegistry_test, SearchInFullRegistryWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c4519d4-1873-4837-a3eb-0367106fb9b5");

    constexpr auto CAP = string_t::capacity();

    string_t fixedId(iox::cxx::TruncateToCapacity, std::string(CAP, '0'));

    ServiceDescription lastAdded;
    do
    {
        auto id = randomString();
        ServiceDescription sd(fixedId, fixedId, id);

        auto result = sut.add(sd);
        if (result.has_error())
        {
            break;
        }
        lastAdded = sd;
    } while (true);

    // remove the last and replace it with a unique service description
    sut.purge(lastAdded);

    // is unique (random does not generate 0s) and last if a vector is used internally
    // for almost worst case search (search on last string will terminate early whp)
    auto id = randomString(CAP - 1);
    id.unsafe_append("0");

    ServiceDescription uniqueSd(fixedId, fixedId, id);
    auto result = sut.add(uniqueSd);
    EXPECT_FALSE(result.has_error());

    ServiceRegistry::ServiceDescriptionVector_t searchResult;
    auto& service = uniqueSd.getServiceIDString();
    auto& instance = uniqueSd.getInstanceIDString();
    auto& event = uniqueSd.getEventIDString();

    // This is close to a worst case search but not quite due to randomness in the last string.
    // Different strings are required as we need different strings to create a full registry,
    // and randomness is the easiest way to achieve this.
    // It could also be achieved with determinstic string enumeration instead of randomness,
    // but it is more cumbersome and not required here.

    sut.find(searchResult, service, instance, event);
    ASSERT_EQ(searchResult.size(), 1);
}

using Entry = iox::roudi::ServiceRegistry::ServiceDescriptionEntry;

TEST_F(ServiceRegistry_test, FunctionIsAppliedToAllEntriesInSearchResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7828085-d879-43b7-9fee-e5e88cf36995");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("a", "b", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    auto searchFunction = [&](const Entry& entry) { this->searchResults.emplace_back(entry); };
    sut.find(iox::capro::IdString_t("a"), iox::capro::IdString_t("b"), iox::capro::Wildcard, searchFunction);

    ASSERT_THAT(searchResults.size(), Eq(2));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
    EXPECT_THAT(searchResults[1].serviceDescription, Eq(service3));
}

TEST_F(ServiceRegistry_test, NoFunctionIsAppliedToEmptySearchResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "220213cb-8fdf-4fd0-b8e2-24a96f11bfbc");
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("a", "b", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    auto searchFunction = [&](const Entry& entry) { searchResults.emplace_back(entry); };
    sut.find(iox::capro::Wildcard, iox::capro::IdString_t("a"), iox::capro::Wildcard, searchFunction);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, FindWithEmptyCallableDoesNotDie)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ac27528-8650-4d8a-8440-4c9bbcbee4fb");
    iox::capro::ServiceDescription service("ninjababy", "pow", "pow");
    ASSERT_FALSE(sut.add(service).has_error());
    iox::cxx::function_ref<void(const Entry&)> searchFunction;
    sut.find(iox::capro::Wildcard, iox::capro::Wildcard, iox::capro::Wildcard, searchFunction);
}

} // namespace
