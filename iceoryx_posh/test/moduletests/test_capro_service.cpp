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

TEST_F(ServiceDescription_test, CtorSerial)
{
    ServiceDescription::ClassHash testHash = {1, 2, 3, 4};
    auto serialObj = iox::cxx::Serialization::create(service1.c_str(),
                                                     instance1.c_str(),
                                                     event1.c_str(),
                                                     1,
                                                     2,
                                                     3,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true);
    ServiceDescription csd = ServiceDescription(serialObj);
    csd.setInternal();
    EXPECT_THAT(csd.getServiceIDString(), Eq(service1));
    EXPECT_THAT(csd.getInstanceIDString(), Eq(instance1));
    EXPECT_THAT(csd.getEventIDString(), Eq(event1));
    EXPECT_THAT(csd.getServiceID(), Eq(1));
    EXPECT_THAT(csd.getInstanceID(), Eq(2));
    EXPECT_THAT(csd.getEventID(), Eq(3));
    EXPECT_THAT((csd.getClassHash())[0], Eq(testHash[0]));
    EXPECT_THAT((csd.getClassHash())[1], Eq(testHash[1]));
    EXPECT_THAT((csd.getClassHash())[2], Eq(testHash[2]));
    EXPECT_THAT((csd.getClassHash())[3], Eq(testHash[3]));
    EXPECT_THAT(csd.hasServiceOnlyDescription(), Eq(true));
    EXPECT_THAT(csd.isInternal(), Eq(true));
    auto  serialObj2 = iox::cxx::Serialization::create(service1.c_str(),
                                                     instance1.c_str(),
                                                     event1.c_str(),
                                                     1,
                                                     2,
                                                     3,
                                                     testHash[0],
                                                     testHash[1],
                                                     testHash[2],
                                                     testHash[3],
                                                     true,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd2 = ServiceDescription(serialObj2);
    EXPECT_THAT(csd2.getSourceInterface(), Eq(Interfaces::INTERFACE_END));

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
    IdString_t testService("Service");
    IdString_t testInstance("Instance");
    IdString_t testEvent("Event");
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

TEST_F(ServiceDescription_test, EqualityOperatorClassHash)
{
    ServiceDescription desc1("TestService", "TestInstance", "TestEvent", {1, 2, 3, 4});
    ServiceDescription desc2 = desc1;
    ServiceDescription desc3("TestService", "TestInstance", "TestEvent", {5, 6, 7, 8});
    EXPECT_TRUE(desc1.getClassHash()==desc2.getClassHash());
    EXPECT_FALSE(desc1.getClassHash()==desc3.getClassHash());
}

TEST_F(ServiceDescription_test, NotEqualToOperatorClassHash)
{
    ServiceDescription desc1("TestService", "TestInstance", "TestEvent", {1, 2, 3, 4});
    ServiceDescription desc2 = desc1;
    EXPECT_FALSE(desc1.getClassHash()!=desc2.getClassHash());
}

TEST_F(ServiceDescription_test, LessThanOperatorServiceDescription)
{
    ServiceDescription desc1("TestService1", "TestInstance", "TestEvent");
    ServiceDescription desc2("TestService", "TestInstance", "TestEvent");
    ServiceDescription desc3("TestService", "TestInstance4", "TestEvent");
    ServiceDescription desc4("TestService", "TestInstance", "TestEvent");
    ServiceDescription desc5("TestService", "TestInstance", "TestEvent6");
    ServiceDescription desc6("TestService", "TestInstance", "TestEvent");
    EXPECT_TRUE(desc1<desc2);
    EXPECT_TRUE(desc3<desc4);
    EXPECT_TRUE(desc5<desc6);

}

TEST_F(ServiceDescription_test, ServiceMatchServiceDescription)
{
    ServiceDescription desc1 = ServiceDescription(testAnyServiceID, 2, 3);
    ServiceDescription desc2 = ServiceDescription(testAnyServiceID, 9, 3);
    EXPECT_TRUE(iox::capro::serviceMatch(desc1,desc2));
}

TEST_F(ServiceDescription_test, IsValidServiceDescription)
{
    auto  serialObj1 = iox::cxx::Serialization::create(iox::capro::InvalidIDString,
                                                     iox::capro::InvalidIDString,
                                                     iox::capro::InvalidIDString,
                                                     iox::capro::AnyService,
                                                     iox::capro::AnyInstance,
                                                     iox::capro::AnyEvent,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     true,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd1 = ServiceDescription(serialObj1);
    EXPECT_FALSE(csd1.isValid());
    auto  serialObj2 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     "test3",
                                                     iox::capro::AnyService,
                                                     2,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     true,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd2 = ServiceDescription(serialObj2);
    EXPECT_FALSE(csd2.isValid());
    auto  serialObj3 = iox::cxx::Serialization::create("test1",
                                                     iox::capro::InvalidIDString,
                                                     "test3",
                                                     1,
                                                     2,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     true,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd3 = ServiceDescription(serialObj3);
    EXPECT_FALSE(csd3.isValid());
    auto  serialObj4 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     "test3",
                                                     1,
                                                     iox::capro::AnyInstance,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     true,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd4 = ServiceDescription(serialObj4);
    EXPECT_FALSE(csd4.isValid());
    auto  serialObj5 = iox::cxx::Serialization::create(iox::capro::InvalidIDString,
                                                     iox::capro::InvalidIDString,
                                                     iox::capro::InvalidIDString,
                                                     iox::capro::AnyService,
                                                     iox::capro::AnyInstance,
                                                     iox::capro::AnyEvent,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd5 = ServiceDescription(serialObj5);
    EXPECT_FALSE(csd5.isValid());

    auto  serialObj6 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     "test3",
                                                     iox::capro::AnyService,
                                                     2,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd6 = ServiceDescription(serialObj6);
    EXPECT_FALSE(csd6.isValid());

    auto  serialObj7 = iox::cxx::Serialization::create("test1",
                                                     iox::capro::InvalidIDString,
                                                     "test3",
                                                     1,
                                                     2,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd7 = ServiceDescription(serialObj7);
    EXPECT_FALSE(csd7.isValid());
    auto  serialObj8 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     "test3",
                                                     1,
                                                     iox::capro::AnyInstance,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd8 = ServiceDescription(serialObj8);
    EXPECT_FALSE(csd8.isValid());
    auto  serialObj9 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     iox::capro::InvalidIDString,
                                                     1,
                                                     2,
                                                     3,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd9 = ServiceDescription(serialObj8);
    EXPECT_FALSE(csd9.isValid());

    auto  serialObj10 = iox::cxx::Serialization::create("test1",
                                                     "test2",
                                                     "test3",
                                                     1,
                                                     2,
                                                     iox::capro::AnyInstance,
                                                     1,
                                                     2,
                                                     3,
                                                     4,
                                                     false,
                                                     static_cast<std::underlying_type<Scope>::type>(Scope::INVALID),
                                                     10);
    ServiceDescription csd10 = ServiceDescription(serialObj9);
    EXPECT_FALSE(csd10.isValid());
}

