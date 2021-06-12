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
    uint16_t testServiceID = 1U;
    uint16_t testInstanceID = 2U;
    uint16_t testEventID = 3U;
    Scope testScope = Scope::INTERNAL;
    Interfaces testInterfaceSource = Interfaces::INTERNAL;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testServiceID,
                                                     testInstanceID,
                                                     testEventID,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     false,
                                                     static_cast<uint16_t>(testScope),
                                                     static_cast<uint16_t>(testInterfaceSource));

    ServiceDescription serviceDescription1 = ServiceDescription(serialObj);

    EXPECT_THAT(serviceDescription1.getServiceIDString(), Eq(testService));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), Eq(testInstance));
    EXPECT_THAT(serviceDescription1.getEventIDString(), Eq(testEvent));
    EXPECT_THAT(serviceDescription1.getServiceID(), Eq(testServiceID));
    EXPECT_THAT(serviceDescription1.getInstanceID(), Eq(testInstanceID));
    EXPECT_THAT(serviceDescription1.getEventID(), Eq(testEventID));
    EXPECT_THAT((serviceDescription1.getClassHash())[0], Eq(testHash[0]));
    EXPECT_THAT((serviceDescription1.getClassHash())[1], Eq(testHash[1]));
    EXPECT_THAT((serviceDescription1.getClassHash())[2], Eq(testHash[2]));
    EXPECT_THAT((serviceDescription1.getClassHash())[3], Eq(testHash[3]));
    EXPECT_THAT(serviceDescription1.hasServiceOnlyDescription(), Eq(false));
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
    uint16_t testServiceID = 1U;
    uint16_t testInstanceID = 2U;
    uint16_t testEventID = 3U;
    uint16_t invalidScope = 3U;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testServiceID,
                                                     testInstanceID,
                                                     testEventID,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true,
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
    uint16_t testServiceID = 1U;
    uint16_t testInstanceID = 2U;
    uint16_t testEventID = 3U;
    uint16_t testScope = 2U;
    uint16_t invalidInterfaceSource = 10U;
    auto serialObj = iox::cxx::Serialization::create(testService.c_str(),
                                                     testInstance.c_str(),
                                                     testEvent.c_str(),
                                                     testServiceID,
                                                     testInstanceID,
                                                     testEventID,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true,
                                                     static_cast<uint16_t>(testScope),
                                                     invalidInterfaceSource);

    ServiceDescription serviceDescription1 = ServiceDescription(serialObj);

    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERFACE_END));
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionCtorWithOnlyIDsCreatesServiceDescriptionWithTheSameServiceEventAndInstanceStringsAsIDs)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_THAT(iox::cxx::convert::toString(serviceDescription1.getServiceID()),
                serviceDescription1.getServiceIDString());
    EXPECT_THAT(iox::cxx::convert::toString(serviceDescription1.getEventID()), serviceDescription1.getEventIDString());
    EXPECT_THAT(iox::cxx::convert::toString(serviceDescription1.getInstanceID()),
                serviceDescription1.getInstanceIDString());
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheIDsAndStringsToZero)
{
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_EQ(serviceDescription1.getServiceID(), 0U);
    EXPECT_EQ(serviceDescription1.getEventID(), 0U);
    EXPECT_EQ(serviceDescription1.getInstanceID(), 0U);
    EXPECT_THAT(serviceDescription1.getServiceIDString(), StrEq("0"));
    EXPECT_THAT(serviceDescription1.getEventIDString(), StrEq("0"));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), StrEq("0"));
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
    EXPECT_EQ(uint16_t(1), serviceDescription1.getServiceID());
    EXPECT_EQ(uint16_t(2), serviceDescription1.getInstanceID());
    EXPECT_EQ(uint16_t(3), serviceDescription1.getEventID());
    EXPECT_EQ(uint32_t(12), serviceDescription1.getClassHash()[0]);
    EXPECT_EQ(uint32_t(23), serviceDescription1.getClassHash()[1]);
    EXPECT_EQ(uint32_t(34), serviceDescription1.getClassHash()[2]);
    EXPECT_EQ(uint32_t(45), serviceDescription1.getClassHash()[3]);
}
TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorWithNonIntegerStringValuesSetTheIDsToInvalid)
{
    testService = "Service";
    testInstance = "Instance";
    testEvent = "Event";
    ServiceDescription::ClassHash testHash = {01U, 02U, 03U, 04U};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_EQ(InvalidID, serviceDescription1.getServiceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getInstanceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getEventID());
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorWithZeroAsStringValuesSetTheIDsToInvalid)
{
    testService = "0";
    testInstance = "0";
    testEvent = "0";
    ServiceDescription::ClassHash testHash = {19U, 29U, 39U, 49U};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_EQ(InvalidID, serviceDescription1.getServiceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getInstanceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getEventID());
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorWithOutOfBoundaryIntegerStringValuesSetTheIDsToInvalid)
{
    IdString_t outOfBoundaryTestService(
        iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(uint32_t(1) + std::numeric_limits<uint16_t>::max()));
    IdString_t outOfBoundaryTestInstance(
        iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(uint32_t(1) + std::numeric_limits<uint16_t>::max()));
    IdString_t outOfBoundaryTestEvent(iox::cxx::TruncateToCapacity,
                                      iox::cxx::convert::toString(uint32_t(1) + std::numeric_limits<uint16_t>::max()));
    ServiceDescription::ClassHash testHash = {1U, 2U, 3U, 4U};

    ServiceDescription serviceDescription1 =
        ServiceDescription(outOfBoundaryTestService, outOfBoundaryTestInstance, outOfBoundaryTestEvent, testHash);

    EXPECT_EQ(InvalidID, serviceDescription1.getServiceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getInstanceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getEventID());
}

