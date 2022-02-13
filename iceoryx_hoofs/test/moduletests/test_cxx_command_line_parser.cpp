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
    static constexpr uint64_t MAX_ARGUMENTS = CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS + 1;
};

// TEST TODO:
// failureMixWithSwitchAndOption
// failureRequiredOptions
// maxArgumentsAreSupported
// failureWhenLargerMaxArguments
// argcOffset
// UnknownOption
// correct results, switch, option/value, mix

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
        auto options = CommandLineParser("").parse(0, nullptr);
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
        auto options = CommandLineParser("").parse(args.argc, args.argv);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

///////////////////////////
/// BEGIN optionFailureTest
///////////////////////////

void OptionFailureTest(const std::vector<std::string>& options,
                       const std::vector<std::string>& optionsToRegister = {}) noexcept
{
    const CommandLineOptions::binaryName_t binaryName("GloryToTheHasselToad");
    std::vector<std::string> optionVector{binaryName.c_str()};
    optionVector.insert(optionVector.end(), options.begin(), options.end());
    CmdArgs args(optionVector);

    bool wasErrorHandlerCalled = false;
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        CommandLineParser parser("");
        for (const auto& o : optionsToRegister)
        {
            parser.addOption({o[0],
                              CommandLineOptions::name_t(TruncateToCapacity, o),
                              "",
                              ArgumentType::OPTIONAL_VALUE,
                              "int",
                              "0"});
        }
        auto options = parser.parse(args.argc, args.argv);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, FailWhenOptionDoesNotStartWithMinus_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus"};
    OptionFailureTest({"i-have-no-minus"});
    OptionFailureTest({"i-have-no-minus", "someValue"});
}

