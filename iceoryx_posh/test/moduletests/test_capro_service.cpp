// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/serialization.hpp"
#include "iceoryx_utils/cxx/string.hpp"

#include <cstdint>

using namespace ::testing;
using ::testing::Return;

using namespace iox::capro;

class ServiceDescription_test : public Test
{
  public:
    uint16_t testAnyServiceID = iox::capro::AnyService;
    uint16_t testAnyEventID = iox::capro::AnyEvent;
    uint16_t testAnyInstanceID = iox::capro::AnyInstance;
    IdString_t service1{"Service"};
    IdString_t instance1{"Instance"};
    IdString_t event1{"Event"};

    ServiceDescription csd1 = ServiceDescription(1, 2, 3);
    ServiceDescription csd1Eq = ServiceDescription(testAnyServiceID, 2, 3);
    ServiceDescription csd1Ne = ServiceDescription(testAnyServiceID, 9, 3);
    ServiceDescription csd2 = ServiceDescription(1, testAnyEventID, 3);
    ServiceDescription csd2Eq = ServiceDescription(1, 2, 3);
    ServiceDescription csd2Ne = ServiceDescription(1, 2, 9);
    ServiceDescription csd3 = ServiceDescription(1, 2, testAnyInstanceID);
    ServiceDescription csd3Eq = ServiceDescription(1, 2, testAnyInstanceID);
    ServiceDescription csd3Ne = ServiceDescription(9, 2, testAnyInstanceID);
    ServiceDescription csd4 = ServiceDescription(1, 2, 3);
    ServiceDescription csd4Eq = ServiceDescription(1, 2, 3);
    ServiceDescription csd4Ne = ServiceDescription(9, 9, 9);
    ServiceDescription csdIdStr = ServiceDescription(service1, instance1, event1);
    void SetUp(){};
    void TearDown(){};
};

TEST_F(ServiceDescription_test, ServiceDescriptionSerializationCtorCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    IdString_t testService{"Service"};
    IdString_t testInstance{"Instance"};
    IdString_t testEvent{"Event"};
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
                                                     true,
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
    EXPECT_THAT(serviceDescription1.hasServiceOnlyDescription(), Eq(true));
    EXPECT_THAT(serviceDescription1.getScope(), Eq(Scope::INTERNAL));
    EXPECT_THAT(serviceDescription1.getSourceInterface(), Eq(Interfaces::INTERNAL));
}

TEST_F(ServiceDescription_test, ServiceDescriptionObjectInitialisationWithOutOfBoundaryScopeSetsTheScopeToInvalid)
{
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    IdString_t testService{"Service"};
    IdString_t testInstance{"Instance"};
    IdString_t testEvent{"Event"};
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

TEST_F(ServiceDescription_test,
       ServiceDescriptionObjectInitialisationWithOutOfBoundaryInterfaceSourceSetsTheInterfaceSourceToInterfaceEnd)
{
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    IdString_t testService{"Service"};
    IdString_t testInstance{"Instance"};
    IdString_t testEvent{"Event"};
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

    EXPECT_THAT(std::to_string(serviceDescription1.getServiceID()), serviceDescription1.getServiceIDString());
    EXPECT_THAT(std::to_string(serviceDescription1.getEventID()), serviceDescription1.getEventIDString());
    EXPECT_THAT(std::to_string(serviceDescription1.getInstanceID()), serviceDescription1.getInstanceIDString());
}

TEST_F(ServiceDescription_test, ServiceDescriptionDefaultCtorInitializesTheIDsAndStringsToZero)
{
    ServiceDescription serviceDescription1 = ServiceDescription();

    EXPECT_EQ(serviceDescription1.getServiceID(), 0);
    EXPECT_EQ(serviceDescription1.getEventID(), 0);
    EXPECT_EQ(serviceDescription1.getInstanceID(), 0);
    EXPECT_THAT(serviceDescription1.getServiceIDString(), StrEq("0"));
    EXPECT_THAT(serviceDescription1.getEventIDString(), StrEq("0"));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), StrEq("0"));
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorCreatesServiceDescriptionWithValuesPassedToTheCtor)
{
    IdString_t testService("1");
    IdString_t testInstance("2");
    IdString_t testEvent("3");
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_THAT(serviceDescription1.getServiceIDString(), StrEq("1"));
    EXPECT_THAT(serviceDescription1.getInstanceIDString(), StrEq("2"));
    EXPECT_THAT(serviceDescription1.getEventIDString(), StrEq("3"));
    EXPECT_EQ(uint16_t(1), serviceDescription1.getServiceID());
    EXPECT_EQ(uint16_t(2), serviceDescription1.getInstanceID());
    EXPECT_EQ(uint16_t(3), serviceDescription1.getEventID());
    EXPECT_EQ(uint32_t(1), serviceDescription1.getClassHash()[0]);
    EXPECT_EQ(uint32_t(2), serviceDescription1.getClassHash()[1]);
    EXPECT_EQ(uint32_t(3), serviceDescription1.getClassHash()[2]);
    EXPECT_EQ(uint32_t(4), serviceDescription1.getClassHash()[3]);
}
TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorWithNonIntegerStringValuesSetTheIDsToInvalid)
{
    IdString_t testService("Service");
    IdString_t testInstance("Instance");
    IdString_t testEvent("Event");
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_EQ(InvalidID, serviceDescription1.getServiceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getInstanceID());
    EXPECT_EQ(InvalidID, serviceDescription1.getEventID());
}

