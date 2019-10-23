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
#include "mocks/time_mock.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"

using namespace ::testing;

using iox::runtime::MqMessage;

class MqMessage_test : public Test
{
  protected:
    void SetUp()
    {
        time_MOCK::doUseMock = true;
        time_MOCK::mock.reset(new NiceMock<time_MOCK>());
        internal::CaptureStdout();
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
        time_MOCK::mock.reset();
        time_MOCK::doUseMock = false;
    }
};

TEST_F(MqMessage_test, DefaultCTor)
{
    MqMessage message;
    EXPECT_THAT(message.getNumberOfElements(), Eq(0u));
    EXPECT_THAT(message.getMessage(), Eq(""));
    EXPECT_THAT(message.isValid(), Eq(true));
    EXPECT_THAT(message.getElementAtIndex(2), Eq(""));
}

TEST_F(MqMessage_test, CTorWithInitializerList_validEntries)
{
    MqMessage message1({"abc", "def", "123123", ")(!*@&#^$)", "ABASDASD"});
    EXPECT_THAT(message1.getNumberOfElements(), Eq(5u));
    EXPECT_THAT(message1.getMessage(), Eq("abc,def,123123,)(!*@&#^$),ABASDASD,"));
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("abc"));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("def"));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("123123"));
    EXPECT_THAT(message1.getElementAtIndex(3), Eq(")(!*@&#^$)"));
    EXPECT_THAT(message1.getElementAtIndex(4), Eq("ABASDASD"));

    MqMessage message2({});
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));

    MqMessage message3({"", "", ""});
    EXPECT_THAT(message3.isValid(), Eq(true));
    EXPECT_THAT(message3.getNumberOfElements(), Eq(3u));
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_THAT(message3.getElementAtIndex(i), Eq(""));
    }
    EXPECT_THAT(message3.getMessage(), Eq(",,,"));

    MqMessage message4({"", "", "a", ""});
    EXPECT_THAT(message4.isValid(), Eq(true));
    EXPECT_THAT(message4.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message4.getElementAtIndex(2), Eq("a"));
    EXPECT_THAT(message4.getMessage(), Eq(",,a,,"));
}

TEST_F(MqMessage_test, CTorWithInitializerList_invalidEntries)
{
    MqMessage message1({"abc", "def", "123i,123", ")(!*@&#^$)", "ABASDASD"});
    EXPECT_THAT(message1.isValid(), Eq(false));

    MqMessage message2({"abc", "def", "123i123", ")(!*@&,#^$)", "ABASDASD"});
    EXPECT_THAT(message1.isValid(), Eq(false));

    MqMessage message3({",,,"});
    EXPECT_THAT(message3.isValid(), Eq(false));
}

TEST_F(MqMessage_test, CTorWithString_validMessage)
{
    MqMessage message1("asd,asd,asd,asd,");
    EXPECT_THAT(message1.getNumberOfElements(), Eq(4u));
    EXPECT_THAT(message1.isValid(), Eq(true));

    MqMessage message2("");
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));
    EXPECT_THAT(message2.isValid(), Eq(true));

    MqMessage message3("a,");
    EXPECT_THAT(message3.getNumberOfElements(), Eq(1u));
    EXPECT_THAT(message3.isValid(), Eq(true));
}

TEST_F(MqMessage_test, CTorWithString_invalidMessage)
{
    MqMessage message1("asd,asd,asd,asd");
    EXPECT_THAT(message1.isValid(), Eq(false));

    MqMessage message2(",a");
    EXPECT_THAT(message2.isValid(), Eq(false));

    MqMessage message3("a,ia");
    EXPECT_THAT(message3.isValid(), Eq(false));
}

