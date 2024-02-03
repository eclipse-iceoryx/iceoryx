// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/capro/service_description.hpp"
#include "iox/detail/convert.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"
#include "iox/detail/serialization.hpp"
#include "iox/string.hpp"
/// @todo iox-#415 replace the service registry include with the new discovery API header
#include "iceoryx_posh/internal/roudi/service_registry.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"

#include <cstdint>

namespace
{
using namespace ::testing;
using namespace iox::testing;

using namespace iox::capro;

class ServiceDescription_test : public Test
{
  public:
    IdString_t testService;
    IdString_t testInstance;
    IdString_t testEvent;
    void SetUp(){};
    void TearDown(){};
};

/// BEGIN CLASSHASH TESTS

TEST_F(ServiceDescription_test, ServiceDescriptionClassHashDefaultCtorCreatesClassHashWithDefaultValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b57b18f-cd68-49ee-8fbb-7b1fcc878a16");
    ServiceDescription::ClassHash testHash{};

    EXPECT_EQ(uint32_t(0), testHash[0]);
    EXPECT_EQ(uint32_t(0), testHash[1]);
    EXPECT_EQ(uint32_t(0), testHash[2]);
    EXPECT_EQ(uint32_t(0), testHash[3]);
}

TEST_F(ServiceDescription_test, ServiceDescriptionClassHashCtorCreatesClassHashWithValuesPassedToTheCtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "00d1f6f1-4011-406e-a18e-85af7fa401f4");
    ServiceDescription::ClassHash testHash{1U, 2U, 3U, 4U};

    EXPECT_EQ(uint32_t(1), testHash[0]);
    EXPECT_EQ(uint32_t(2), testHash[1]);
    EXPECT_EQ(uint32_t(3), testHash[2]);
    EXPECT_EQ(uint32_t(4), testHash[3]);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithEqualityOperatorReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc2e03b0-d9ac-49fd-8d1d-6e2393ce3d68");
    ServiceDescription::ClassHash testHash1{15U, 25U, 35U, 45U};
    ServiceDescription::ClassHash testHash2{55U, 65U, 75U, 85U};

