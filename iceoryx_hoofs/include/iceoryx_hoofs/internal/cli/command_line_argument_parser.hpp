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
#ifndef IOX_HOOFS_CLI_COMMAND_LINE_PARSER_HPP
#define IOX_HOOFS_CLI_COMMAND_LINE_PARSER_HPP

#include "iceoryx_hoofs/internal/cli/command_line_option_set.hpp"
#include "iceoryx_hoofs/internal/cli/command_line_option_value.hpp"
#include <cstdint>

namespace iox
{
namespace cli
{
namespace internal
{
/// @brief Factory class for the CommandLineOption. First, one has to register
///        all switches and options before calling parse. This is required for
///        the help page which is generated and printed on failure as well as
///        for consistency and syntax checks.
class CommandLineArgumentParser
{
  public:
    static constexpr uint64_t OPTION_OUTPUT_WIDTH = 45;

  private:
    friend class OptionManager;
    friend CommandLineOptionValue
    parseCommandLineArguments(const CommandLineOptionSet&, int, char*[], const uint64_t, const UnknownOption) noexcept;

    /// @brief Parses the arguments from the command line.
    ///        Calls onFailureCallback in optionSet when the command line arguments contain illegal syntax or required
    ///        values are not provided and prints the help.
    /// @param[in] optionSet the user defined options, based on those options the CommandLineOptionValue object is
    ///            generated
    /// @param[in] argc number of arguments, see int main(int argc, char*argv[])
    /// @param[in] argv the string array of arguments, see int main(int argc, char*argv[])
    /// @param[in] argcOffset the starting point for the parsing. 1U starts at the first argument.
    /// @param[in] actionWhenOptionUnknown defines the action which should be performed when the user sets a
    ///             option/switch which is unknown
    CommandLineOptionValue parse(const CommandLineOptionSet& optionSet,
                                 int argc,
                                 char* argv[],
                                 const uint64_t argcOffset = 1U,
                                 const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE) noexcept;

    void printHelpAndExit() const noexcept;

    /// BEGIN only used in parse to improve readability
    ///
    /// Do not use those internal methods outside of parse. They were written
    /// to improve the readability of the code. None of those functions verify
    /// the pre conditions they require, this has to be done by calling them
    /// in the correct order.
    bool areAllRequiredValuesPresent() const noexcept;
    bool hasArguments(const int argc) const noexcept;
    bool assignBinaryName(const char* name) noexcept;
    bool doesOptionStartWithMinus(const char* option) const noexcept;
    bool hasOptionName(const char* option) const noexcept;
    bool hasValidShortOptionDashCount(const char* option) const noexcept;
    bool hasValidOptionDashCount(const char* option) const noexcept;
    bool doesOptionNameFitIntoString(const char* option) const noexcept;
    bool isNextArgumentAValue(const uint64_t position) const noexcept;
    bool isOptionSet(const CommandLineOptionSet::Value& entry) const noexcept;
    bool doesOptionValueFitIntoString(const char* value) const noexcept;
    bool doesOptionHasSucceedingValue(const CommandLineOptionSet::Value& entry, const uint64_t position) const noexcept;
    /// END only used in parse to improve readability

    void setDefaultValuesToUnsetOptions() noexcept;

  private:
    uint64_t m_argc = 0;
    char** m_argv = nullptr;
    uint64_t m_argcOffset = 0;

    const CommandLineOptionSet* m_optionSet = nullptr;
    CommandLineOptionValue m_optionValue;
};

/// @copydoc CommandLineArgumentParser::parse()
CommandLineOptionValue
parseCommandLineArguments(const CommandLineOptionSet& optionSet,
                          int argc,
                          char* argv[],
                          const uint64_t argcOffset = 1U,
                          const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE) noexcept;

} // namespace internal
} // namespace cli
} // namespace iox

#endif
