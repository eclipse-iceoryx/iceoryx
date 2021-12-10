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
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/posh_discovery.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"
#include "mocks/posh_runtime_mock.hpp"
#include "test.hpp"

#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::cxx;
using iox::roudi::RouDiEnvironment;

class PoshDiscovery_test : public Test
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

    const iox::RuntimeName_t m_runtimeName{"publisher"};
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};
    PoshDiscovery m_poshDiscovery;
};

TIMING_TEST_F(PoshDiscovery_test, GetServiceRegistryChangeCounterOfferStopOfferService, Repeat(5), [&] {
    auto serviceCounter = m_poshDiscovery.getServiceRegistryChangeCounter();
    auto initialCout = serviceCounter->load();

    m_poshDiscovery.offerService({"service1", "instance1", "event1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 1 == serviceCounter->load());

    m_poshDiscovery.stopOfferService({"service1", "instance1", "event1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 2 == serviceCounter->load());
});

TEST_F(PoshDiscovery_test, OfferEmptyServiceIsInvalid)
{
    auto isServiceOffered = m_poshDiscovery.offerService(iox::capro::ServiceDescription());

    EXPECT_FALSE(isServiceOffered);
}

TEST_F(PoshDiscovery_test, FindServiceWithWildcardsReturnsOnlyIntrospectionServices)
{
    iox::runtime::PoshRuntime::initRuntime("subscriber");

    PoshDiscovery m_receiverDiscovery;

    EXPECT_FALSE(m_poshDiscovery.offerService(iox::capro::ServiceDescription()));
    this->InterOpWait();

    auto serviceContainer = m_receiverDiscovery.findService(iox::runtime::Wildcard_t(), iox::runtime::Wildcard_t());
    ASSERT_FALSE(serviceContainer.has_error());

    auto searchResult = serviceContainer.value();

    for (auto& service : searchResult)
    {
        EXPECT_THAT(service.getServiceIDString().c_str(), StrEq("Introspection"));
    }
}

} // namespace
