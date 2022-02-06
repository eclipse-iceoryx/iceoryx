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
            parser.addOption({CommandLineParser::NO_SHORT_OPTION,
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

/// TODO: this test has to many error outputs check line 159 in cpp
TEST_F(CommandLineParser_test, FailWhenOptionDoesNotStartWithMinus)
{
    OptionFailureTest({"i-have-no-minus"});
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameIsEmpty)
{
    OptionFailureTest({"-"});
}

TEST_F(CommandLineParser_test, FailWhenOptionNameIsEmpty)
{
    OptionFailureTest({"--"});
}

TEST_F(CommandLineParser_test, FailWhenShortOptionNameHasMoreThenOneLetter)
{
    OptionFailureTest({"-invalid-option"});
}

TEST_F(CommandLineParser_test, FailWhenLongOptionStartsWithTripleMinus)
{
    OptionFailureTest({"---invalid-long-option"});
}

TEST_F(CommandLineParser_test, FailWhenOptionNameExceedMaximumSize)
{
    OptionFailureTest({std::string("--") + std::string(CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1, 'a')});
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsFollowedByAnotherOption)
{
    std::vector<std::string> optionsToRegister{"set"};
    OptionFailureTest({"--set", "--oh-no-i-am-an-option"}, optionsToRegister);
}

TEST_F(CommandLineParser_test, FailWhenValueOptionIsAtTheEnd)
{
    std::vector<std::string> optionsToRegister{"set"};
    OptionFailureTest({"--set"}, optionsToRegister);
}
} // namespace