TEST_F(ServiceDescription_test, ServiceDescriptionCtorWithServiceIDAndInstanceIDSetsHasServiceDescriptionTrue)
{
    uint16_t testService = 1U;
    uint16_t testInstance = 2U;

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance);

    EXPECT_TRUE(serviceDescription1.hasServiceOnlyDescription());
}

TEST_F(ServiceDescription_test, ServiceDescriptionCtorWithServiceStringAndInstanceStringSetsHasServiceDescriptionTrue)
{
    testService = "Service";
    testInstance = "Instance";

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance);

    EXPECT_TRUE(serviceDescription1.hasServiceOnlyDescription());
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithAnyServiceAnyInstanceAnyEventIDsAreEqual)
{
    uint16_t testServiceID = iox::capro::AnyService;
    uint16_t testEventID = iox::capro::AnyEvent;
    uint16_t testInstanceID = iox::capro::AnyInstance;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_TRUE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionWithAnyServiceAnyInstanceAnyEventIDAndServiceDescriptionWithValidIDsAreEqual)
{
    uint16_t testServiceID1 = 1U;
    uint16_t testEventID1 = 2U;
    uint16_t testInstanceID1 = 3U;
    uint16_t testServiceID2 = iox::capro::AnyService;
    uint16_t testEventID2 = iox::capro::AnyEvent;
    uint16_t testInstanceID2 = iox::capro::AnyInstance;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID1, testEventID1, testInstanceID1);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID2, testEventID2, testInstanceID2);

    EXPECT_TRUE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidServiceIDsAreNotEqual)
{
    uint16_t testServiceID1 = 1U;
    uint16_t testEventID1 = 2U;
    uint16_t testInstanceID1 = 3U;
    uint16_t testServiceID2 = 4U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID1, testEventID1, testInstanceID1);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID2, testEventID1, testInstanceID1);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidEventIDsAreNotEqual)
{
    uint16_t testServiceID1 = 1U;
    uint16_t testEventID1 = 2U;
    uint16_t testInstanceID1 = 3U;
    uint16_t testEventID2 = 4U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID1, testEventID1, testInstanceID1);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID1, testEventID2, testInstanceID1);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentButValidInstanceIDsAreNotEqual)
{
    uint16_t testServiceID1 = 1U;
    uint16_t testEventID1 = 2U;
    uint16_t testInstanceID1 = 3U;
    uint16_t testInstanceID2 = 4U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID1, testEventID1, testInstanceID1);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID1, testEventID1, testInstanceID2);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentAndValidServiceInstanceEventIDsAreNotEqual)
{
    uint16_t testServiceID1 = 1U;
    uint16_t testEventID1 = 2U;
    uint16_t testInstanceID1 = 3U;
    uint16_t testServiceID2 = 4U;
    uint16_t testEventID2 = 5U;
    uint16_t testInstanceID2 = 6U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID1, testEventID1, testInstanceID1);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID2, testEventID2, testInstanceID2);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentInstanceStringsAreNotEqual)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = InvalidID;
    testService = "1";
    testEvent = "2";
    testInstance = "instance";

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(testService, testInstance, testEvent);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentEventStringsAreNotEqual)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = InvalidID;
    uint16_t testInstanceID = 3U;
    testService = "1";
    testEvent = "event";
    testInstance = "3";

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(testService, testInstance, testEvent);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithSameIDsComparedWithInequalityOperatorReturnsFalse)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1 != serviceDescription2);
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsTrueIfTheServiceIDsAreSame)
{
    uint16_t sameserviceID = 1U;
    ServiceDescription description1 = ServiceDescription(sameserviceID, iox::capro::AnyEvent, iox::capro::AnyInstance);
    ServiceDescription description2 = ServiceDescription(sameserviceID, iox::capro::AnyEvent, iox::capro::AnyInstance);

    EXPECT_TRUE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, ServiceMatchMethodReturnsFalseIfTheServiceIDsAreDifferent)
{
    uint16_t serviceID1 = 1U;
    uint16_t serviceID2 = 2U;
    ServiceDescription description1 = ServiceDescription(serviceID1, iox::capro::AnyEvent, iox::capro::AnyInstance);
    ServiceDescription description2 = ServiceDescription(serviceID2, iox::capro::AnyEvent, iox::capro::AnyInstance);

    EXPECT_FALSE(iox::capro::serviceMatch(description1, description2));
}

