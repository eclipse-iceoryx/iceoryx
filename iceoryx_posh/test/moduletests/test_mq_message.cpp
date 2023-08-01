// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#if !defined(_WIN32) && !defined(__APPLE__)
#include "iceoryx_hoofs/testing/mocks/time_mock.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using iox::runtime::IpcMessage;

class IpcMessage_test : public Test
{
  protected:
    void SetUp() override
    {
        time_MOCK::doUseMock = true;
        time_MOCK::mock.reset(new NiceMock<time_MOCK>());
    }

    void TearDown() override
    {
        time_MOCK::mock.reset();
        time_MOCK::doUseMock = false;
    }
};

TEST_F(IpcMessage_test, DefaultCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f45cce1-5d93-49eb-8344-fa318a46e035");
    IpcMessage message;
    EXPECT_THAT(message.getNumberOfElements(), Eq(0u));
    EXPECT_THAT(message.getMessage(), Eq(""));
    EXPECT_THAT(message.isValid(), Eq(true));
    EXPECT_THAT(message.getElementAtIndex(2), Eq(""));
}

TEST_F(IpcMessage_test, CTorWithInitializerList_validEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "a21483b7-be79-400d-beeb-571c017db42c");
    IpcMessage message1({"abc", "def", "123123", ")(!*@&#^$)", "ABASDASD"});
    EXPECT_THAT(message1.getNumberOfElements(), Eq(5u));
    EXPECT_THAT(message1.getMessage(), Eq("abc,def,123123,)(!*@&#^$),ABASDASD,"));
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("abc"));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("def"));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("123123"));
    EXPECT_THAT(message1.getElementAtIndex(3), Eq(")(!*@&#^$)"));
    EXPECT_THAT(message1.getElementAtIndex(4), Eq("ABASDASD"));

    IpcMessage message2({});
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));

    IpcMessage message3({"", "", ""});
    EXPECT_THAT(message3.isValid(), Eq(true));
    EXPECT_THAT(message3.getNumberOfElements(), Eq(3u));
    for (uint16_t i = 0; i < 3; ++i)
    {
        EXPECT_THAT(message3.getElementAtIndex(i), Eq(""));
    }
    EXPECT_THAT(message3.getMessage(), Eq(",,,"));

    IpcMessage message4({"", "", "a", ""});
    EXPECT_THAT(message4.isValid(), Eq(true));
    EXPECT_THAT(message4.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message4.getElementAtIndex(2), Eq("a"));
    EXPECT_THAT(message4.getMessage(), Eq(",,a,,"));
}

TEST_F(IpcMessage_test, CTorWithInitializerList_invalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0f96a86-d579-4959-8633-bb30ab8d654f");
    IpcMessage message1({"abc", "def", "123i,123", ")(!*@&#^$)", "ABASDASD"});
    EXPECT_THAT(message1.isValid(), Eq(false));

    IpcMessage message2({"abc", "def", "123i123", ")(!*@&,#^$)", "ABASDASD"});
    EXPECT_THAT(message1.isValid(), Eq(false));

    IpcMessage message3({",,,"});
    EXPECT_THAT(message3.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, CTorWithString_validMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb3418b-1d96-4d15-b5e3-36ef1097c1a7");
    IpcMessage message1("asd,asd,asd,asd,");
    EXPECT_THAT(message1.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message1.isValid(), Eq(true));

    IpcMessage message2("");
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));
    EXPECT_THAT(message2.isValid(), Eq(true));

    IpcMessage message3("a,");
    EXPECT_THAT(message3.getNumberOfElements(), Eq(1u));
    EXPECT_THAT(message3.isValid(), Eq(true));
}

