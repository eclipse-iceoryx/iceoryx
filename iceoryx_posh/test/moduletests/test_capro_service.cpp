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

#include <cstdint>

using namespace ::testing;
using ::testing::Return;

using ServiceDescription = iox::capro::ServiceDescription;

class ServiceDescription_test : public Test
{
  public:
    uint16_t testAnyServiceID = iox::capro::AnyService;
    uint16_t testAnyEventID = iox::capro::AnyEvent;
    uint16_t testAnyInstanceID = iox::capro::AnyInstance;
    ServiceDescription::IdString service1 = "Service";
    ServiceDescription::IdString instance1 = "Instance";
    ServiceDescription::IdString event1 = "Event";
    std::string service2 = "Service";
    std::string instance2 = "Instance";
    std::string event2 = "Event";

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
    ServiceDescription csdStr = ServiceDescription(service2, instance2, event2);
    void SetUp(){};
    void TearDown(){};
};

TEST_F(ServiceDescription_test, CtorSerial)
{
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    auto serialObj = iox::cxx::Serialization::create(
        "Service", "Instance", "Event", 1, 2, 3, testHash[0], testHash[1], testHash[2], testHash[3], true);
    ServiceDescription csd = ServiceDescription(serialObj);
    csd.setInternal();
    EXPECT_THAT(csd.getServiceIDString(), Eq("Service"));
    EXPECT_THAT(csd.getInstanceIDString(), Eq("Instance"));
    EXPECT_THAT(csd.getEventIDString(), Eq("Event"));
    EXPECT_THAT(csd.getServiceID(), Eq(1));
    EXPECT_THAT(csd.getInstanceID(), Eq(2));
    EXPECT_THAT(csd.getEventID(), Eq(3));
    EXPECT_THAT((csd.getClassHash())[0], Eq(testHash[0]));
    EXPECT_THAT((csd.getClassHash())[1], Eq(testHash[1]));
    EXPECT_THAT((csd.getClassHash())[2], Eq(testHash[2]));
    EXPECT_THAT((csd.getClassHash())[3], Eq(testHash[3]));
    EXPECT_THAT(csd.hasServiceOnlyDescription(), Eq(true));
    EXPECT_THAT(csd.isInternal(), Eq(true));
}

TEST_F(ServiceDescription_test, CtorIDs)
{
    uint16_t testServiceID = 1;
    uint16_t testEventID = 2;
    uint16_t testInstanceID = 3;
    ServiceDescription csd = ServiceDescription(testServiceID, testEventID, testInstanceID);

    EXPECT_EQ(csd.getServiceID(), 1);
    EXPECT_EQ(csd.getEventID(), 2);
    EXPECT_EQ(csd.getInstanceID(), 3);

    EXPECT_THAT(csd.getServiceIDString(), StrEq("1"));
    EXPECT_THAT(csd.getEventIDString(), StrEq("2"));
    EXPECT_THAT(csd.getInstanceIDString(), StrEq("3"));
    EXPECT_THAT(csd.isInternal(), Eq(false)); // default should be not internal (backward compatible)
}

TEST_F(ServiceDescription_test, CtorNoParams)
{
    ServiceDescription csd = ServiceDescription();

    EXPECT_EQ(csd.getServiceID(), 0);
    EXPECT_EQ(csd.getEventID(), 0);
    EXPECT_EQ(csd.getInstanceID(), 0);

    EXPECT_THAT(csd.getServiceIDString(), StrEq("0"));
    EXPECT_THAT(csd.getEventIDString(), StrEq("0"));
    EXPECT_THAT(csd.getInstanceIDString(), StrEq("0"));
}

TEST_F(ServiceDescription_test, CtorIDStrings)
{
    std::string testService = "Service";
    std::string testInstance = "Instance";
    std::string testEvent = "Event";
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    ServiceDescription csd = ServiceDescription(testService, testInstance, testEvent, testHash);

    EXPECT_THAT(csd.getServiceIDString(), StrEq("Service"));
    EXPECT_THAT(csd.getInstanceIDString(), StrEq("Instance"));
    EXPECT_THAT(csd.getEventIDString(), StrEq("Event"));
    EXPECT_EQ(uint32_t(1), csd.getClassHash()[0]);
    EXPECT_EQ(uint32_t(2), csd.getClassHash()[1]);
    EXPECT_EQ(uint32_t(3), csd.getClassHash()[2]);
    EXPECT_EQ(uint32_t(4), csd.getClassHash()[3]);
}

