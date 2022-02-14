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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/internal/cxx/command_line_parser.hpp"
#include "test.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::cxx;

class CommandLineParser_test : public Test
{
  public:
    void SetUp() override
    {
        // ::testing::internal::CaptureStdout();
    }
    void TearDown() override
    {
        // std::string output = ::testing::internal::GetCapturedStdout();
        // if (Test::HasFailure())
        // {
        //     std::cout << output << std::endl;
        // }
    }

    using str_t = char[CommandLineParser::MAX_DESCRIPTION_LENGTH];
    static CommandLineOptions::value_t defaultValue;
};
CommandLineOptions::value_t CommandLineParser_test::defaultValue = "DEFAULT VALUE";

// TEST TODO:
// conversion failure
// struct macro builder

struct CmdArgs
{
    int argc = 0;
    char** argv = nullptr;

    explicit CmdArgs(const std::vector<std::string>& arguments)
        : argc{static_cast<int>(arguments.size())}
        , argv{new char*[argc]}
    {
        contents = std::make_unique<std::vector<std::string>>(arguments);
        for (int i = 0; i < argc; ++i)
        {
            argv[i] = const_cast<char*>((*contents)[i].data());
        }
    }

    ~CmdArgs()
    {
        delete[] argv;
    }

    std::unique_ptr<std::vector<std::string>> contents;
};

TEST_F(CommandLineParser_test, SettingBinaryNameWorks)
{
    const CommandLineOptions::binaryName_t binaryName("AllHailHypnotoad");
    CmdArgs args({binaryName.c_str()});
    auto options = CommandLineParser("").parse(args.argc, args.argv);

    EXPECT_THAT(options.binaryName(), Eq(binaryName));
}

