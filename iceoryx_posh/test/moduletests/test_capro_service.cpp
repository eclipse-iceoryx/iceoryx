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

#include "test.hpp"

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/cxx/serialization.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include <cstdint>

namespace
{
using namespace ::testing;

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
    ServiceDescription::ClassHash testHash{};

    EXPECT_EQ(uint32_t(0), testHash[0]);
    EXPECT_EQ(uint32_t(0), testHash[1]);
    EXPECT_EQ(uint32_t(0), testHash[2]);
    EXPECT_EQ(uint32_t(0), testHash[3]);
}

TEST_F(ServiceDescription_test, ServiceDescriptionClassHashCtorCreatesClassHashWithValuesPassedToTheCtor)
{
    ServiceDescription::ClassHash testHash{1U, 2U, 3U, 4U};

    EXPECT_EQ(uint32_t(1), testHash[0]);
    EXPECT_EQ(uint32_t(2), testHash[1]);
    EXPECT_EQ(uint32_t(3), testHash[2]);
    EXPECT_EQ(uint32_t(4), testHash[3]);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithEqualityOperatorReturnsFalse)
{
    ServiceDescription::ClassHash testHash1{15U, 25U, 35U, 45U};
    ServiceDescription::ClassHash testHash2{55U, 65U, 75U, 85U};

    EXPECT_FALSE(testHash1 == testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithEqualityOperatorReturnsTrue)
{
    ServiceDescription::ClassHash testHash1{10U, 20U, 30U, 40U};
    ServiceDescription::ClassHash testHash2{10U, 20U, 30U, 40U};

    EXPECT_TRUE(testHash1 == testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoUnequalClassHashWithNotEqualOperatorReturnsTrue)
{
    ServiceDescription::ClassHash testHash1{12U, 24U, 36U, 48U};
    ServiceDescription::ClassHash testHash2{60U, 72U, 84U, 96U};

    EXPECT_TRUE(testHash1 != testHash2);
}

TEST_F(ServiceDescription_test, ComparingTwoEqualClassHashWithNotEqualOperatorReturnsFalse)
{
    ServiceDescription::ClassHash testHash1{11U, 22U, 33U, 44U};
    ServiceDescription::ClassHash testHash2{11U, 22U, 33U, 44U};

    EXPECT_FALSE(testHash1 != testHash2);
}

TEST_F(ServiceDescription_test, ClassHashWithValuesAssignedUsingAssignmentOperatorStoresTheValueInTheCorrespondingIndex)
{
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
    ServiceDescription::ClassHash testHash{1U, 2U, 3U, 4U};

    testHash[0] = 1U;
    testHash[1] = 2U;
    testHash[2] = 3U;
    testHash[3] = 4U;

    EXPECT_DEATH({ testHash[4] = 5U; }, ".*");
}

/// END CLASSHASH TESTS

/// BEGIN SERVICEDESCRIPTION TESTS

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test, ServiceDescriptionSerializationCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    ServiceDescription::ClassHash testHash = {11U, 21U, 31U, 41U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    Scope testScope = Scope::INTERNAL;
    Interfaces testInterfaceSource = Interfaces::INTERNAL;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     static_cast<uint16_t>(testScope),
                                                     static_cast<uint16_t>(testInterfaceSource));

    ServiceDescription serviceDescription1 = ServiceDescription(serialObj);

    EXPECT_THAT(serviceDescription1.getServiceIDString(), Eq(testService));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), Eq(testInstance));
    EXPECT_THAT(serviceDescription1.getEventIDString(), Eq(testEvent));
    EXPECT_THAT((serviceDescription1.getClassHash())[0], Eq(testHash[0]));
    EXPECT_THAT((serviceDescription1.getClassHash())[1], Eq(testHash[1]));
    EXPECT_THAT((serviceDescription1.getClassHash())[2], Eq(testHash[2]));
    EXPECT_THAT((serviceDescription1.getClassHash())[3], Eq(testHash[3]));
    EXPECT_THAT(serviceDescription1.getScope(), Eq(Scope::INTERNAL));
    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERNAL));
}

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test, ServiceDescriptionObjectInitialisationWithOutOfBoundaryScopeSetsTheScopeToInvalid)
{
    ServiceDescription::ClassHash testHash = {14U, 28U, 42U, 56U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    uint16_t invalidScope = 3U;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     invalidScope);

    ServiceDescription serviceDescription1 = ServiceDescription(serialObj);

    EXPECT_THAT(serviceDescription1.getScope(), Eq(Scope::INVALID));
}

/// @attention The purpose of the Serialization is not to be an alternative Constructor. It is intended to send/receive
/// the ServiceDescription over communication protocols which transfers strings like the MessageQueue. The testcase is
/// only intended to check the functionality by injecting the valus directly.
TEST_F(ServiceDescription_test,
       ServiceDescriptionObjectInitialisationWithOutOfBoundaryInterfaceSourceSetsTheInterfaceSourceToInterfaceEnd)
{
    ServiceDescription::ClassHash testHash = {17U, 34U, 51U, 68U};
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    uint16_t testScope = 2U;
    uint16_t invalidInterfaceSource = 10U;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     static_cast<uint16_t>(testScope),
                                                     invalidInterfaceSource);

    ServiceDescription serviceDescription1 = ServiceDescription(serialObj);

    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERFACE_END));
}