TEST_F(ServiceDescription_test, operatorEq)
{
    EXPECT_TRUE(csd1 == csd1Eq);
    EXPECT_TRUE(csd2 == csd2Eq);
    EXPECT_TRUE(csd3 == csd3Eq);
    EXPECT_TRUE(csd4 == csd4Eq);

    EXPECT_FALSE(csd1 == csd1Ne);
    EXPECT_FALSE(csd2 == csd2Ne);
    EXPECT_FALSE(csd3 == csd3Ne);
    EXPECT_FALSE(csd4 == csd4Ne);
}

TEST_F(ServiceDescription_test, operatorNe)
{
    EXPECT_FALSE(csd1 != csd1Eq);
    EXPECT_FALSE(csd2 != csd2Eq);
    EXPECT_FALSE(csd3 != csd3Eq);
    EXPECT_FALSE(csd4 != csd4Eq);

    EXPECT_TRUE(csd1 != csd1Ne);
    EXPECT_TRUE(csd2 != csd2Ne);
    EXPECT_TRUE(csd3 != csd3Ne);
    EXPECT_TRUE(csd4 != csd4Ne);
}

TEST_F(ServiceDescription_test, operatorAssign)
{
    ServiceDescription csdNew = csd1;
    EXPECT_TRUE(csdNew == csd1);
}

TEST_F(ServiceDescription_test, getServiceString)
{
    EXPECT_THAT(csd1.getServiceString(), StrEq("Service_0001_0002_0003"));
    EXPECT_THAT(csd2.getServiceString(), StrEq("Service_0001_FFFF_0003"));
}

TEST_F(ServiceDescription_test, string100)
{
    ServiceDescription::IdString service1 = "Service";
    ServiceDescription::IdString instance1 = "Instance";
    ServiceDescription::IdString event1 = "Event";
    ServiceDescription csdFixed = ServiceDescription(service1, instance1, event1);

    std::string service2 = "Service";
    std::string instance2 = "Instance";
    std::string event2 = "Event";
    ServiceDescription csd = ServiceDescription(service2, instance2, event2);

    EXPECT_TRUE(csd.getServiceIDString() == csdFixed.getServiceIDString());
    EXPECT_TRUE(csd.getInstanceIDString() == csdFixed.getInstanceIDString());
    EXPECT_TRUE(csd.getEventIDString() == csdFixed.getEventIDString());

    EXPECT_THAT(csd.getInstanceID(), Eq(iox::capro::InvalidID));
    EXPECT_THAT(csdFixed.getInstanceID(), Eq(iox::capro::InvalidID));

    EXPECT_TRUE(csd == csdFixed);
}

TEST_F(ServiceDescription_test, CreateServiceOnlyDescription)
{
    ServiceDescription desc1(1u, 2u);
    ServiceDescription desc2("bla", "fuh");
    ServiceDescription desc3(1u, 2u, 3u);
    ServiceDescription desc4("bla", "fuh", "dumbledoodoo");

    EXPECT_TRUE(desc1.hasServiceOnlyDescription());
    EXPECT_TRUE(desc2.hasServiceOnlyDescription());
    EXPECT_FALSE(desc3.hasServiceOnlyDescription());
    EXPECT_FALSE(desc4.hasServiceOnlyDescription());
}

TEST_F(ServiceDescription_test, defaultClassHash)
{
    EXPECT_EQ(uint32_t(0), csdIdStr.getClassHash()[0]);
    EXPECT_EQ(uint32_t(0), csdIdStr.getClassHash()[1]);
    EXPECT_EQ(uint32_t(0), csdIdStr.getClassHash()[2]);
    EXPECT_EQ(uint32_t(0), csdIdStr.getClassHash()[3]);
}

TEST_F(ServiceDescription_test, CopyAssignmentClassHash)
{
    ServiceDescription desc1("TestService", "TestInstance", "TestEvent", {1, 2, 3, 4});
    ServiceDescription desc2 = desc1;
    EXPECT_EQ(desc2.getClassHash()[0], desc1.getClassHash()[0]);
    EXPECT_EQ(desc2.getClassHash()[1], desc1.getClassHash()[1]);
    EXPECT_EQ(desc2.getClassHash()[2], desc1.getClassHash()[2]);
    EXPECT_EQ(desc2.getClassHash()[3], desc1.getClassHash()[3]);
}
