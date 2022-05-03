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
#include "iceoryx_hoofs/internal/cli/command_line_option_set.hpp"
#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"
#include "test.hpp"
#include "test_cli_command_line_common.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::cli;
using namespace iox::cli::internal;
using namespace iox::cxx;

class CommandLineOptionSet_test : public Test
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

    uint64_t numberOfErrorCallbackCalls = 0U;
    iox::cxx::function<void()> errorCallback = [this] { ++numberOfErrorCallbackCalls; };
    static Argument_t defaultValue;
};
Argument_t CommandLineOptionSet_test::defaultValue = "DEFAULT VALUE";

TEST_F(CommandLineOptionSet_test, AddingTheSameShortOptionLeadsToExit)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1340876-e3f6-4f62-b0f3-4e9551a5f67a");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addOptional('c', "firstEntry", "", "", "");

    optionSet.addOptional('c', "duplicateShortOption", "", "", "");
    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingTheSameLongOptionLeadsToExit)
{
    ::testing::Test::RecordProperty("TEST_ID", "076b8877-e3fc-46f7-851b-d3e7953f67d6");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addOptional('c', "duplicate", "", "", "");

    optionSet.addOptional('x', "duplicate", "", "", "");
    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingOptionWithSameShortAndLongNameLeadsToExit)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e01ed47-473d-4915-aed2-60aacce37de8");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addOptional('d', "duplicate", "", "", "");

    optionSet.addOptional('d', "duplicate", "", "", "");
    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingSwitchWithMinusAsShortOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c6558ec-ecd9-47e9-b396-593445cef68f");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addSwitch('-', "", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingOptionalValueWithMinusAsShortOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "8afd403b-9a77-4bde-92df-0200d4fb661b");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addOptional('-', "", "", "", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingRequiredValueWithMinusAsShortOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "04e358dd-6ef4-48e4-988e-ee1d0514632b");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addMandatory('-', "", "", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingSwitchWithMinusStartingLongOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "62c0882a-7055-4a74-9dd9-8505d72da1e0");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addSwitch('a', "-oh-no-i-start-with-minus", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingOptionalValueWithMinusStartingLongOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "69c975d1-57d3-429a-b894-7ff1efa9f473");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addOptional('c', "-whoopsie-there-is-a-minus", "", "", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}

TEST_F(CommandLineOptionSet_test, AddingRequiredValueWithMinusStartingLongOptionLeadsToFailure)
{
    ::testing::Test::RecordProperty("TEST_ID", "43929047-1051-45cd-8a13-ebf8ea8c4e26");
    CommandLineOptionSet optionSet("", errorCallback);
    optionSet.addMandatory('b', "-minus-is-all-i-need", "", "");

    EXPECT_THAT(numberOfErrorCallbackCalls, Eq(1));
}
} // namespace
