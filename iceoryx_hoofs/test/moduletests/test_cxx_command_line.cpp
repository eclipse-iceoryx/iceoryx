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

#include "iceoryx_hoofs/cxx/command_line.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "test.hpp"
#include "test_cxx_command_line_common.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::cxx;

/// This test is only some kind of compilation test to verify that the
/// command line parser macros are working and connecting everything together correctly
/// The actual test of the command line parser can be found in test_cxx_command_line_parser.cpp
class CommandLine_test : public Test
{
  public:
    void SetUp() override
    {
        // if we do not capture stdout then the console is filled with garbage
        // since the command line parser prints the help on failure
        ::testing::internal::CaptureStdout();
    }
    void TearDown() override
    {
        std::string output = ::testing::internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

struct CommandLineSut
{
    COMMAND_LINE(CommandLineSut, "My program description");

    OPTIONAL_VALUE(string<100>, stringValue1, "default value", 's', "string-value-1", "some description");
    OPTIONAL_VALUE(string<100>, stringValue2, "some other value", 't', "string-value-2", "some description");
    OPTIONAL_VALUE(int64_t, optionalInt1, 123, 'i', "int-value-1", "some description");
    OPTIONAL_VALUE(int64_t, optionalInt2, 456, 'j', "int-value-2", "some description");
    OPTIONAL_VALUE(uint8_t, optionalUint1, 123, 'u', "uint-value-1", "some description");
    OPTIONAL_VALUE(uint8_t, optionalUint2, 212, 'v', "uint-value-2", "some description");

    SWITCH(lightSwitch1, 'l', "light-switch-1", "do some stuff - some description");
    SWITCH(lightSwitch2, 'm', "light-switch-2", "do some stuff - some description");

    REQUIRED_VALUE(string<100>, requiredString, 'r', "required-string", "some description");
    REQUIRED_VALUE(float, requiredFloat, 'b', "required-float", "some description");
    REQUIRED_VALUE(uint16_t, requiredUint, 'c', "required-uint", "some description");
};

TEST_F(CommandLine_test, OnlyRequiredValuesSetsRemainingValuesToDefault)
{
    CmdArgs args(
        {"myBinaryName", "--required-string", "bluubb", "--required-float", "123.456", "--required-uint", "12"});
    CommandLineSut sut(args.argc, args.argv);

    EXPECT_THAT(sut.binaryName().c_str(), StrEq("myBinaryName"));

    // default values
    EXPECT_THAT(sut.stringValue1().c_str(), StrEq("default value"));
    EXPECT_THAT(sut.stringValue2().c_str(), StrEq("some other value"));
    EXPECT_THAT(sut.optionalInt1(), Eq(123));
    EXPECT_THAT(sut.optionalInt2(), Eq(456));
    EXPECT_THAT(sut.optionalUint1(), Eq(123));
    EXPECT_THAT(sut.optionalUint2(), Eq(212));
    EXPECT_THAT(sut.lightSwitch1(), Eq(false));
    EXPECT_THAT(sut.lightSwitch2(), Eq(false));

    EXPECT_THAT(sut.requiredString().c_str(), StrEq("bluubb"));
    EXPECT_THAT(sut.requiredFloat(), Eq(123.456F));
    EXPECT_THAT(sut.requiredUint(), Eq(12));
}

TEST_F(CommandLine_test, AllValuesViaCommandLineAreSetCorrectly)
{
    CmdArgs args({"anotherOneBitesTheDust",
                  "--required-string",
                  "schnappidububa",
                  "--required-float",
                  "456.123",
                  "--required-uint",
                  "1212"});
    CommandLineSut sut(args.argc, args.argv);

    EXPECT_THAT(sut.binaryName().c_str(), StrEq("anotherOneBitesTheDust"));

    EXPECT_THAT(sut.stringValue1().c_str(), StrEq("default value"));
    EXPECT_THAT(sut.stringValue2().c_str(), StrEq("some other value"));
    EXPECT_THAT(sut.optionalInt1(), Eq(123));
    EXPECT_THAT(sut.optionalInt2(), Eq(456));
    EXPECT_THAT(sut.optionalUint1(), Eq(123));
    EXPECT_THAT(sut.optionalUint2(), Eq(212));
    EXPECT_THAT(sut.lightSwitch1(), Eq(false));
    EXPECT_THAT(sut.lightSwitch2(), Eq(false));

    EXPECT_THAT(sut.requiredString().c_str(), StrEq("schnappidububa"));
    EXPECT_THAT(sut.requiredFloat(), Eq(456.123F));
    EXPECT_THAT(sut.requiredUint(), Eq(1212));
}


} // namespace