TEST_F(IpcMessage_test, CTorWithString_invalidMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "9af0d7a6-f4b4-4be6-8a46-654992cb021a");
    IpcMessage message1("asd,asd,asd,asd");
    EXPECT_THAT(message1.isValid(), Eq(false));

    IpcMessage message2(",a");
    EXPECT_THAT(message2.isValid(), Eq(false));

    IpcMessage message3("a,ia");
    EXPECT_THAT(message3.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, CopyCTorValidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a44de96-0441-42e5-b3a8-90dfcc3acce6");
    IpcMessage* source = new IpcMessage({"fuu", "bar", "bla"});
    IpcMessage destination(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(IpcMessage_test, CopyCTorInvalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a5011bd-be96-4619-9384-563c0a5f4d11");
    IpcMessage* source = new IpcMessage({"f,uu", "bar", "bla"});
    IpcMessage destination(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, MoveCTorValidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "9980d693-eb2e-441f-a707-18a5901da020");
    IpcMessage* source = new IpcMessage({"fuu", "bar", "bla"});
    IpcMessage destination(std::move(*source));
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(IpcMessage_test, MoveCTorInvalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "65e0ac6d-ec3c-4eae-8f69-45784fc52016");
    IpcMessage* source = new IpcMessage({"f,uu", "bar", "bla"});
    IpcMessage destination(std::move(*source));
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, CopyOperatorValidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "702fcd55-447b-4d20-8137-18931d002d6b");
    IpcMessage* source = new IpcMessage({"fuu", "bar", "bla"});
    IpcMessage destination;
    destination = *source;
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(IpcMessage_test, CopyOperatorInvalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "133c5c9e-0880-4be4-b660-acb14fc445fe");
    IpcMessage* source = new IpcMessage({"f,uu", "bar", "bla"});
    IpcMessage destination;
    destination = *source;
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, MoveOperatorValidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "05b92b6a-2ac8-491b-a7b8-09b1c132eea6");
    IpcMessage* source = new IpcMessage({"fuu", "bar", "bla"});
    IpcMessage destination;
    destination = std::move(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(IpcMessage_test, MoveOperatorInvalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "84a2f506-66b2-46d5-a154-45d375dc37c0");
    IpcMessage* source = new IpcMessage({"f,uu", "bar", "bla"});
    IpcMessage destination;
    destination = std::move(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, getElementAtIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "554163c3-2119-462e-90f7-f3bd01421fad");
    IpcMessage message1({"fuu", "bar", "bla"});

    EXPECT_THAT(message1.getElementAtIndex(1), Eq("bar"));
    message1.addEntry(123.123f);
    EXPECT_THAT(message1.getElementAtIndex(3), Eq("123.123"));
    message1.addEntry("asd");
    message1.addEntry("asd");
    message1.addEntry("asd");
    EXPECT_THAT(message1.getElementAtIndex(5), Eq("asd"));

    IpcMessage message2({});
    EXPECT_THAT(message2.getElementAtIndex(0), Eq(""));
    message2.addEntry(11U);
    message2.addEntry(12U);
    EXPECT_THAT(message2.getElementAtIndex(0), Eq("11"));
    message2.addEntry(13U);
    message2.addEntry(14U);
    message2.addEntry(15U);
    EXPECT_THAT(message2.getElementAtIndex(2), Eq("13"));
}

TEST_F(IpcMessage_test, isValidEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b09e0ca-66b4-40ca-9449-2ae9e79eb48e");
    IpcMessage message;

    EXPECT_THAT(message.isValidEntry(""), Eq(true));
    EXPECT_THAT(message.isValidEntry("asdasd"), Eq(true));
    EXPECT_THAT(message.isValidEntry("10923"), Eq(true));
    EXPECT_THAT(message.isValidEntry("~!@#$%^\\&&*()_+_|}{][''\"]}"), Eq(true));

    EXPECT_THAT(message.isValidEntry(","), Eq(false));
    EXPECT_THAT(message.isValidEntry("asdasd,"), Eq(false));
    EXPECT_THAT(message.isValidEntry(",asdasdasd"), Eq(false));
    EXPECT_THAT(message.isValidEntry("i91283,asdasdasd"), Eq(false));
}

TEST_F(IpcMessage_test, IsValidWithCTorConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "57aed4a1-2817-4b33-a8b4-ba75891f29a2");
    IpcMessage message1;
    EXPECT_THAT(message1.isValid(), Eq(true));

    IpcMessage message2({"asdasd"});
    EXPECT_THAT(message2.isValid(), Eq(true));

    IpcMessage message3({"123123"});
    EXPECT_THAT(message3.isValid(), Eq(true));

    IpcMessage message4({"~!@#$%^\\&&*()_+_|}{][''\"]}"});
    EXPECT_THAT(message4.isValid(), Eq(true));

    IpcMessage message5({","});
    EXPECT_THAT(message5.isValid(), Eq(false));

    IpcMessage message6({"asdasdasd,"});
    EXPECT_THAT(message6.isValid(), Eq(false));

    IpcMessage message7({",asdasss"});
    EXPECT_THAT(message7.isValid(), Eq(false));

    IpcMessage message8({"a8w9ej1,089sau;'1'"});
    EXPECT_THAT(message8.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, IsValidWithAddEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "c29c21a5-0ec9-4016-9fca-d4804b3562bd");
    IpcMessage message1;
    EXPECT_THAT(message1.isValid(), Eq(true));

    IpcMessage message2;
    message2.addEntry("asdasd");
    EXPECT_THAT(message2.isValid(), Eq(true));

    IpcMessage message3;
    message3.addEntry("123123");
    EXPECT_THAT(message3.isValid(), Eq(true));

    IpcMessage message4;
    message4.addEntry("~!@#$%^\\&&*()_+_|}{][''\"]}");
    EXPECT_THAT(message4.isValid(), Eq(true));

    IpcMessage message5;
    message5.addEntry(",");
    EXPECT_THAT(message5.isValid(), Eq(false));

    IpcMessage message6;
    message6.addEntry("asdasdasd,");
    EXPECT_THAT(message6.isValid(), Eq(false));

    IpcMessage message7;
    message7.addEntry(",asdasss");
    EXPECT_THAT(message7.isValid(), Eq(false));

    IpcMessage message8;
    message8.addEntry("a8w9ej1,089sau;'1'");
    EXPECT_THAT(message8.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, getMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "947eef64-112b-480c-b216-57e0c9b5688f");
    IpcMessage message1;
    EXPECT_THAT(message1.getMessage(), Eq(""));
    message1.addEntry(123);
    EXPECT_THAT(message1.getMessage(), Eq("123,"));
    message1.addEntry("asd");
    EXPECT_THAT(message1.getMessage(), Eq("123,asd,"));
    message1.addEntry("&*!_)(@)");
    EXPECT_THAT(message1.getMessage(), Eq("123,asd,&*!_)(@),"));

    IpcMessage message2({"f812", "92-3kjd", "\"'s02'"});
    EXPECT_THAT(message2.getMessage(), Eq("f812,92-3kjd,\"'s02',"));
}

