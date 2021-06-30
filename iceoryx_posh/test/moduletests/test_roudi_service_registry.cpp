// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::roudi;
/// @todo #415 Replace Wildcards once service registry has its new data structure
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
    iox::roudi::ServiceRegistry registry;

    iox::roudi::ServiceRegistry::ServiceDescriptionVector_t searchResults;
};

/// @todo Add EXPECT_THAT(add(), Eq(ExpectedError::FOO))

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFound)
{
    iox::capro::ServiceDescription service1("a", "b", "c");
    ASSERT_FALSE(registry.add(service1).has_error());
    registry.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0], Eq(service1));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameCanAllBeFound)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());
    registry.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(3));

    bool hasFoundB = false;
    bool hasFoundC = false;
    bool hasFoundD = false;

    for (auto& e : searchResults)
    {
        if (e == service1)
            hasFoundB = true;
        if (e == service2)
            hasFoundC = true;
        if (e == service3)
            hasFoundD = true;
    }

    EXPECT_THAT(hasFoundB && hasFoundC && hasFoundD, Eq(true));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithDifferentServiceNameCanAllBeFound)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("c", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    registry.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0], Eq(service1));
    searchResults.clear();

    registry.find(searchResults, "c", Wildcard);
    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0], Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameFindsSpecificInstance)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());
    registry.find(searchResults, "a", "c");

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0], Eq(service2));
}

TEST_F(ServiceRegistry_test, FindSpecificNonExistingServiceDescription)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());
    registry.find(searchResults, "a", "g");

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionWithSameServicesAndRemovingSpecificDoesNotFindSpecific)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());

    EXPECT_TRUE(registry.remove(service2));

    registry.find(searchResults, "a", "c");
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test,
       AddingMultipleServiceDescriptionWithDifferentServicesAndRemovingSpecificDoesNotFindSpecific)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("c", "d", "d");

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());

    EXPECT_TRUE(registry.remove(service2));

    registry.find(searchResults, "b", "c");
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionAndRemovingAllDoesNotFindAnything)
{
    iox::capro::ServiceDescription service1("a", "b", "b"); // 0
    iox::capro::ServiceDescription service2("a", "c", "c"); // 1
    iox::capro::ServiceDescription service3("a", "d", "d"); // 2

    ASSERT_FALSE(registry.add(service1).has_error());
    ASSERT_FALSE(registry.add(service2).has_error());
    ASSERT_FALSE(registry.add(service3).has_error());

    EXPECT_TRUE(registry.remove(service1));
    EXPECT_TRUE(registry.remove(service2));
    EXPECT_TRUE(registry.remove(service3));

    registry.find(searchResults, "a", Wildcard);
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingVariousServiceDescriptionAndGetAllServicesDoesNotReturnDuplicate)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");
    iox::capro::ServiceDescription service4("e", "f", "f");

    ASSERT_FALSE(registry.add(service1).has_error());
    // add same service a, instance c to check if in registry only one entry is created
    ASSERT_FALSE(registry.add(service2).has_error());
    auto result = registry.add(service2);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(),
                Eq(iox::roudi::ServiceRegistry::ServiceRegistryError::SERVICE_DESCRIPTION_ALREADY_ADDED));
    ASSERT_FALSE(registry.add(service3).has_error());
    ASSERT_FALSE(registry.add(service4).has_error());

    auto serviceDescriptionVector = registry.getAllServices();

    bool service1Found = false;
    bool service2Found = false;
    bool service4Found = false;

    for (auto const& element : serviceDescriptionVector)
    {
        if (element == service1)
        {
            service1Found = true;
        }

        if (element == service2)
        {
            service2Found = true;
        }

        if (element == service4)
        {
            service4Found = true;
        }
    }
    EXPECT_THAT(serviceDescriptionVector.size(), Eq(4));
    EXPECT_THAT(service1Found && service2Found && service4Found, Eq(true));
}

/// @todo implement missing tests

} // namespace