TEST_F(MqMessage_test, CopyCTorValidEntries)
{
    MqMessage* source = new MqMessage({"fuu", "bar", "bla"});
    MqMessage destination(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(MqMessage_test, CopyCTorInvalidEntries)
{
    MqMessage* source = new MqMessage({"f,uu", "bar", "bla"});
    MqMessage destination(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(MqMessage_test, MoveCTorValidEntries)
{
    MqMessage* source = new MqMessage({"fuu", "bar", "bla"});
    MqMessage destination(std::move(*source));
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(MqMessage_test, MoveCTorInvalidEntries)
{
    MqMessage* source = new MqMessage({"f,uu", "bar", "bla"});
    MqMessage destination(std::move(*source));
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(MqMessage_test, CopyOperatorValidEntries)
{
    MqMessage* source = new MqMessage({"fuu", "bar", "bla"});
    MqMessage destination;
    destination = *source;
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(MqMessage_test, CopyOperatorInvalidEntries)
{
    MqMessage* source = new MqMessage({"f,uu", "bar", "bla"});
    MqMessage destination;
    destination = *source;
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(MqMessage_test, MoveOperatorValidEntries)
{
    MqMessage* source = new MqMessage({"fuu", "bar", "bla"});
    MqMessage destination;
    destination = std::move(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(true));
    EXPECT_THAT(destination.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(destination.getMessage(), Eq("fuu,bar,bla,"));
    EXPECT_THAT(destination.getElementAtIndex(0), Eq("fuu"));
    EXPECT_THAT(destination.getElementAtIndex(1), Eq("bar"));
    EXPECT_THAT(destination.getElementAtIndex(2), Eq("bla"));
}

TEST_F(MqMessage_test, MoveOperatorInvalidEntries)
{
    MqMessage* source = new MqMessage({"f,uu", "bar", "bla"});
    MqMessage destination;
    destination = std::move(*source);
    delete source;

    EXPECT_THAT(destination.isValid(), Eq(false));
}

TEST_F(MqMessage_test, getElementAtIndex)
{
    MqMessage message1({"fuu", "bar", "bla"});

    EXPECT_THAT(message1.getElementAtIndex(1), Eq("bar"));
    message1.addEntry(123.123f);
    EXPECT_THAT(message1.getElementAtIndex(3), Eq("123.123"));
    message1.addEntry("asd");
    message1.addEntry("asd");
    message1.addEntry("asd");
    EXPECT_THAT(message1.getElementAtIndex(5), Eq("asd"));

    MqMessage message2({});
    EXPECT_THAT(message2.getElementAtIndex(0), Eq(""));
    message2.addEntry(11U);
    message2.addEntry(12U);
    EXPECT_THAT(message2.getElementAtIndex(0), Eq("11"));
    message2.addEntry(13U);
    message2.addEntry(14U);
    message2.addEntry(15U);
    EXPECT_THAT(message2.getElementAtIndex(2), Eq("13"));
}

TEST_F(MqMessage_test, isValidEntry)
{
    MqMessage message;

    EXPECT_THAT(message.isValidEntry(""), Eq(true));
    EXPECT_THAT(message.isValidEntry("asdasd"), Eq(true));
    EXPECT_THAT(message.isValidEntry("10923"), Eq(true));
    EXPECT_THAT(message.isValidEntry("~!@#$%^\\&&*()_+_|}{][''\"]}"), Eq(true));

    EXPECT_THAT(message.isValidEntry(","), Eq(false));
    EXPECT_THAT(message.isValidEntry("asdasd,"), Eq(false));
    EXPECT_THAT(message.isValidEntry(",asdasdasd"), Eq(false));
    EXPECT_THAT(message.isValidEntry("i91283,asdasdasd"), Eq(false));
}

TEST_F(MqMessage_test, IsValidWithCTorConstruction)
{
    MqMessage message1;
    EXPECT_THAT(message1.isValid(), Eq(true));

    MqMessage message2({"asdasd"});
    EXPECT_THAT(message2.isValid(), Eq(true));

    MqMessage message3({"123123"});
    EXPECT_THAT(message3.isValid(), Eq(true));

    MqMessage message4({"~!@#$%^\\&&*()_+_|}{][''\"]}"});
    EXPECT_THAT(message4.isValid(), Eq(true));

    MqMessage message5({","});
    EXPECT_THAT(message5.isValid(), Eq(false));

    MqMessage message6({"asdasdasd,"});
    EXPECT_THAT(message6.isValid(), Eq(false));

    MqMessage message7({",asdasss"});
    EXPECT_THAT(message7.isValid(), Eq(false));

    MqMessage message8({"a8w9ej1,089sau;'1'"});
    EXPECT_THAT(message8.isValid(), Eq(false));
}

TEST_F(MqMessage_test, IsValidWithAddEntry)
{
    MqMessage message1;
    EXPECT_THAT(message1.isValid(), Eq(true));

    MqMessage message2;
    message2.addEntry("asdasd");
    EXPECT_THAT(message2.isValid(), Eq(true));

    MqMessage message3;
    message3.addEntry("123123");
    EXPECT_THAT(message3.isValid(), Eq(true));

    MqMessage message4;
    message4.addEntry("~!@#$%^\\&&*()_+_|}{][''\"]}");
    EXPECT_THAT(message4.isValid(), Eq(true));

    MqMessage message5;
    message5.addEntry(",");
    EXPECT_THAT(message5.isValid(), Eq(false));

    MqMessage message6;
    message6.addEntry("asdasdasd,");
    EXPECT_THAT(message6.isValid(), Eq(false));

    MqMessage message7;
    message7.addEntry(",asdasss");
    EXPECT_THAT(message7.isValid(), Eq(false));

    MqMessage message8;
    message8.addEntry("a8w9ej1,089sau;'1'");
    EXPECT_THAT(message8.isValid(), Eq(false));
}

TEST_F(MqMessage_test, getMessage)
{
    MqMessage message1;
    EXPECT_THAT(message1.getMessage(), Eq(""));
    message1.addEntry(123);
    EXPECT_THAT(message1.getMessage(), Eq("123,"));
    message1.addEntry("asd");
    EXPECT_THAT(message1.getMessage(), Eq("123,asd,"));
    message1.addEntry("&*!_)(@)");
    EXPECT_THAT(message1.getMessage(), Eq("123,asd,&*!_)(@),"));

    MqMessage message2({"f812", "92-3kjd", "\"'s02'"});
    EXPECT_THAT(message2.getMessage(), Eq("f812,92-3kjd,\"'s02',"));
}

TEST_F(MqMessage_test, AddEntryWithValidEntries)
{
    MqMessage message1;

    message1.addEntry("aaaa");
    EXPECT_THAT(message1.getNumberOfElements(), Eq(1u));
    EXPECT_THAT(message1.getElementAtIndex(0), Eq("aaaa"));

    message1.addEntry(123.123f);
    EXPECT_THAT(message1.getNumberOfElements(), Eq(2u));
    EXPECT_THAT(message1.getElementAtIndex(1), Eq("123.123"));

    message1.addEntry('x');
    EXPECT_THAT(message1.getNumberOfElements(), Eq(3u));
    EXPECT_THAT(message1.getElementAtIndex(2), Eq("x"));

    MqMessage message2({"fuu", "bar"});
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

TEST_F(MqMessage_test, AddEntryWithInvalidEntries)
{
    MqMessage message1;

    EXPECT_THAT(message1.isValid(), Eq(true));
    message1.addEntry("aa,aa");
    EXPECT_THAT(message1.isValid(), Eq(false));
    message1.addEntry("aaa");
    EXPECT_THAT(message1.isValid(), Eq(false));

    MqMessage message2({"asd", "913u"});

    EXPECT_THAT(message2.isValid(), Eq(true));
    message2.addEntry("aaa");
    EXPECT_THAT(message2.isValid(), Eq(true));
    message2.addEntry(",aa");
    EXPECT_THAT(message2.isValid(), Eq(false));
}

TEST_F(MqMessage_test, clearMessage)
{
    MqMessage message1;

    message1.clearMessage();
    EXPECT_THAT(message1.isValid(), Eq(true));
    EXPECT_THAT(message1.getMessage(), Eq(""));
    EXPECT_THAT(message1.getNumberOfElements(), Eq(0u));

    MqMessage message2({"a", "asd", "asd", "aaaaa"});
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getMessage(), Eq("a,asd,asd,aaaaa,"));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(4u));
    message2.clearMessage();
    EXPECT_THAT(message2.isValid(), Eq(true));
    EXPECT_THAT(message2.getMessage(), Eq(""));
    EXPECT_THAT(message2.getNumberOfElements(), Eq(0u));

    MqMessage message3({",,,a", "asd", "asd", "aaaaa"});
    EXPECT_THAT(message3.isValid(), Eq(false));
    message3.clearMessage();
    EXPECT_THAT(message3.isValid(), Eq(true));
    EXPECT_THAT(message3.getMessage(), Eq(""));
    EXPECT_THAT(message3.getNumberOfElements(), Eq(0u));
}

TEST_F(MqMessage_test, setMessage)
{
    MqMessage message1;

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