    EXPECT_FALSE(testHash1 == testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithEqualityOperatorReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3423678c-d45e-4b36-bce6-7e6d0a5bc2a6");
    ServiceDescription::ClassHash testHash1{10U, 20U, 30U, 40U};
    ServiceDescription::ClassHash testHash2{10U, 20U, 30U, 40U};

    EXPECT_TRUE(testHash1 == testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithNotEqualOperatorReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "59d7790f-5d1f-4f1f-9cf6-8f474ae8978f");
    ServiceDescription::ClassHash testHash1{12U, 24U, 36U, 48U};
    ServiceDescription::ClassHash testHash2{60U, 72U, 84U, 96U};

    EXPECT_TRUE(testHash1 != testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoEqualClassHashWithNotEqualOperatorReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "498fa728-7fbb-4e99-8e95-eaf267284f22");
    ServiceDescription::ClassHash testHash1{11U, 22U, 33U, 44U};
    ServiceDescription::ClassHash testHash2{11U, 22U, 33U, 44U};

    EXPECT_FALSE(testHash1 != testHash2);
}

TEST_F(ServiceDescription_test, ClassHashWithValuesAssignedUsingAssignmentOperatorStoresTheValueInTheCorrespondingIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d37a48e-ba08-4fc8-9215-77bac17bd49b");
    ServiceDescription::ClassHash testHash{};

    testHash[0] = 10U;
    testHash[1] = 20U;
    testHash[2] = 30U;
    testHash[3] = 40U;

    EXPECT_THAT(testHash[0], Eq(10U));
    EXPECT_THAT(testHash[1], Eq(20U));
    EXPECT_THAT(testHash[2], Eq(30U));
    EXPECT_THAT(testHash[3], Eq(40U));
}

TEST_F(ServiceDescription_test, ClassHashSubsriptOperatorOutOfBoundsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ac4b4cb3-503c-4e39-a549-684176e7557a");
    ServiceDescription::ClassHash testHash{1U, 2U, 3U, 4U};

    testHash[0] = 1U;
    testHash[1] = 2U;
    testHash[2] = 3U;
    testHash[3] = 4U;


    IOX_EXPECT_FATAL_FAILURE([&] { testHash[4] = 5U; }, iox::er::FATAL);
}

/// END CLASSHASH TESTS

/// BEGIN SERVICEDESCRIPTION TESTS

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test, ServiceDescriptionSerializationCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "0bda1264-f1b0-41d5-b1c4-f8e7f2a5806a");
    ServiceDescription::ClassHash testHash = {11U, 21U, 31U, 41U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    Scope testScope = Scope::LOCAL;
    Interfaces testInterfaceSource = Interfaces::INTERNAL;
    auto serialObj = iox::Serialization::create(testService.c_str(),
                                                testInstance.c_str(),
                                                testEvent.c_str(),
                                                testHash[0],
                                                testHash[1],
                                                testHash[2],
                                                testHash[3],
                                                static_cast<uint16_t>(testScope),
                                                static_cast<uint16_t>(testInterfaceSource));


    ServiceDescription::deserialize(serialObj)
        .and_then([&](const auto& service) {
            EXPECT_THAT(service.getServiceIDString(), Eq(testService));
            EXPECT_THAT(service.getInstanceIDString(), Eq(testInstance));
            EXPECT_THAT(service.getEventIDString(), Eq(testEvent));
            EXPECT_THAT((service.getClassHash())[0], Eq(testHash[0]));
            EXPECT_THAT((service.getClassHash())[1], Eq(testHash[1]));
            EXPECT_THAT((service.getClassHash())[2], Eq(testHash[2]));
            EXPECT_THAT((service.getClassHash())[3], Eq(testHash[3]));
            EXPECT_THAT(service.getScope(), Eq(Scope::LOCAL));
            EXPECT_THAT(service.getSourceInterface(), Eq(Interfaces::INTERNAL));
        })
        .or_else([](const auto& error) {
            GTEST_FAIL() << "Deserialization should not fail but failed with: " << static_cast<uint32_t>(error);
        });
}

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test,
       ServiceDescriptionObjectInitialisationWithOutOfBoundaryScopeLeadsToInvalidDeserialization)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a94b000-54ac-415a-a7c7-6f1348676f03");
    ServiceDescription::ClassHash testHash = {14U, 28U, 42U, 56U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    uint16_t invalidScope = 3U;
    auto serialObj = iox::Serialization::create(testService.c_str(),
                                                testInstance.c_str(),
                                                testEvent.c_str(),
                                                testHash[0],
                                                testHash[1],
                                                testHash[2],
                                                testHash[3],
                                                invalidScope);

    auto deserializationResult = ServiceDescription::deserialize(serialObj);

    ASSERT_TRUE(deserializationResult.has_error());
    EXPECT_THAT(deserializationResult.error(), Eq(iox::Serialization::Error::DESERIALIZATION_FAILED));
}

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test,
       ServiceDescriptionObjectInitialisationWithOutOfBoundaryInterfaceSourceLeadsToInvalidDeserialization)
{
    ::testing::Test::RecordProperty("TEST_ID", "29fac03f-a845-4180-89b7-8367a203646e");
    ServiceDescription::ClassHash testHash = {17U, 34U, 51U, 68U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    uint16_t testScope = 2U;
    uint16_t invalidInterfaceSource = 10U;
    auto serialObj = iox::Serialization::create(testService.c_str(),
                                                testInstance.c_str(),
                                                testEvent.c_str(),
                                                testHash[0],
                                                testHash[1],
                                                testHash[2],
                                                testHash[3],
                                                testScope,
                                                invalidInterfaceSource);

    auto deserializationResult = ServiceDescription::deserialize(serialObj);

    ASSERT_TRUE(deserializationResult.has_error());
    EXPECT_THAT(deserializationResult.error(), Eq(iox::Serialization::Error::DESERIALIZATION_FAILED));
}

TEST_F(ServiceDescription_test, ServiceDescriptionObjectInitialisationWithEmptyStringLeadsToInvalidDeserialization)
{
    ::testing::Test::RecordProperty("TEST_ID", "4607d73d-d27d-4694-833d-2e28162589cd");
    std::string emptyString;
    iox::Serialization invalidSerialObj{emptyString};

    auto deserializationResult = ServiceDescription::deserialize(invalidSerialObj);

    ASSERT_TRUE(deserializationResult.has_error());
    EXPECT_THAT(deserializationResult.error(), Eq(iox::Serialization::Error::DESERIALIZATION_FAILED));
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesStringsToEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "707156f8-8145-4710-b6ac-3e94dbac7237");
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getServiceIDString(), Eq(IdString_t()));
    EXPECT_THAT(serviceDescription1.getEventIDString(), Eq(IdString_t()));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), Eq(IdString_t()));
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheScopeToWorldWide)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e6b26b1-3363-45d8-abad-3b4c1ec122af");
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getScope(), Eq(Scope::WORLDWIDE));
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheInterfaceToLocal)
{
    ::testing::Test::RecordProperty("TEST_ID", "87c50b2a-d771-4985-8fdd-497a5f97dc35");
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERNAL));
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "560685b0-780c-420e-8f9d-bbfe2460d15f");
    testService = "1";
    testInstance = "2";
    testEvent = "3";
    ServiceDescription::ClassHash testHash = {12U, 23U, 34U, 45U};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_THAT(serviceDescription1.getServiceIDString(), Eq(IdString_t("1")));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), Eq(IdString_t("2")));
    EXPECT_THAT(serviceDescription1.getEventIDString(), Eq(IdString_t("3")));
    EXPECT_EQ(uint32_t(12), serviceDescription1.getClassHash()[0]);
    EXPECT_EQ(uint32_t(23), serviceDescription1.getClassHash()[1]);
    EXPECT_EQ(uint32_t(34), serviceDescription1.getClassHash()[2]);
    EXPECT_EQ(uint32_t(45), serviceDescription1.getClassHash()[3]);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidServicesAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "42329498-78b4-4cef-8629-918ca2783529");
    IdString_t testService1 = "1";
    IdString_t testEvent1 = "2";
    IdString_t testInstance1 = "3";
    IdString_t testService2 = "4";
    ServiceDescription serviceDescription1 = ServiceDescription(testService1, testEvent1, testInstance1);
    ServiceDescription serviceDescription2 = ServiceDescription(testService2, testEvent1, testInstance1);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidEventsAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a06cd60-af12-4bf8-abb7-ad42b301d879");
    IdString_t testService1 = "1";
    IdString_t testEvent1 = "2";
    IdString_t testInstance1 = "3";
    IdString_t testEvent2 = "4";
    ServiceDescription serviceDescription1 = ServiceDescription(testService1, testEvent1, testInstance1);
    ServiceDescription serviceDescription2 = ServiceDescription(testService1, testEvent2, testInstance1);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidInstancesAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1e13385-89b0-4aa0-9b97-8f39d5f5c0ae");
    IdString_t testService1 = "1";
    IdString_t testEvent1 = "2";
    IdString_t testInstance1 = "3";
    IdString_t testInstance2 = "4";
    ServiceDescription serviceDescription1 = ServiceDescription(testService1, testEvent1, testInstance1);
    ServiceDescription serviceDescription2 = ServiceDescription(testService1, testEvent1, testInstance2);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentAndValidServiceInstanceEventsAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0ab9583-802b-4d9f-b114-08e44be74e44");
    IdString_t testService1 = "1";
    IdString_t testEvent1 = "2";
    IdString_t testInstance1 = "3";
    IdString_t testService2 = "4";
    IdString_t testEvent2 = "5";
    IdString_t testInstance2 = "6";
    ServiceDescription serviceDescription1 = ServiceDescription(testService1, testEvent1, testInstance1);
    ServiceDescription serviceDescription2 = ServiceDescription(testService2, testEvent2, testInstance2);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithSameStringsComparedWithInequalityOperatorReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "1623a8a8-b892-45ce-a54a-ff13491069b7");
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);
    ServiceDescription serviceDescription2 = ServiceDescription(testService, testEvent, testInstance);

    EXPECT_FALSE(serviceDescription1 != serviceDescription2);
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsTrueIfTheServiceStringIsSame)
{
    ::testing::Test::RecordProperty("TEST_ID", "47bb698b-bb13-4885-afab-b5a975b67715");
    IdString_t sameService = "1";
    ServiceDescription description1 = ServiceDescription(sameService, "instance1", "event1");
    ServiceDescription description2 = ServiceDescription(sameService, "instance2", "event2");

    EXPECT_TRUE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsFalseIfTheServiceIDsAreDifferent)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ccd5f69-aca9-4e3d-9ba7-83581abde0f3");
    IdString_t serviceID1 = "1";
    IdString_t serviceID2 = "2";
    ServiceDescription description1 = ServiceDescription(serviceID1, "instance", "event");
    ServiceDescription description2 = ServiceDescription(serviceID2, "instance", "event");

    EXPECT_FALSE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, IsLocalMethodReturnsTrueWhenTheScopeIsSetToLocal)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc611c5d-484f-43c7-899e-12085d3e6018");
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);

    serviceDescription1.setLocal();

    EXPECT_TRUE(serviceDescription1.isLocal());
}

