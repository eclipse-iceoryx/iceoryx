// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

using namespace iox;
using namespace iox::runtime;
using namespace iox::roudi_env;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/service_discovery.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;
using description_vector = vector<iox_service_description_t, MAX_FINDSERVICE_RESULT_SIZE>;

class iox_service_discovery_test : public RouDi_GTest
{
  public:
    iox_service_discovery_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }
    void SetUp() override
    {
        iox_runtime_init("runtime");
        sut = iox_service_discovery_init(&sutStorage);
    }

    void TearDown() override
    {
        searchResult.clear();
        iox_service_discovery_deinit(sut);
    }

    iox_service_discovery_storage_t sutStorage;
    iox_service_discovery_t sut;

    static description_vector searchResult;
    static void findHandler(const iox_service_description_t s, void* contextData)
    {
        static_cast<description_vector*>(contextData)->emplace_back(s);
    }
};

description_vector iox_service_discovery_test::searchResult;

TEST(iox_service_discovery_DeathTest, InitServiceDiscoveryWithNullptrForStorageTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "be551a9e-7dcf-406a-a74c-7dcb1ee16c30");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_service_discovery_init(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

/// @note We test only if the arguments of iox_service_discovery_find_service are correctly passed to
/// ServiceDiscovery::findService.
TEST_F(iox_service_discovery_test,
       FindServiceWithCallableAndContextDataWithNullptrsForServiceInstanceEventReturnsAllServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "09a2cd6c-fba9-4b9d-af96-c5a6cc168d98");

    // let the roudi discovery loop run at least once
    triggerDiscoveryLoopAndWaitToFinish();

    iox_service_discovery_find_service_apply_callable_with_context_data(
        sut, nullptr, nullptr, nullptr, findHandler, &searchResult, MessagingPattern_PUB_SUB);
    for (const auto& service : searchResult)
    {
        EXPECT_THAT(service.instanceString, StrEq("RouDi_ID"));
    }
}

TEST_F(iox_service_discovery_test, FindServiceWithCallableAndContextDataReturnsOfferedService)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb12e514-e7af-4946-b098-98b3cd0f43a5");
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    iox_pub_storage_t storage;
    auto* publisher = iox_pub_init(&storage, "service", "instance", "event", &options);
    ASSERT_NE(publisher, nullptr);
    const iox_service_description_t SERVICE_DESCRIPTION = iox_pub_get_service_description(publisher);

    triggerDiscoveryLoopAndWaitToFinish();

    iox_service_discovery_find_service_apply_callable_with_context_data(sut,
                                                                        SERVICE_DESCRIPTION.serviceString,
                                                                        SERVICE_DESCRIPTION.instanceString,
                                                                        SERVICE_DESCRIPTION.eventString,
                                                                        findHandler,
                                                                        &searchResult,
                                                                        MessagingPattern_PUB_SUB);
    ASSERT_THAT(searchResult.size(), Eq(1U));
    EXPECT_THAT(*searchResult.begin()->serviceString, Eq(*SERVICE_DESCRIPTION.serviceString));
    EXPECT_THAT(*searchResult.begin()->instanceString, Eq(*SERVICE_DESCRIPTION.instanceString));
    EXPECT_THAT(*searchResult.begin()->eventString, Eq(*SERVICE_DESCRIPTION.eventString));

    iox_pub_deinit(publisher);
}

TEST_F(iox_service_discovery_test, FindServiceWithCallableWithNullptrsForServiceInstanceEventFindsCorrectServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec565ca3-7494-42d7-9440-2000f1513759");

    triggerDiscoveryLoopAndWaitToFinish();

    auto findHandler = [](const iox_service_description_t s) { EXPECT_THAT(s.instanceString, StrEq("RouDi_ID")); };
    iox_service_discovery_find_service_apply_callable(
        sut, nullptr, nullptr, nullptr, findHandler, MessagingPattern_PUB_SUB);
}

TEST_F(iox_service_discovery_test, FindServiceWithCallableReturnsFindsCorrectService)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ac1f029-3c05-4f95-90e9-e848ceb2d4d7");
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    iox_pub_storage_t storage;
    auto* publisher = iox_pub_init(&storage, "service", "instance", "event", &options);
    ASSERT_NE(publisher, nullptr);

    triggerDiscoveryLoopAndWaitToFinish();

    auto findHandler = [](const iox_service_description_t s) {
        EXPECT_THAT(s.serviceString, StrEq("service"));
        EXPECT_THAT(s.instanceString, StrEq("instance"));
        EXPECT_THAT(s.eventString, StrEq("event"));
    };
    iox_service_discovery_find_service_apply_callable(
        sut, "service", "instance", "event", findHandler, MessagingPattern_PUB_SUB);

    iox_pub_deinit(publisher);
}

TEST_F(iox_service_discovery_test, FindServiceWithNullptrsForServiceInstanceEventReturnsAllServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "75b411d7-b8c7-42d5-8acd-3916fd172081");

    triggerDiscoveryLoopAndWaitToFinish();

    const uint64_t SERVICE_CONTAINER_CAPACITY = 10U;
    iox_service_description_t serviceContainer[SERVICE_CONTAINER_CAPACITY];
    uint64_t missedServices = 0U;
    const auto numberFoundServices = iox_service_discovery_find_service(sut,
                                                                        nullptr,
                                                                        nullptr,
                                                                        nullptr,
                                                                        serviceContainer,
                                                                        SERVICE_CONTAINER_CAPACITY,
                                                                        &missedServices,
                                                                        MessagingPattern_PUB_SUB);

    EXPECT_THAT(numberFoundServices, Eq(6U));
    EXPECT_THAT(missedServices, Eq(0U));
    for (uint64_t i = 0U; i < numberFoundServices; ++i)
    {
        EXPECT_THAT(serviceContainer[i].instanceString, StrEq("RouDi_ID"));
    }
}