TEST_F(ServiceDescription_test, IsInternalMethodReturnsTrueWhenTheScopeIsSetToInternal)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    serviceDescription1.setInternal();

    EXPECT_TRUE(serviceDescription1.isInternal());
}

TEST_F(ServiceDescription_test, GetScopeMethodReturnsTheCorrespondingValueOfScope)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    serviceDescription1.setInternal();

    EXPECT_EQ(serviceDescription1.getScope(), Scope::INTERNAL);
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsTrueAndServiceStringIsInvalid)
{
    testService = iox::capro::InvalidIDString;
    testInstance = "validinstance";

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsTrueAndServiceIDIsAnyService)
{
    uint16_t testServiceID = iox::capro::AnyService;
    uint16_t testInstanceID = iox::capro::AnyInstance;

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsTrueAndInstanceStringIsInvalid)
{
    testService = "validservice";
    testInstance = iox::capro::InvalidIDString;

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsTrueAndInstanceIDIsAnyInstance)
{
    uint16_t testServiceID = 1U;
    uint16_t testInstanceID = iox::capro::AnyInstance;

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsValidWhen_m_hasServiceOnlyDescriptionIsTrueAndServiceAndInstanceAreValid)
{
    uint16_t testServiceID = 1U;
    uint16_t testInstanceID = 2U;

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testInstanceID);

    EXPECT_TRUE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndServiceIDIsInvalid)
{
    uint16_t testServiceID = 0U;
    uint16_t testEventID = 1U;
    uint16_t testInstanceID = 1U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndServiceIDIsAnyService)
{
    uint16_t testServiceID = iox::capro::AnyService;
    uint16_t testEventID = 1U;
    uint16_t testInstanceID = 1U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndInstanceIDIsInvalid)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 1U;
    uint16_t testInstanceID = 0U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndInstanceIDIsAnyInstance)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 1U;
    uint16_t testInstanceID = iox::capro::AnyInstance;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndEventIDIsInvalid)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 0U;
    uint16_t testInstanceID = 1U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test, ServiceDescriptionIsInvalidWhen_m_hasServiceOnlyDescriptionIsFalseAndEventIDIsAnyEvent)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = iox::capro::AnyEvent;
    uint16_t testInstanceID = 1U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1.isValid());
}

TEST_F(ServiceDescription_test,
       ServiceDescriptionIsValidWhen_m_hasServiceOnlyDescriptionIsFalseAndServiceInstanceAndEventIDsAreValid)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 1U;
    uint16_t testInstanceID = 1U;
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