TEST_F(ServiceDescription_test, GetScopeMethodReturnsTheCorrespondingValueOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "ddc13a6b-a2aa-4271-b479-f4d4177d048e");
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);

    serviceDescription1.setLocal();

    EXPECT_EQ(serviceDescription1.getScope(), Scope::LOCAL);
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfServiceStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fe380cc-fa94-48e9-99dd-ec2e220eff16");
    ServiceDescription serviceDescription1("TestService1", "TestInstance", "TestEvent");
    ServiceDescription serviceDescription2("TestService2", "TestInstance", "TestEvent");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfInstanceStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5b0053e-9e7d-4176-80e1-52f057978c42");
    ServiceDescription serviceDescription1("TestService", "TestInstance1", "TestEvent");
    ServiceDescription serviceDescription2("TestService", "TestInstance2", "TestEvent");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfEventStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ab96b9a-5464-4b60-9d15-f31b5e3b4ee9");
    ServiceDescription serviceDescription1("TestService", "TestInstance", "TestEvent1");
    ServiceDescription serviceDescription2("TestService", "TestInstance", "TestEvent2");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

TEST_F(ServiceDescription_test, LogStreamConvertsServiceDescriptionToString)
{
    ::testing::Test::RecordProperty("TEST_ID", "42bc3f21-d9f4-4cc3-a37e-6508e1f981c1");
    iox::testing::Logger_Mock loggerMock;

    const IdString_t SERVICE_ID{"all"};
    const IdString_t INSTANCE_ID{"glory"};
    const IdString_t EVENT_ID{"hypnotoad"};
    const std::string SERVICE_DESCRIPTION_AS_STRING{"Service: all, Instance: glory, Event: hypnotoad"};
    auto sut = ServiceDescription{SERVICE_ID, INSTANCE_ID, EVENT_ID};

    {
        IOX_LOGSTREAM_MOCK(loggerMock) << sut;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(SERVICE_DESCRIPTION_AS_STRING));
}

/// END SERVICEDESCRIPTION TESTS

} // namespace