TEST_F(CommandLineParser_test, EmptyArgcLeadsToExit)
{
    bool wasErrorHandlerCalled = false;
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        IOX_DISCARD_RESULT(CommandLineParser("").parse(0, nullptr));
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, TooLargeBinaryNameLeadsToExit)
{
    CmdArgs args({std::string(CommandLineOptions::binaryName_t::capacity() + 1, 'a')});
    bool wasErrorHandlerCalled = false;
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        IOX_DISCARD_RESULT(CommandLineParser("").parse(args.argc, args.argv));
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, AddingTheSameShortOptionLeadsToExist)
{
    CmdArgs args({std::string(CommandLineOptions::binaryName_t::capacity() + 1, 'a')});
    bool wasErrorHandlerCalled = false;
    CommandLineParser parser("");
    parser.addOptionalValue('c', "firstEntry", "", "", "");

    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        parser.addOptionalValue('c', "duplicateShortOption", "", "", "");
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, AddingTheSameLongOptionLeadsToExist)
{
    bool wasErrorHandlerCalled = false;
    CommandLineParser parser("");
    parser.addOptionalValue('c', "duplicate", "", "", "");

    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        parser.addOptionalValue('x', "duplicate", "", "", "");
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, AddingOptionWithSameShortAndLongNameLeadsToExist)
{
    bool wasErrorHandlerCalled = false;
    CommandLineParser parser("");
    parser.addOptionalValue('d', "duplicate", "", "", "");

    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        parser.addOptionalValue('d', "duplicate", "", "", "");
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

void FailureTest(const std::vector<std::string>& options,
                 const std::vector<std::string>& optionsToRegister = {},
                 const std::vector<std::string>& switchesToRegister = {},
                 const std::vector<std::string>& requiredValuesToRegister = {},
                 const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE) noexcept
{
    const CommandLineOptions::binaryName_t binaryName("GloryToTheHasselToad");
    std::vector<std::string> optionVector{binaryName.c_str()};
    optionVector.insert(optionVector.end(), options.begin(), options.end());
    CmdArgs args(optionVector);

    bool wasErrorHandlerCalled = false;
    {
        CommandLineParser parser("");
        for (const auto& o : optionsToRegister)
        {
            parser.addOptionalValue(o[0], CommandLineOptions::name_t(TruncateToCapacity, o), "", "int", "0");
        }
        for (const auto& s : switchesToRegister)
        {
            parser.addSwitch(s[0], CommandLineOptions::name_t{TruncateToCapacity, s}, "");
        }
        for (const auto& r : requiredValuesToRegister)
        {
            parser.addRequiredValue(r[0], CommandLineOptions::name_t(TruncateToCapacity, r), "", "int");
        }

        {
            auto handle =
                iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
            IOX_DISCARD_RESULT(parser.parse(args.argc, args.argv, 1U, actionWhenOptionUnknown));
        }
    }

    switch (actionWhenOptionUnknown)
    {
    case UnknownOption::TERMINATE:
        EXPECT_TRUE(wasErrorHandlerCalled);
        break;
    case UnknownOption::IGNORE:
        EXPECT_FALSE(wasErrorHandlerCalled);
        break;
    }
}

/// BEGIN syntax failure test

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionDoesNotStartWithMinus_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus"};
    FailureTest({"i-have-no-minus"});
    FailureTest({"i-have-no-minus", "someValue"});
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionDoesNotStartWithMinus_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus", "set", "bla"};
    // begin
    FailureTest({"i-have-no-minus", "--set", "setValue", "--bla", "blaValue"}, optionsToRegister);
    FailureTest({"i-have-no-minus", "someValue", "--set", "setValue", "--bla", "blaValue"}, optionsToRegister);
    // middle
    FailureTest({"--set", "setValue", "i-have-no-minus", "--bla", "blaValue"}, optionsToRegister);
    FailureTest({"--set", "setValue", "i-have-no-minus", "someValue", "--bla", "blaValue"}, optionsToRegister);
    // end
    FailureTest({"--set", "setValue", "--bla", "blaValue", "i-have-no-minus"}, optionsToRegister);
    FailureTest({"--set", "setValue", "--bla", "blaValue", "i-have-no-minus", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionDoesNotStartWithMinus_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus", "set", "bla"};
    // begin
    FailureTest({"i", "-s", "setValue", "-b", "blaValue"}, optionsToRegister);
    FailureTest({"i", "someValue", "-s", "setValue", "-b", "blaValue"}, optionsToRegister);
    // middle
    FailureTest({"-s", "setValue", "i", "-b", "blaValue"}, optionsToRegister);
    FailureTest({"-s", "setValue", "i", "someValue", "-b", "blaValue"}, optionsToRegister);
    // end
    FailureTest({"-s", "setValue", "-b", "blaValue", "i"}, optionsToRegister);
    FailureTest({"-s", "setValue", "-b", "blaValue", "i", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenShortOptionNameIsEmpty_SingleArgument)
{
    FailureTest({"-"});
    FailureTest({"-", "someValue"});
}

TEST_F(CommandLineParser_test, FailSyntaxWhenShortOptionNameIsEmpty_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    FailureTest({"-", "--set", "setValue123", "--bla", "blaValue455"}, optionsToRegister);
    FailureTest({"-", "someValue", "--set", "setValue123", "--bla", "blaValue455"}, optionsToRegister);
    // middle
    FailureTest({"--set", "setValue123", "-", "--bla", "blaValue455"}, optionsToRegister);
    FailureTest({"--set", "setValue123", "-", "someValue", "--bla", "blaValue455"}, optionsToRegister);
    // end
    FailureTest({"--set", "setValue123", "--bla", "blaValue455", "-"}, optionsToRegister);
    FailureTest({"--set", "setValue123", "--bla", "blaValue455", "-", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionNameIsEmpty_SingleArgument)
{
    FailureTest({"--"});
    FailureTest({"--", "someValue"});
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionNameIsEmpty_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    FailureTest({"--", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"--", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    // middle
    FailureTest({"--bla", "blaValue123123", "--", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "--", "someValue", "--set", "setValueXXX"}, optionsToRegister);
    // end
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "--"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "--", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenShortOptionNameHasMoreThenOneLetter_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"invalid-option"};
    FailureTest({"-invalid-option"}, optionsToRegister);
    FailureTest({"-invalid-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenShortOptionNameHasMoreThenOneLetter_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "invalid-option"};
    // begin
    FailureTest({"-invalid-option", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"-invalid-option", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    // middle
    FailureTest({"--bla", "blaValue123123", "-invalid-option", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "-invalid-option", "someValue", "--set", "setValueXXX"}, optionsToRegister);
    // end
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "-invalid-option"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "-invalid-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenLongOptionStartsWithTripleMinus_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"invalid-long-option"};
    FailureTest({"---invalid-long-option"}, optionsToRegister);
    FailureTest({"---invalid-long-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenLongOptionStartsWithTripleMinus_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "invalid-long-option"};
    // begin
    FailureTest({"---invalid-long-option", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"---invalid-long-option", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"},
                optionsToRegister);
    // middle
    FailureTest({"--bla", "blaValue123123", "---invalid-long-option", "--set", "setValueXXX"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "---invalid-long-option", "someValue", "--set", "setValueXXX"},
                optionsToRegister);
    // end
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "---invalid-long-option"}, optionsToRegister);
    FailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "---invalid-long-option", "someValue"},
                optionsToRegister);
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionNameExceedMaximumSize_SingleArgument)
{
    FailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a')});
    FailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'), "someValue"});
}

TEST_F(CommandLineParser_test, FailSyntaxWhenOptionNameExceedMaximumSize_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    FailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                 "--set",
                 "setValue",
                 "--bla",
                 "blaValue"},
                optionsToRegister);
    FailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                 "someValue",
                 "--set",
                 "setValue",
                 "--bla",
                 "blaValue"},
                optionsToRegister);
    // middle
    FailureTest({"--set",
                 "setValue",
                 std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                 "--bla",
                 "blaValue"},
                optionsToRegister);
    FailureTest({"someValue",
                 "--set",
                 std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                 "setValue",
                 "--bla",
                 "blaValue"},
                optionsToRegister);
    // end
    FailureTest({"--set",
                 "setValue",
                 "--bla",
                 "blaValue",
                 std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a')},
                optionsToRegister);
    FailureTest({"--set",
                 "setValue",
                 "--bla",
                 "blaValue",
                 std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                 "someValue"},
                optionsToRegister);
}

/// END syntax failure test

/// BEGIN option failure test
TEST_F(CommandLineParser_test, FailWhenOptionWasNotRegistered_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"sputnik", "rosetta"};
    FailureTest({"--conway", "gameOfLife"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionWasNotRegistered_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"sputnik", "rosetta"};
    // begin
    FailureTest({"--conway", "gameOfLife", "--sputnik", "iWasFirst", "--rosetta", "uhWhatsThere"}, optionsToRegister);
    // middle
    FailureTest({"--sputnik", "iWasFirst", "--conway", "gameOfLife", "--rosetta", "uhWhatsThere"}, optionsToRegister);
    // end
    FailureTest({"--sputnik", "iWasFirst", "--rosetta", "uhWhatsThere", "--conway", "gameOfLife"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionWasNotRegistered_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"sputnik", "rosetta"};
    // begin
    FailureTest({"-c",
                 "gameOfLife",
                 "-s",
                 "iWasFirst"
                 "-r",
                 "uhWhatsThere"},
                optionsToRegister);
    // middle
    FailureTest({"-s", "gameOfLife", "-c", "gameOfLife", "-r", "uhWhatsThere"}, optionsToRegister);
    // end
    FailureTest({"-s", "gameOfLife", "-r", "uhWhatsThere", "-c", "gameOfLife"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set", "oh-no-i-am-an-option"};
    FailureTest({"--set", "--oh-no-i-am-an-option"}, optionsToRegister);
    FailureTest({"--set", "--oh-no-i-am-an-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu", "oh-no-i-am-an-option"};
    // begin
    FailureTest({"--set", "--oh-no-i-am-an-option", "--bla", "blaValue", "--fuu", "fuuValue"}, optionsToRegister);
    FailureTest({"--set", "--oh-no-i-am-an-option", "someValue", "--bla", "blaValue", "--fuu", "fuuValue"},
                optionsToRegister);
    // middle
    FailureTest({"--bla", "--set", "--oh-no-i-am-an-option", "--fuu", "fuuValue"}, optionsToRegister);
    FailureTest({"--bla", "blaValue", "--set", "--oh-no-i-am-an-option", "someValue", "--fuu", "fuuValue"},
                optionsToRegister);

    // end
    FailureTest({"--fuu", "--bla", "--set", "--oh-no-i-am-an-option"}, optionsToRegister);
    FailureTest({"--fuu", "fuuValue", "--bla", "blaValue", "--set", "--oh-no-i-am-an-option", "someValue"},
                optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu", "oh-no-i-am-an-option"};
    // begin
    FailureTest({"-s", "-o", "-b", "blaValue", "-f", "fuuValue"}, optionsToRegister);
    FailureTest({"-s", "-o", "someValue", "-b", "blaValue", "-f", "fuuValue"}, optionsToRegister);
    // middle
    FailureTest({"-b", "-s", "-o", "-f", "fuuValue"}, optionsToRegister);
    FailureTest({"-b", "blaValue", "-s", "-o", "someValue", "-f", "fuuValue"}, optionsToRegister);

    // end
    FailureTest({"-f", "-b", "-s", "-o"}, optionsToRegister);
    FailureTest({"-f", "fuuValue", "-b", "blaValue", "-s", "-o", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    FailureTest({"--set", "bla", "--set", "fuu"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};
    FailureTest({"--set", "fuuu", "--bla", "blaValue", "--fuu", "fuuValue", "--set", "bla"}, optionsToRegister);
    FailureTest({"--bla", "blaValue", "--set", "fuuu", "--fuu", "fuuValue", "--set", "bla"}, optionsToRegister);
    FailureTest({"--set", "fuuu", "--bla", "blaValue", "--set", "bla", "--fuu", "fuuValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};
    FailureTest({"-s", "fuuu", "-b", "blaValue", "-f", "fuuValue", "-s", "bla"}, optionsToRegister);
    FailureTest({"-b", "blaValue", "-s", "fuuu", "-f", "fuuValue", "-s", "bla"}, optionsToRegister);
    FailureTest({"-s", "fuuu", "-b", "blaValue", "-s", "bla", "-f", "fuuValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionValueExceedMaximumSize_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    FailureTest({"--set", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a')}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionValueExceedMaximumSize_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    FailureTest({"--set",
                 std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'),
                 "--bla",
                 "blaValue",
                 "--fuu",
                 "fuuValue"},
                optionsToRegister);
    // middle
    FailureTest({"--set",
                 "blaValue",
                 "--bla",
                 std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'),
                 "--fuu",
                 "fuuValue"},
                optionsToRegister);
    // end
    FailureTest({"--set",
                 "blaValue",
                 "--bla",
                 "fuuValue",
                 "--fuu",
                 std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a')},
                optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionValueExceedMaximumSize_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    FailureTest(
        {"-s", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'), "-b", "blaValue", "-f", "fuuValue"},
        optionsToRegister);
    // middle
    FailureTest(
        {"-s", "blaValue", "-b", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'), "-f", "fuuValue"},
        optionsToRegister);
    // end
    FailureTest(
        {"-s", "blaValue", "-b", "fuuValue", "-f", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a')},
        optionsToRegister);
}
/// END option failure test

/// BEGIN switch failure test
TEST_F(CommandLineParser_test, FailWhenSwitchWasNotRegistered_SingleArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"supergandalf", "grand-alf"};

    FailureTest({"--mario"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchWasNotRegistered_MultiArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"supergandalf", "grand-alf"};

    // begin
    FailureTest({"--mario", "--supergandalf", "--grand-alf"}, optionsToRegister, switchesToRegister);
    // middle
    FailureTest({"--supergandalf", "--mario", "--grand-alf"}, optionsToRegister, switchesToRegister);
    // end
    FailureTest({"--supergandalf", "--grand-alf", "--mario"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchWasNotRegistered_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"supergandalf", "grand-alf"};

    // begin
    FailureTest({"-m", "-s", "-g"}, optionsToRegister, switchesToRegister);
    // middle
    FailureTest({"-s", "-m", "-g"}, optionsToRegister, switchesToRegister);
    // end
    FailureTest({"-s", "-g", "-m"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_SingleArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"set"};

    FailureTest({"--set", "noValueAfterSwitch"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_MultiArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"set", "bla", "fuu"};

    // begin
    FailureTest({"--set", "noValueAfterSwitch", "--bla", "--fuu"}, optionsToRegister, switchesToRegister);
    // middle
    FailureTest({"--set", "--bla", "noValueAfterSwitch", "--fuu"}, optionsToRegister, switchesToRegister);
    // end
    FailureTest({"--set", "--bla", "--fuu", "noValueAfterSwitch"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"set", "bla", "fuu"};

    // begin
    FailureTest({"-s", "noValueAfterSwitch", "-b", "-f"}, optionsToRegister, switchesToRegister);
    // middle
    FailureTest({"-s", "-b", "noValueAfterSwitch", "-f"}, optionsToRegister, switchesToRegister);
    // end
    FailureTest({"-s", "-b", "-f", "noValueAfterSwitch"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchIsSetMultipleTimes_SingleArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"set"};
    FailureTest({"--set", "--set"}, optionsToRegister, switchesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchIsSetMultipleTimes_MultiArgument)
{
    std::vector<std::string> optionsToRegister;
    std::vector<std::string> switchesToRegister{"set", "bla", "fuu"};

    // begin
    FailureTest({"--set", "--set", "--bla", "--fuu"}, optionsToRegister, switchesToRegister);
    // middle
    FailureTest({"--set", "--bla", "--set", "--fuu"}, optionsToRegister, switchesToRegister);
    // end
    FailureTest({"--set", "--bla", "--fuu", "--set"}, optionsToRegister, switchesToRegister);
    // center
    FailureTest({"--set", "--fuu", "--fuu", "--bla"}, optionsToRegister, switchesToRegister);
}

/// END switch failure test

/// BEGIN required option failure test
TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotPresent_SingleArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set", "fuu"};

    FailureTest({"--set", "ohIForgotFuu"}, optionsToRegister, switchesToRegister, requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotPresent_MultiArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set", "fuu", "bla", "muu"};

    // begin
    FailureTest({"--bla", "ohIForgotSet", "--fuu", "someFuu", "--muu", "blaarb"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // middle
    FailureTest({"--set", "ohIForgetBla", "--fuu", "someFuu", "--muu", "blaarb"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // end
    FailureTest({"--set", "ohIForgotMuu", "--fuu", "someFuu", "--bla", "someBlaa"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotPresent_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set", "fuu", "bla", "muu"};

    // begin
    FailureTest({"-b", "ohIForgotSet", "-f", "someFuu", "-m", "blaarb"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // middle
    FailureTest({"-s", "ohIForgetBla", "-f", "someFuu", "-m", "blaarb"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // end
    FailureTest({"-s", "ohIForgotMuu", "-f", "someFuu", "-b", "someBlaa"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotFollowedByValue_SingleArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set"};

    FailureTest({"--set"}, optionsToRegister, switchesToRegister, requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotFollowedByValue_MultiArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set", "fuu", "bla", "toad"};

    // begin
    FailureTest({"--set", "--fuu", "someValue", "--bla", "blaValue", "--toad", "hypno"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // middle
    FailureTest({"--set", "someSet", "--fuu", "someValue", "--bla", "--toad", "hypno"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // end
    FailureTest({"--set", "someSet", "--fuu", "someValue", "--bla", "--toad"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionIsNotFollowedByValue_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"set", "fuu", "bla", "toad"};

    // begin
    FailureTest({"-s", "-f", "someValue", "-b", "blaValue", "-t", "hypno"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // middle
    FailureTest({"-s", "someSet", "-f", "someValue", "-b", "-t", "hypno"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
    // end
    FailureTest({"-s", "someSet", "-f", "someValue", "-b", "-t"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}
/// END required option failure test

/// BEGIN required, optional option and switch failure mix
TEST_F(CommandLineParser_test, FailWhenOneRequiredOptionIsNotSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch", "--f-switch", "--a-opt", "someA", "--k-req", "fSet", "--i-req", "asd"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenMultipleRequiredOptionsAreNotSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch", "--f-switch", "--a-opt", "someA", "--i-req", "asd", "--b-opt", "asd"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenNoRequiredOptionIsSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch", "--f-switch", "--a-opt", "someA", "--e-switch", "--b-opt", "asd"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch",
                 "ohNoASwitchValue",
                 "--f-switch",
                 "--a-opt",
                 "someA",
                 "--k-req",
                 "fSet",
                 "--i-req",
                 "asd",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionHasNoValueSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "ohBHasNoValue",
                 "--b-opt",
                 "--k-req",
                 "fSet",
                 "--i-req",
                 "asd",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenRequiredOptionHasNoValueSet_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "aVal",
                 "--b-opt",
                 "bVal",
                 "--k-req",
                 "ohNoIHasNoValue",
                 "--i-req",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionIsNotRegistered_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "aVal",
                 "--nobody-knows-me",
                 "mrUnknown",
                 "--b-opt",
                 "bVal",
                 "--k-req",
                 "ohNoIHasNoValue",
                 "--i-req",
                 "someI",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchIsNotRegistered_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--unknown-switch",
                 "--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "aVal",
                 "--b-opt",
                 "bVal",
                 "--k-req",
                 "ohNoIHasNoValue",
                 "--i-req",
                 "someI",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister);
}

TEST_F(CommandLineParser_test, IgnoreWhenOptionIsNotRegistered_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "aVal",
                 "--nobody-knows-me",
                 "mrUnknown",
                 "--b-opt",
                 "bVal",
                 "--k-req",
                 "ohNoIHasNoValue",
                 "--i-req",
                 "someI",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister,
                UnknownOption::IGNORE);
}

TEST_F(CommandLineParser_test, IgnoreWhenSwitchIsNotRegistered_MixedArguments)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"i-req", "j-req", "k-req"};

    FailureTest({"--unknown-switch",
                 "--d-switch",
                 "--f-switch",
                 "--a-opt",
                 "aVal",
                 "--b-opt",
                 "bVal",
                 "--k-req",
                 "ohNoIHasNoValue",
                 "--i-req",
                 "someI",
                 "--j-req",
                 "fuu"},
                optionsToRegister,
                switchesToRegister,
                requiredValuesToRegister,
                UnknownOption::IGNORE);
}
/// END required, optional option and switch failure mix

CommandLineOptions SuccessTest(const std::vector<std::string>& options,
                               const std::vector<std::string>& optionsToRegister = {},
                               const std::vector<std::string>& switchesToRegister = {},
                               const std::vector<std::string>& requiredValuesToRegister = {},
                               const uint64_t argcOffset = 1U) noexcept
{
    const CommandLineOptions::binaryName_t binaryName("GloryToTheHasselToad");
    std::vector<std::string> optionVector{binaryName.c_str()};
    optionVector.insert(optionVector.end(), options.begin(), options.end());
    CmdArgs args(optionVector);
    CommandLineOptions retVal;

    bool wasErrorHandlerCalled = false;
    {
        CommandLineParser parser("");
        for (const auto& o : optionsToRegister)
        {
            parser.addOptionalValue(o[0],
                                    CommandLineOptions::name_t(TruncateToCapacity, o),
                                    "",
                                    "int",
                                    CommandLineParser_test::defaultValue);
        }
        for (const auto& s : switchesToRegister)
        {
            parser.addSwitch(s[0], CommandLineOptions::name_t{TruncateToCapacity, s}, "");
        }
        for (const auto& r : requiredValuesToRegister)
        {
            parser.addRequiredValue(r[0], CommandLineOptions::name_t(TruncateToCapacity, r), "", "int");
        }

        {
            auto handle =
                iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
            retVal = parser.parse(args.argc, args.argv, argcOffset, UnknownOption::IGNORE);
        }
    }
    EXPECT_FALSE(wasErrorHandlerCalled);
    return retVal;
}

template <typename T>
void verifyEntry(const CommandLineOptions& options,
                 const CommandLineOptions::name_t& entry,
                 const iox::cxx::optional<T>& value)
{
    auto result = options.get<T>(entry);

    value
        .and_then([&](auto& v) {
            // ASSERT_ does not work in function calls
            if (result.has_error())
            {
                EXPECT_TRUE(false);
                return;
            }

            EXPECT_THAT(*result, Eq(v));
        })
        .or_else([&] {
            if (!result.has_error())
            {
                EXPECT_TRUE(false);
                return;
            }
            EXPECT_THAT(result.get_error(), Eq(CommandLineOptions::Result::NO_SUCH_VALUE));
        });
}

/// BEGIN acquire values correctly

TEST_F(CommandLineParser_test, ReadOptionSuccessfully_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"conway"};
    auto option = SuccessTest({"--conway", "gameOfLife"}, optionsToRegister);

    verifyEntry<std::string>(option, "conway", {"gameOfLife"});
}

TEST_F(CommandLineParser_test, ReadOptionSuccessfully_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"conway", "tungsten", "moon"};
    auto option = SuccessTest({"--moon", "bright", "--conway", "gameOfLife", "--tungsten", "heavy"}, optionsToRegister);

    verifyEntry<std::string>(option, "conway", {"gameOfLife"});
    verifyEntry<std::string>(option, "moon", {"bright"});
    verifyEntry<std::string>(option, "tungsten", {"heavy"});
}

TEST_F(CommandLineParser_test, ReadOptionSuccessfully_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"conway", "tungsten", "moon"};
    auto option = SuccessTest({"-m", "bright", "-c", "gameOfLife", "-t", "heavy"}, optionsToRegister);

    verifyEntry<std::string>(option, "c", {"gameOfLife"});
    verifyEntry<std::string>(option, "m", {"bright"});
    verifyEntry<std::string>(option, "t", {"heavy"});
}

TEST_F(CommandLineParser_test, ReadOptionSuccessfully_PartialSet)
{
    std::vector<std::string> optionsToRegister{"conway", "tungsten", "moon"};
    auto option = SuccessTest({"-m", "bright"}, optionsToRegister);

    verifyEntry<std::string>(option, "moon", {"bright"});
    verifyEntry<std::string>(option, "conway", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "tungsten", {defaultValue.c_str()});
}

TEST_F(CommandLineParser_test, ReadOptionSuccessfully_Offset)
{
    std::vector<std::string> optionsToRegister{"conway", "tungsten", "moon"};
    constexpr uint64_t ARGC_OFFSET = 5U;
    auto option =
        SuccessTest({"whatever", "bright", "-t", "heavy", "-c", "gameOfLife"}, optionsToRegister, {}, {}, ARGC_OFFSET);

    verifyEntry<std::string>(option, "moon", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "conway", {"gameOfLife"});
    verifyEntry<std::string>(option, "tungsten", {defaultValue.c_str()});
}

TEST_F(CommandLineParser_test, ReadRequiredValueSuccessfully_SingleArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"fuubar"};
    auto option = SuccessTest({"--fuubar", "ohFuBa"}, optionsToRegister, switchesToRegister, requiredValuesToRegister);

    verifyEntry<std::string>(option, "fuubar", {"ohFuBa"});
}

TEST_F(CommandLineParser_test, ReadRequiredValueSuccessfully_MultiArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"fuubar", "c64", "amiga"};
    auto option = SuccessTest({"--fuubar", "ohFuBa", "--amiga", "Os2 Warp", "--c64", "cobra"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister);

    verifyEntry<std::string>(option, "fuubar", {"ohFuBa"});
    verifyEntry<std::string>(option, "amiga", {"Os2 Warp"});
    verifyEntry<std::string>(option, "c64", {"cobra"});
}

TEST_F(CommandLineParser_test, ReadRequiredValueSuccessfully_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"fuubar", "c64", "amiga"};
    auto option = SuccessTest({"-f", "ohFuBa", "-a", "Os2 Warp", "-c", "cobra"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister);

    verifyEntry<std::string>(option, "f", {"ohFuBa"});
    verifyEntry<std::string>(option, "a", {"Os2 Warp"});
    verifyEntry<std::string>(option, "c", {"cobra"});
}

TEST_F(CommandLineParser_test, ReadRequiredValueSuccessfully_Offset)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{};
    std::vector<std::string> requiredValuesToRegister{"fuubar", "c64", "amiga"};
    constexpr uint64_t ARGC_OFFSET = 3U;
    auto option = SuccessTest({"-f", "iWillNotBeParsed", "-f", "ohFuBa", "-a", "Os2 Warp", "-c", "cobra"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister,
                              ARGC_OFFSET);

    verifyEntry<std::string>(option, "f", {"ohFuBa"});
    verifyEntry<std::string>(option, "a", {"Os2 Warp"});
    verifyEntry<std::string>(option, "c", {"cobra"});
}

TEST_F(CommandLineParser_test, ReadSwitchValueSuccessfullyWhenSet_SingleArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{"light"};
    auto option = SuccessTest({"--light"}, optionsToRegister, switchesToRegister);

    EXPECT_TRUE(option.has("light"));
}

TEST_F(CommandLineParser_test, ReadSwitchValueSuccessfullyWhenSet_MultiArgument)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{"light", "fridge", "muu"};
    auto option = SuccessTest({"--light", "--fridge", "--muu"}, optionsToRegister, switchesToRegister);

    EXPECT_TRUE(option.has("light"));
    EXPECT_TRUE(option.has("fridge"));
    EXPECT_TRUE(option.has("muu"));
}

TEST_F(CommandLineParser_test, ReadSwitchValueSuccessfullyWhenSet_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{"light", "fridge", "muu"};
    auto option = SuccessTest({"-l", "-f", "-m"}, optionsToRegister, switchesToRegister);

    EXPECT_TRUE(option.has("l"));
    EXPECT_TRUE(option.has("f"));
    EXPECT_TRUE(option.has("m"));
}

TEST_F(CommandLineParser_test, ReadSwitchValueSuccessfullyWhenSet_PartialSet)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{"light", "fridge", "muu"};
    auto option = SuccessTest({"-l"}, optionsToRegister, switchesToRegister);

    EXPECT_TRUE(option.has("light"));
    EXPECT_FALSE(option.has("fridge"));
    EXPECT_FALSE(option.has("muu"));
}

TEST_F(CommandLineParser_test, ReadSwitchValueSuccessfullyWhenSet_Offset)
{
    std::vector<std::string> optionsToRegister{};
    std::vector<std::string> switchesToRegister{"light", "fridge", "muu"};
    constexpr uint64_t ARGC_OFFSET = 2U;
    auto option =
        SuccessTest({"----unknown-dont-care", "-f", "-m"}, optionsToRegister, switchesToRegister, {}, ARGC_OFFSET);

    EXPECT_FALSE(option.has("light"));
    EXPECT_TRUE(option.has("fridge"));
    EXPECT_TRUE(option.has("muu"));
}
/// END acquire values correctly

/// BEGIN acquire mixed values correctly

TEST_F(CommandLineParser_test, ReadMixedValueSuccessfully)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"g-req", "i-req", "j-req"};
    auto option = SuccessTest({"--a-opt",
                               "oh-my-blah",
                               "--d-switch",
                               "--i-req",
                               "someI",
                               "--j-req",
                               "someJ",
                               "--f-switch",
                               "--g-req",
                               "someG"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister);

    verifyEntry<std::string>(option, "a-opt", {"oh-my-blah"});
    verifyEntry<std::string>(option, "b-opt", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "c-opt", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "i-req", {"someI"});
    verifyEntry<std::string>(option, "j-req", {"someJ"});
    verifyEntry<std::string>(option, "g-req", {"someG"});

    EXPECT_TRUE(option.has("d-switch"));
    EXPECT_FALSE(option.has("e-switch"));
    EXPECT_TRUE(option.has("f-switch"));
}

TEST_F(CommandLineParser_test, ReadMixedValueSuccessfully_ShortOption)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"g-req", "i-req", "j-req"};
    auto option = SuccessTest({"-a", "anotherA", "-b", "someB", "-e", "-i", "blaI", "-j", "blaJ", "-g", "blaG"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister);

    verifyEntry<std::string>(option, "a-opt", {"anotherA"});
    verifyEntry<std::string>(option, "b-opt", {"someB"});
    verifyEntry<std::string>(option, "c-opt", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "i-req", {"blaI"});
    verifyEntry<std::string>(option, "j-req", {"blaJ"});
    verifyEntry<std::string>(option, "g-req", {"blaG"});

    EXPECT_FALSE(option.has("d-switch"));
    EXPECT_TRUE(option.has("e-switch"));
    EXPECT_FALSE(option.has("f-switch"));
}

TEST_F(CommandLineParser_test, ReadMixedValueSuccessfully_Offset)
{
    std::vector<std::string> optionsToRegister{"a-opt", "b-opt", "c-opt"};
    std::vector<std::string> switchesToRegister{"d-switch", "e-switch", "f-switch"};
    std::vector<std::string> requiredValuesToRegister{"g-req", "i-req", "j-req"};
    constexpr uint64_t ARGC_OFFSET = 3U;
    auto option = SuccessTest({"-a", "anotherA", "-b", "someB", "-e", "-i", "blaI", "-j", "blaJ", "-g", "blaG"},
                              optionsToRegister,
                              switchesToRegister,
                              requiredValuesToRegister,
                              ARGC_OFFSET);

    verifyEntry<std::string>(option, "a-opt", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "b-opt", {"someB"});
    verifyEntry<std::string>(option, "c-opt", {defaultValue.c_str()});
    verifyEntry<std::string>(option, "i-req", {"blaI"});
    verifyEntry<std::string>(option, "j-req", {"blaJ"});
    verifyEntry<std::string>(option, "g-req", {"blaG"});

    EXPECT_FALSE(option.has("d-switch"));
    EXPECT_TRUE(option.has("e-switch"));
    EXPECT_FALSE(option.has("f-switch"));
}
/// END acquire mixed values correctly
} // namespace