TEST_F(ServiceDescription_test, ServiceDescriptionStringCtorWithOutOfBoundaryIntegerStringValuesSetTheIDsToInvalid)
{
    IdString_t testService("65536");
    IdString_t testInstance("65536");
    IdString_t testEvent("65536");
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};

    ServiceDescription serviceDescription1 = ServiceDescription(testService, testInstance, testEvent, testHash);

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
    IdString_t testService("Service");
    IdString_t testInstance("Instance");

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
    uint16_t testInstanceID = 3U;
    IdString_t testService = "1";
    IdString_t testEvent = "2";
    IdString_t testInstance = "instance";
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    auto serialObj = iox::cxx::Serialization::create(testService,
                                                     testInstance,
                                                     testEvent,
                                                     testServiceID,
                                                     testInstanceID,
                                                     testEventID,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true);

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(serialObj);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentEventStringsAreNotEqual)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;
    IdString_t testService = "1";
    IdString_t testEvent = "event";
    IdString_t testInstance = "3";
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    auto serialObj = iox::cxx::Serialization::create(testService,
                                                     testInstance,
                                                     testEvent,
                                                     testServiceID,
                                                     testInstanceID,
                                                     testEventID,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true);

    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(serialObj);

    EXPECT_FALSE(serviceDescription1 == serviceDescription2);
}

TEST_F(ServiceDescription_test, TwoServiceDescriptionsWithDifferentValidIDsAreNotEqual)
{
    uint16_t testServiceID = 1U;
    uint16_t testEventID = 2U;
    uint16_t testInstanceID = 3U;
    ServiceDescription serviceDescription1 = ServiceDescription(testServiceID, testEventID, testInstanceID);
    ServiceDescription serviceDescription2 = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_FALSE(serviceDescription1 != serviceDescription2);
}

TEST_F(ServiceDescription_test, ServiceDescriptionClassHashCtorCreatesClassHashWithValuesPassedToTheCtor)
{
    ServiceDescription::ClassHash testHash{1, 2, 3, 4};

    EXPECT_EQ(uint32_t(1), testHash[0]);
    EXPECT_EQ(uint32_t(2), testHash[1]);
    EXPECT_EQ(uint32_t(3), testHash[2]);
    EXPECT_EQ(uint32_t(4), testHash[3]);
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
    ServiceDescription description2 =
        ServiceDescription(serviceID2, iox::capro::AnyEvent, iox::capro::AnyInstance);

    EXPECT_FALSE(iox::capro::serviceMatch(description1, description2));
}