TEST_F(CommandLineParser_test, FailWhenOptionDoesNotStartWithMinus_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus", "set", "bla"};
    // begin
    OptionFailureTest({"i-have-no-minus", "--set", "setValue", "--bla", "blaValue"}, optionsToRegister);
    OptionFailureTest({"i-have-no-minus", "someValue", "--set", "setValue", "--bla", "blaValue"}, optionsToRegister);
    // middle
    OptionFailureTest({"--set", "setValue", "i-have-no-minus", "--bla", "blaValue"}, optionsToRegister);
    OptionFailureTest({"--set", "setValue", "i-have-no-minus", "someValue", "--bla", "blaValue"}, optionsToRegister);
    // end
    OptionFailureTest({"--set", "setValue", "--bla", "blaValue", "i-have-no-minus"}, optionsToRegister);
    OptionFailureTest({"--set", "setValue", "--bla", "blaValue", "i-have-no-minus", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionDoesNotStartWithMinus_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"i-have-no-minus", "set", "bla"};
    // begin
    OptionFailureTest({"i", "-s", "setValue", "-b", "blaValue"}, optionsToRegister);
    OptionFailureTest({"i", "someValue", "-s", "setValue", "-b", "blaValue"}, optionsToRegister);
    // middle
    OptionFailureTest({"-s", "setValue", "i", "-b", "blaValue"}, optionsToRegister);
    OptionFailureTest({"-s", "setValue", "i", "someValue", "-b", "blaValue"}, optionsToRegister);
    // end
    OptionFailureTest({"-s", "setValue", "-b", "blaValue", "i"}, optionsToRegister);
    OptionFailureTest({"-s", "setValue", "-b", "blaValue", "i", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameIsEmpty_SingleArgument)
{
    OptionFailureTest({"-"});
    OptionFailureTest({"-", "someValue"});
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameIsEmpty_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    OptionFailureTest({"-", "--set", "setValue123", "--bla", "blaValue455"}, optionsToRegister);
    OptionFailureTest({"-", "someValue", "--set", "setValue123", "--bla", "blaValue455"}, optionsToRegister);
    // middle
    OptionFailureTest({"--set", "setValue123", "-", "--bla", "blaValue455"}, optionsToRegister);
    OptionFailureTest({"--set", "setValue123", "-", "someValue", "--bla", "blaValue455"}, optionsToRegister);
    // end
    OptionFailureTest({"--set", "setValue123", "--bla", "blaValue455", "-"}, optionsToRegister);
    OptionFailureTest({"--set", "setValue123", "--bla", "blaValue455", "-", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionNameIsEmpty_SingleArgument)
{
    OptionFailureTest({"--"});
    OptionFailureTest({"--", "someValue"});
}

TEST_F(CommandLineParser_test, FailWhenOptionNameIsEmpty_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    OptionFailureTest({"--", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"--", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    // middle
    OptionFailureTest({"--bla", "blaValue123123", "--", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "--", "someValue", "--set", "setValueXXX"}, optionsToRegister);
    // end
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "--"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "--", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameHasMoreThenOneLetter_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"invalid-option"};
    OptionFailureTest({"-invalid-option"}, optionsToRegister);
    OptionFailureTest({"-invalid-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameHasMoreThenOneLetter_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "invalid-option"};
    // begin
    OptionFailureTest({"-invalid-option", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"-invalid-option", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"},
                      optionsToRegister);
    // middle
    OptionFailureTest({"--bla", "blaValue123123", "-invalid-option", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "-invalid-option", "someValue", "--set", "setValueXXX"},
                      optionsToRegister);
    // end
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "-invalid-option"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "-invalid-option", "someValue"},
                      optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenLongOptionStartsWithTripleMinus_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"invalid-long-option"};
    OptionFailureTest({"---invalid-long-option"}, optionsToRegister);
    OptionFailureTest({"---invalid-long-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenLongOptionStartsWithTripleMinus_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "invalid-long-option"};
    // begin
    OptionFailureTest({"---invalid-long-option", "--bla", "blaValue123123", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"---invalid-long-option", "someValue", "--bla", "blaValue123123", "--set", "setValueXXX"},
                      optionsToRegister);
    // middle
    OptionFailureTest({"--bla", "blaValue123123", "---invalid-long-option", "--set", "setValueXXX"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "---invalid-long-option", "someValue", "--set", "setValueXXX"},
                      optionsToRegister);
    // end
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "---invalid-long-option"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue123123", "--set", "setValueXXX", "---invalid-long-option", "someValue"},
                      optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionNameExceedMaximumSize_SingleArgument)
{
    OptionFailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a')});
    OptionFailureTest(
        {std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'), "someValue"});
}

TEST_F(CommandLineParser_test, FailWhenOptionNameExceedMaximumSize_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla"};
    // begin
    OptionFailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                       "--set",
                       "setValue",
                       "--bla",
                       "blaValue"},
                      optionsToRegister);
    OptionFailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                       "someValue",
                       "--set",
                       "setValue",
                       "--bla",
                       "blaValue"},
                      optionsToRegister);
    // middle
    OptionFailureTest({"--set",
                       "setValue",
                       std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                       "--bla",
                       "blaValue"},
                      optionsToRegister);
    OptionFailureTest({"someValue",
                       "--set",
                       std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                       "setValue",
                       "--bla",
                       "blaValue"},
                      optionsToRegister);
    // end
    OptionFailureTest({"--set",
                       "setValue",
                       "--bla",
                       "blaValue",
                       std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a')},
                      optionsToRegister);
    OptionFailureTest({"--set",
                       "setValue",
                       "--bla",
                       "blaValue",
                       std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a'),
                       "someValue"},
                      optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set", "oh-no-i-am-an-option"};
    OptionFailureTest({"--set", "--oh-no-i-am-an-option"}, optionsToRegister);
    OptionFailureTest({"--set", "--oh-no-i-am-an-option", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu", "oh-no-i-am-an-option"};
    // begin
    OptionFailureTest({"--set", "--oh-no-i-am-an-option", "--bla", "blaValue", "--fuu", "fuuValue"}, optionsToRegister);
    OptionFailureTest({"--set", "--oh-no-i-am-an-option", "someValue", "--bla", "blaValue", "--fuu", "fuuValue"},
                      optionsToRegister);
    // middle
    OptionFailureTest({"--bla", "--set", "--oh-no-i-am-an-option", "--fuu", "fuuValue"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue", "--set", "--oh-no-i-am-an-option", "someValue", "--fuu", "fuuValue"},
                      optionsToRegister);

    // end
    OptionFailureTest({"--fuu", "--bla", "--set", "--oh-no-i-am-an-option"}, optionsToRegister);
    OptionFailureTest({"--fuu", "fuuValue", "--bla", "blaValue", "--set", "--oh-no-i-am-an-option", "someValue"},
                      optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu", "oh-no-i-am-an-option"};
    // begin
    OptionFailureTest({"-s", "-o", "-b", "blaValue", "-f", "fuuValue"}, optionsToRegister);
    OptionFailureTest({"-s", "-o", "someValue", "-b", "blaValue", "-f", "fuuValue"}, optionsToRegister);
    // middle
    OptionFailureTest({"-b", "-s", "-o", "-f", "fuuValue"}, optionsToRegister);
    OptionFailureTest({"-b", "blaValue", "-s", "-o", "someValue", "-f", "fuuValue"}, optionsToRegister);

    // end
    OptionFailureTest({"-f", "-b", "-s", "-o"}, optionsToRegister);
    OptionFailureTest({"-f", "fuuValue", "-b", "blaValue", "-s", "-o", "someValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    OptionFailureTest({"--set", "bla", "--set", "fuu"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};
    OptionFailureTest({"--set", "fuuu", "--bla", "blaValue", "--fuu", "fuuValue", "--set", "bla"}, optionsToRegister);
    OptionFailureTest({"--bla", "blaValue", "--set", "fuuu", "--fuu", "fuuValue", "--set", "bla"}, optionsToRegister);
    OptionFailureTest({"--set", "fuuu", "--bla", "blaValue", "--set", "bla", "--fuu", "fuuValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsSetMultipleTimes_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};
    OptionFailureTest({"-s", "fuuu", "-b", "blaValue", "-f", "fuuValue", "-s", "bla"}, optionsToRegister);
    OptionFailureTest({"-b", "blaValue", "-s", "fuuu", "-f", "fuuValue", "-s", "bla"}, optionsToRegister);
    OptionFailureTest({"-s", "fuuu", "-b", "blaValue", "-s", "bla", "-f", "fuuValue"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionValueExceedMaximumSize_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    OptionFailureTest({"--set", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a')}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenOptionValueExceedMaximumSize_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    OptionFailureTest({"--set",
                       std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'),
                       "--bla",
                       "blaValue",
                       "--fuu",
                       "fuuValue"},
                      optionsToRegister);
    // middle
    OptionFailureTest({"--set",
                       "blaValue",
                       "--bla",
                       std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'),
                       "--fuu",
                       "fuuValue"},
                      optionsToRegister);
    // end
    OptionFailureTest({"--set",
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
    OptionFailureTest(
        {"-s", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'), "-b", "blaValue", "-f", "fuuValue"},
        optionsToRegister);
    // middle
    OptionFailureTest(
        {"-s", "blaValue", "-b", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a'), "-f", "fuuValue"},
        optionsToRegister);
    // end
    OptionFailureTest(
        {"-s", "blaValue", "-b", "fuuValue", "-f", std::string(CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1, 'a')},
        optionsToRegister);
}

///////////////////////////
/// END optionFailureTest
///////////////////////////

///////////////////////////
/// BEGIN switchFailureTest
///////////////////////////

void SwitchFailureTest(const std::vector<std::string>& options,
                       const std::vector<std::string>& switchesToRegister = {}) noexcept
{
    const CommandLineOptions::binaryName_t binaryName("GloryToTheHasselToad");
    std::vector<std::string> optionVector{binaryName.c_str()};
    optionVector.insert(optionVector.end(), options.begin(), options.end());
    CmdArgs args(optionVector);

    bool wasErrorHandlerCalled = false;
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        CommandLineParser parser("");
        for (const auto& o : switchesToRegister)
        {
            parser.addOption(
                {o[0], CommandLineOptions::name_t(TruncateToCapacity, o), "", ArgumentType::SWITCH, "int", "0"});
        }
        auto options = parser.parse(args.argc, args.argv);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    SwitchFailureTest({"--set", "noValueAfterSwitch"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    SwitchFailureTest({"--set", "noValueAfterSwitch", "--bla", "--fuu"}, optionsToRegister);
    // middle
    SwitchFailureTest({"--set", "--bla", "noValueAfterSwitch", "--fuu"}, optionsToRegister);
    // end
    SwitchFailureTest({"--set", "--bla", "--fuu", "noValueAfterSwitch"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchHasValueSet_MultiArgument_ShortOption)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    SwitchFailureTest({"-s", "noValueAfterSwitch", "-b", "-f"}, optionsToRegister);
    // middle
    SwitchFailureTest({"-s", "-b", "noValueAfterSwitch", "-f"}, optionsToRegister);
    // end
    SwitchFailureTest({"-s", "-b", "-f", "noValueAfterSwitch"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchIsSetMultipleTimes_SingleArgument)
{
    std::vector<std::string> optionsToRegister{"set"};
    SwitchFailureTest({"--set", "--set"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenSwitchIsSetMultipleTimes_MultiArgument)
{
    std::vector<std::string> optionsToRegister{"set", "bla", "fuu"};

    // begin
    SwitchFailureTest({"--set", "--set", "--bla", "--fuu"}, optionsToRegister);
    // middle
    SwitchFailureTest({"--set", "--bla", "--set", "--fuu"}, optionsToRegister);
    // end
    SwitchFailureTest({"--set", "--bla", "--fuu", "--set"}, optionsToRegister);
    // center
    SwitchFailureTest({"--set", "--fuu", "--fuu", "--bla"}, optionsToRegister);
}

///////////////////////////
/// END switchFailureTest
///////////////////////////

} // namespace