TEST_F(IpcMessage_test, AddEntryWithValidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "370ab026-7f4c-4bdb-8a36-0a60284aaea4");
    IpcMessage message1;

    message1.addEntry("aaaa");
    EXPECT_THAT(message1.getNumberOfElements(), Eq(1u));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("aaaa"));

    message1.addEntry(123.123f);
    EXPECT_THAT(message1.getNumberOfElements(), Eq(2u));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("123.123"));

    message1.addEntry('x');
    EXPECT_THAT(message1.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("x"));

    IpcMessage message2({"fuu", "bar"});
    message2.addEntry("aaaa");
    EXPECT_THAT(message2.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(message2.getElementAtIndex(2), Eq("aaaa"));

    message2.addEntry(123.123f);
    EXPECT_THAT(message2.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message2.getElementAtIndex(3), Eq("123.123"));

    message2.addEntry('x');
    EXPECT_THAT(message2.getNumberOfElements(), Eq(5u));
    EXPECT_THAT(message2.getElementAtIndex(4), Eq("x"));
}

TEST_F(IpcMessage_test, AddEntryWithInvalidEntries)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e44a35a-f046-4c13-93d5-3ab5ae71931f");
    IpcMessage message1;

    EXPECT_THAT(message1.isValid(), Eq(true));
    message1.addEntry("aa,aa");
    EXPECT_THAT(message1.isValid(), Eq(false));
    message1.addEntry("aaa");
    EXPECT_THAT(message1.isValid(), Eq(false));

    IpcMessage message2({"asd", "913u"});

    EXPECT_THAT(message2.isValid(), Eq(true));
    message2.addEntry("aaa");
    EXPECT_THAT(message2.isValid(), Eq(true));
    message2.addEntry(",aa");
    EXPECT_THAT(message2.isValid(), Eq(false));
}

TEST_F(IpcMessage_test, clearMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "96b6d5a7-797d-4d9b-9334-290d2254dec4");
    IpcMessage message1;

    message1.clearMessage();
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getMessage(), Eq(""));
    EXPECT_THAT(message1.getNumberOfElements(), Eq(0u));

    IpcMessage message2({"a", "asd", "asd", "aaaaa"});
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getMessage(), Eq("a,asd,asd,aaaaa,"));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(4u));
    message2.clearMessage();
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getMessage(), Eq(""));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));

    IpcMessage message3({",,,a", "asd", "asd", "aaaaa"});
    EXPECT_THAT(message3.isValid(), Eq(false));
    message3.clearMessage();
    EXPECT_THAT(message3.isValid(), Eq(true));
    EXPECT_THAT(message3.getMessage(), Eq(""));
    EXPECT_THAT(message3.getNumberOfElements(), Eq(0u));
}

TEST_F(IpcMessage_test, setMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "5affc711-1af9-4906-ac7d-ddf408ee61c7");
    IpcMessage message1;

    message1.setMessage("asd1,asd2,asd3,asd4,");
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("asd1"));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("asd2"));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("asd3"));
    EXPECT_THAT(message1.getElementAtIndex(3), Eq("asd4"));

    message1.setMessage("1,2,3,");
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("1"));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("2"));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("3"));

    message1.setMessage("1,2,3,4");
    EXPECT_THAT(message1.isValid(), Eq(false));
}

} // namespace
#endif