/// @todo remove
TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesStringsToInvalidString)
{
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getServiceIDString(), StrEq(InvalidString));
    EXPECT_THAT(serviceDescription1.getEventIDString(), StrEq(InvalidString));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), StrEq(InvalidString));
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheScopeToWorldWide)
{
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getScope(), Eq(Scope::WORLDWIDE));
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheInterfaceToInternal)
{
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERNAL));
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    testService = "1";
    testInstance = "2";
    testEvent = "3";
    ServiceDescription::ClassHash testHash = {12U, 23U, 34U, 45U};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_THAT(serviceDescription1.getServiceIDString(), StrEq("1"));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), StrEq("2"));
    EXPECT_THAT(serviceDescription1.getEventIDString(), StrEq("3"));
    EXPECT_EQ(uint32_t(12), serviceDescription1.getClassHash()[0]);
    EXPECT_EQ(uint32_t(23), serviceDescription1.getClassHash()[1]);
    EXPECT_EQ(uint32_t(34), serviceDescription1.getClassHash()[2]);
    EXPECT_EQ(uint32_t(45), serviceDescription1.getClassHash()[3]);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithAnyServiceAnyInstanceAnyEventIDsAreEqual)
{
    IdString_t testService = iox::capro::AnyServiceString;
    IdString_t testEvent = iox::capro::AnyEventString;
    IdString_t testInstance = iox::capro::AnyInstanceString;
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);
    ServiceDescription serviceDescription2 = ServiceDescription(testService, testEvent, testInstance);

    EXPECT_TRUE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionWithAnyServiceAnyInstanceAnyEventAndServiceDescriptionWithValidStringsAreEqual)
{
    IdString_t testService1 = "1";
    IdString_t testEvent1 = "2";
    IdString_t testInstance1 = "3";
    IdString_t testService2 = iox::capro::AnyServiceString;
    IdString_t testEvent2 = iox::capro::AnyEventString;
    IdString_t testInstance2 = iox::capro::AnyInstanceString;
    ServiceDescription serviceDescription1 = ServiceDescription(testService1, testEvent1, testInstance1);
    ServiceDescription serviceDescription2 = ServiceDescription(testService2, testEvent2, testInstance2);

    EXPECT_TRUE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidServicesAreNotEqual)
{
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
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);
    ServiceDescription serviceDescription2 = ServiceDescription(testService, testEvent, testInstance);

    EXPECT_FALSE(serviceDescription1 != serviceDescription2);
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsTrueIfTheServiceStringIsSame)
{
    IdString_t sameService = "1";
    ServiceDescription description1 =
        ServiceDescription(sameService, iox::capro::AnyEventString, iox::capro::AnyInstanceString);
    ServiceDescription description2 =
        ServiceDescription(sameService, iox::capro::AnyEventString, iox::capro::AnyInstanceString);

    EXPECT_TRUE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsFalseIfTheServiceIDsAreDifferent)
{
    IdString_t serviceID1 = "1";
    IdString_t serviceID2 = "2";
    ServiceDescription description1 =
        ServiceDescription(serviceID1, iox::capro::AnyEventString, iox::capro::AnyInstanceString);
    ServiceDescription description2 =
        ServiceDescription(serviceID2, iox::capro::AnyEventString, iox::capro::AnyInstanceString);

    EXPECT_FALSE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, IsInternalMethodReturnsTrueWhenTheScopeIsSetToInternal)
{
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);

    serviceDescription1.setInternal();

    EXPECT_TRUE(serviceDescription1.isInternal());
}

TEST_F(ServiceDescription_test, GetScopeMethodReturnsTheCorrespondingValueOfScope)
{
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "3";
    ServiceDescription serviceDescription1 = ServiceDescription(testService, testEvent, testInstance);

    serviceDescription1.setInternal();

    EXPECT_EQ(serviceDescription1.getScope(), Scope::INTERNAL);
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenServiceIDIsInvalid)
{
    IdString_t testServiceID = "INVALID";
    IdString_t testEventID = "1";
    IdString_t testInstanceID = "1";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenServiceIDIsAnyService)
{
    IdString_t testServiceID = iox::capro::AnyServiceString;
    IdString_t testEventID = "1";
    IdString_t testInstanceID = "1";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenInstanceIDIsInvalid)
{
    IdString_t testServiceID = "1";
    IdString_t testEventID = "1";
    IdString_t testInstanceID = "INVALID";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenInstanceIDIsAnyInstance)
{
    IdString_t testServiceID = "1";
    IdString_t testEventID = "1";
    IdString_t testInstanceID = iox::capro::AnyInstanceString;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenEventIDIsInvalid)
{
    IdString_t testServiceID = "1";
    IdString_t testEventID = InvalidString;
    IdString_t testInstanceID = "1";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhenEventIDIsAnyEvent)
{
    IdString_t testServiceID = "1";
    IdString_t testEventID = iox::capro::AnyEventString;
    IdString_t testInstanceID = "1";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsValidWhenServiceInstanceAndEventIDsAreValid)
{
    IdString_t testServiceID = "1";
    IdString_t testEventID = "1";
    IdString_t testInstanceID = "1";
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_TRUE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfServiceStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ServiceDescription serviceDescription1("TestService1", "TestInstance", "TestEvent");
    ServiceDescription serviceDescription2("TestService2", "TestInstance", "TestEvent");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfInstanceStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ServiceDescription serviceDescription1("TestService", "TestInstance1", "TestEvent");
    ServiceDescription serviceDescription2("TestService", "TestInstance2", "TestEvent");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

TEST_F(ServiceDescription_test, LessThanOperatorReturnsFalseIfEventStringOfFirstServiceDescriptionIsLessThanSecond)
{
    ServiceDescription serviceDescription1("TestService", "TestInstance", "TestEvent1");
    ServiceDescription serviceDescription2("TestService", "TestInstance", "TestEvent2");

    EXPECT_FALSE(serviceDescription1 < serviceDescription2);
}

/// @todo add new tests for service description?

/// END SERVICEDESCRIPTION TESTS

} // namespace