TEST_F(iox_service_discovery_test, FindServiceReturnsOfferedService)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bbd0b26-ed9d-4fcd-ae85-e7ea3783996d");
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    iox_pub_storage_t storage;
    auto* publisher = iox_pub_init(&storage, "service", "instance", "event", &options);
    ASSERT_NE(publisher, nullptr);
    const iox_service_description_t SERVICE_DESCRIPTION = iox_pub_get_service_description(publisher);

    triggerDiscoveryLoopAndWaitToFinish();

    const uint64_t SERVICE_CONTAINER_CAPACITY = 10U;
    iox_service_description_t serviceContainer[SERVICE_CONTAINER_CAPACITY];
    uint64_t missedServices = 0U;
    const auto numberFoundServices = iox_service_discovery_find_service(sut,
                                                                        SERVICE_DESCRIPTION.serviceString,
                                                                        SERVICE_DESCRIPTION.instanceString,
                                                                        SERVICE_DESCRIPTION.eventString,
                                                                        serviceContainer,
                                                                        SERVICE_CONTAINER_CAPACITY,
                                                                        &missedServices,
                                                                        MessagingPattern_PUB_SUB);

    EXPECT_THAT(numberFoundServices, Eq(1U));
    EXPECT_THAT(missedServices, Eq(0U));
    EXPECT_THAT(serviceContainer[0U].serviceString, StrEq(SERVICE_DESCRIPTION.serviceString));
    EXPECT_THAT(serviceContainer[0U].instanceString, StrEq(SERVICE_DESCRIPTION.instanceString));
    EXPECT_THAT(serviceContainer[0U].eventString, StrEq(SERVICE_DESCRIPTION.eventString));

    iox_pub_deinit(publisher);
}

TEST_F(iox_service_discovery_test, FindServiceReturnsCorrectNumberOfServicesWhenServiceContainerTooSmall)
{
    ::testing::Test::RecordProperty("TEST_ID", "01047b88-f257-447c-8c5e-9bef7c358433");

    triggerDiscoveryLoopAndWaitToFinish();

    const uint64_t SERVICE_CONTAINER_CAPACITY = 3U;
    iox_service_description_t serviceContainer[SERVICE_CONTAINER_CAPACITY];
    uint64_t missedServices = 0U;
    const auto numberFoundServices = iox_service_discovery_find_service(sut,
                                                                        nullptr,
                                                                        nullptr,
                                                                        nullptr,
                                                                        serviceContainer,
                                                                        SERVICE_CONTAINER_CAPACITY,
                                                                        &missedServices,
                                                                        MessagingPattern_PUB_SUB);

    EXPECT_THAT(numberFoundServices, Eq(SERVICE_CONTAINER_CAPACITY));
    EXPECT_THAT(missedServices, Eq(NUMBER_OF_INTERNAL_PUBLISHERS - SERVICE_CONTAINER_CAPACITY));
}

TEST_F(iox_service_discovery_test, DeinitServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf09c7b3-fb20-44e3-8552-9877d0facacd");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_service_discovery_deinit(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_service_discovery_test, FindServiceServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c908fbf-3855-47b6-8837-62c97a3f6f40");
    const uint64_t SERVICE_CONTAINER_CAPACITY = 10U;
    iox_service_description_t serviceContainer[SERVICE_CONTAINER_CAPACITY];
    uint64_t missedServices = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service(nullptr,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               serviceContainer,
                                               SERVICE_CONTAINER_CAPACITY,
                                               &missedServices,
                                               MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service(sut,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               SERVICE_CONTAINER_CAPACITY,
                                               &missedServices,
                                               MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service(sut,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               serviceContainer,
                                               SERVICE_CONTAINER_CAPACITY,
                                               nullptr,
                                               MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_service_discovery_test, FindServiceApplyCallablekServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b44b1ffc-f466-49f7-a7d5-2bf25bfb22a5");
    auto findHandler = [](const iox_service_description_t s) { EXPECT_THAT(s.instanceString, StrEq("RouDi_ID")); };
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service_apply_callable(
                nullptr, nullptr, nullptr, nullptr, findHandler, MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service_apply_callable(
                sut, nullptr, nullptr, nullptr, nullptr, MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_service_discovery_test, FindServiceApplyCallableWithContextDataServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "72242b75-0c81-4a6b-b4cd-d6990e6e4b7b");
    auto findHandler = [](const iox_service_description_t s, void*) {
        EXPECT_THAT(s.instanceString, StrEq("RouDi_ID"));
    };
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service_apply_callable_with_context_data(
                nullptr, nullptr, nullptr, nullptr, findHandler, &searchResult, MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_service_discovery_find_service_apply_callable_with_context_data(
                sut, nullptr, nullptr, nullptr, nullptr, &searchResult, MessagingPattern_PUB_SUB);
        },
        iox::er::ENFORCE_VIOLATION);
}

} // namespace
