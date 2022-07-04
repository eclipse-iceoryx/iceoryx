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

#include "iceoryx_hoofs/internal/cli/command_line_argument_parser.hpp"
#include "iceoryx_hoofs/cxx/algorithm.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include <algorithm>

namespace iox
{
namespace cli
{
namespace internal
{
CommandLineOptionValue parseCommandLineArguments(const CommandLineOptionSet& optionSet,
                                                 int argc,
                                                 char* argv[],
                                                 const uint64_t argcOffset,
                                                 const UnknownOption actionWhenOptionUnknown) noexcept
{
    return CommandLineArgumentParser().parse(optionSet, argc, argv, argcOffset, actionWhenOptionUnknown);
}

bool CommandLineArgumentParser::hasArguments(const uint64_t argc) const noexcept
{
    const bool hasArguments = (argc > 0);
    if (!hasArguments)
    {
        printHelpAndExit();
    }
    return hasArguments;
}

bool CommandLineArgumentParser::assignBinaryName(const char* name) noexcept
{
    const bool binaryNameFitsIntoString = m_optionValue.m_binaryName.unsafe_assign(name);
    if (!binaryNameFitsIntoString)
    {
        std::cout << "The \"" << name << "\" binary path is too long" << std::endl;
        printHelpAndExit();
    }
    return binaryNameFitsIntoString;
}

bool CommandLineArgumentParser::doesOptionStartWithDash(const char* option) const noexcept
{
    const bool doesOptionStartWithDash = (strnlen(option, 1) > 0 && option[0] == '-');

    if (!doesOptionStartWithDash)
    {
        std::cout << "Every option has to start with \"-\" but \"" << option << "\" does not." << std::endl;
        printHelpAndExit();
    }
    return doesOptionStartWithDash;
}

bool CommandLineArgumentParser::hasNonEmptyOptionName(const char* option) const noexcept
{
    const uint64_t minArgIdentifierLength = strnlen(option, 3);
    const bool hasNonEmptyOptionName =
        !(minArgIdentifierLength == 1 || (minArgIdentifierLength == 2 && option[1] == '-'));

    if (!hasNonEmptyOptionName)
    {
        std::cout << "Empty option names are forbidden" << std::endl;
        printHelpAndExit();
    }

    return hasNonEmptyOptionName;
}

bool CommandLineArgumentParser::doesNotHaveLongOptionDash(const char* option) const noexcept
{
    const uint64_t minArgIdentifierLength = strnlen(option, 3);
    const bool doesNotHaveLongOptionDash = !(minArgIdentifierLength > 2 && option[1] != '-');

    if (!doesNotHaveLongOptionDash)
    {
        std::cout << "Only one letter allowed when using a short option name. The switch \"" << option
                  << "\" is not valid." << std::endl;
        printHelpAndExit();
    }
    return doesNotHaveLongOptionDash;
}

bool CommandLineArgumentParser::doesNotExceedLongOptionDash(const char* option) const noexcept
{
    const uint64_t minArgIdentifierLength = strnlen(option, 3);
    const bool doesNotExceedLongOptionDash = !(minArgIdentifierLength > 2 && option[2] == '-');

    if (!doesNotExceedLongOptionDash)
    {
        std::cout << "A long option name should start after \"--\". This \"" << option << "\" is not valid."
                  << std::endl;
        printHelpAndExit();
    }
    return doesNotExceedLongOptionDash;
}

bool CommandLineArgumentParser::doesFitIntoString(const char* value, const uint64_t maxLength) const noexcept
{
    return (strnlen(value, maxLength + 1) <= maxLength);
}

bool CommandLineArgumentParser::doesOptionNameFitIntoString(const char* option) const noexcept
{
    const bool doesOptionNameFitIntoString = doesFitIntoString(option, MAX_OPTION_NAME_LENGTH);

    if (!doesOptionNameFitIntoString)
    {
        std::cout << "\"" << option << "\" is longer then the maximum supported size of " << MAX_OPTION_NAME_LENGTH
                  << " for option names." << std::endl;
        printHelpAndExit();
    }
    return doesOptionNameFitIntoString;
}

bool CommandLineArgumentParser::isNextArgumentAValue(const uint64_t position) const noexcept
{
    uint64_t nextPosition = position + 1;
    return (m_argc > 0 && m_argc > nextPosition
            && (strnlen(m_argv[nextPosition], MAX_OPTION_NAME_LENGTH) > 0 && m_argv[nextPosition][0] != '-'));
}

bool CommandLineArgumentParser::isOptionSet(const OptionWithDetails& value) const noexcept
{
    bool isOptionSet = false;
    for (const auto& option : m_optionValue.m_arguments)
    {
        if (option.isSameOption(value))
        {
            isOptionSet = true;
            break;
        }
    }

    if (isOptionSet)
    {
        std::cout << "The option \"" << value << "\" is already set!" << std::endl;
        printHelpAndExit();
    }

    return isOptionSet;
}

bool CommandLineArgumentParser::doesOptionValueFitIntoString(const char* value) const noexcept
{
    const bool doesOptionValueFitIntoString = doesFitIntoString(value, MAX_OPTION_ARGUMENT_LENGTH);

    if (!doesOptionValueFitIntoString)
    {
        std::cout << "\"" << value << "\" is longer then the maximum supported size of " << MAX_OPTION_ARGUMENT_LENGTH
                  << " for option values." << std::endl;
        printHelpAndExit();
    }

    return doesOptionValueFitIntoString;
}

bool CommandLineArgumentParser::hasLexicallyValidOption(const char* value) const noexcept
{
    return doesOptionStartWithDash(value) && hasNonEmptyOptionName(value) && doesNotHaveLongOptionDash(value)
           && doesNotExceedLongOptionDash(value) && doesOptionNameFitIntoString(value);
}

CommandLineOptionValue CommandLineArgumentParser::parse(const CommandLineOptionSet& optionSet,
                                                        int argc,
                                                        char* argv[],
                                                        const uint64_t argcOffset,
                                                        const UnknownOption actionWhenOptionUnknown) noexcept
{
    m_optionSet = &optionSet;

    m_argc = static_cast<uint64_t>(algorithm::max(0, argc));
    m_argv = argv;
    m_argcOffset = argcOffset;
    // reset options otherwise multiple parse calls work on already parsed options
    m_optionValue = CommandLineOptionValue();

    if (!hasArguments(m_argc) || !assignBinaryName(m_argv[0]))
    {
        return m_optionValue;
    }

    for (uint64_t i = algorithm::max(argcOffset, static_cast<uint64_t>(1U)); i < m_argc; ++i)
    {
        const auto skipCommandLineArgument = [&] { ++i; };

        if (!hasLexicallyValidOption(m_argv[i]))
        {
            return m_optionValue;
        }

        uint64_t optionNameStart = (m_argv[i][1] == '-') ? 2 : 1;
        auto optionEntry = m_optionSet->getOption(OptionName_t(cxx::TruncateToCapacity, m_argv[i] + optionNameStart));

        if (!optionEntry)
        {
            switch (actionWhenOptionUnknown)
            {
            case UnknownOption::TERMINATE:
            {
                std::cout << "Unknown option \"" << m_argv[i] << "\"" << std::endl;
                printHelpAndExit();
                return m_optionValue;
            }
            case UnknownOption::IGNORE:
            {
                if (isNextArgumentAValue(i))
                {
                    skipCommandLineArgument();
                }
                continue;
            }
            }
        }

        if (isOptionSet(*optionEntry))
        {
            return m_optionValue;
        }

        if (optionEntry->details.type == OptionType::SWITCH)
        {
            m_optionValue.m_arguments.emplace_back(*optionEntry);
            m_optionValue.m_arguments.back().value.clear();
            m_optionValue.m_arguments.back().isSwitch = true;
        }
        else
        {
            if (!doesOptionHasSucceedingValue(*optionEntry, i))
            {
                return m_optionValue;
            }

            if (!doesOptionValueFitIntoString(m_argv[i + 1]))
            {
                return m_optionValue;
            }

            m_optionValue.m_arguments.emplace_back(*optionEntry);
            m_optionValue.m_arguments.back().value.unsafe_assign(m_argv[i + 1]);
            m_optionValue.m_arguments.back().isSwitch = false;
            skipCommandLineArgument();
        }
    }

    setDefaultValuesToUnsetOptions();

    if (m_optionValue.isSwitchSet("help") || !areAllRequiredValuesPresent())
    {
        printHelpAndExit();
        return m_optionValue;
    }

    return m_optionValue;
}

bool CommandLineArgumentParser::doesOptionHasSucceedingValue(const OptionWithDetails& value,
                                                             const uint64_t position) const noexcept
{
    bool doesOptionHasSucceedingValue = (position + 1 < m_argc);
    if (!doesOptionHasSucceedingValue)
    {
        std::cout << "The option \"" << value << "\" must be followed by a value!" << std::endl;
        printHelpAndExit();
    }
    return doesOptionHasSucceedingValue;
}


void CommandLineArgumentParser::setDefaultValuesToUnsetOptions() noexcept
{
    for (const auto& r : m_optionSet->m_availableOptions)
    {
        if (r.details.type != OptionType::OPTIONAL)
        {
            continue;
        }

        bool isOptionAlreadySet = false;
        for (auto& option : m_optionValue.m_arguments)
        {
            if (option.isSameOption(r))
            {
                isOptionAlreadySet = true;
                break;
            }
        }

        if (!isOptionAlreadySet)
        {
            m_optionValue.m_arguments.emplace_back(r);
        }
    }
}

bool CommandLineArgumentParser::areAllRequiredValuesPresent() const noexcept
{
    bool areAllRequiredValuesPresent = true;
    for (const auto& r : m_optionSet->m_availableOptions)
    {
        if (r.details.type == OptionType::REQUIRED)
        {
            bool isValuePresent = false;
            for (const auto& o : m_optionValue.m_arguments)
            {
                if (o.isSameOption(r))
                {
                    isValuePresent = true;
                    break;
                }
            }
            if (!isValuePresent)
            {
                std::cout << "Required option \"" << r << "\" is unset!" << std::endl;
                areAllRequiredValuesPresent = false;
            }
        }
    }

    return areAllRequiredValuesPresent;
}

void CommandLineArgumentParser::printHelpAndExit() const noexcept
{
    std::cout << "\n" << m_optionSet->m_programDescription << "\n" << std::endl;
    std::cout << "Usage: ";
    for (uint64_t i = 0; i < m_argcOffset && i < m_argc; ++i)
    {
        std::cout << m_argv[i] << " ";
    }
    std::cout << "[OPTIONS]\n" << std::endl;

    std::cout << "  Options:" << std::endl;

    auto sortedAvailableOptions = m_optionSet->m_availableOptions;
    std::sort(sortedAvailableOptions.begin(), sortedAvailableOptions.end());

    for (const auto& a : sortedAvailableOptions)
    {
        uint64_t outLength = 4U;
        std::cout << "    ";
        if (a.hasShortOption())
        {
            std::cout << "-" << a.shortOption;
            outLength += 2;
        }

        if (a.hasShortOption() && a.hasLongOption())
        {
            std::cout << ", ";
            outLength += 2;
        }

        if (a.hasLongOption())
        {
            std::cout << "--" << a.longOption.c_str();
            outLength += 2 + a.longOption.size();
        }

        if (a.details.type == OptionType::REQUIRED)
        {
            std::cout << " [" << a.details.typeName << "]";
            outLength += 3 + a.details.typeName.size();
        }
        else if (a.details.type == OptionType::OPTIONAL)
        {
            std::cout << " [" << a.details.typeName << "]";
            outLength += 3 + a.details.typeName.size();
        }

        uint64_t spacing = (outLength + 1 < OPTION_OUTPUT_WIDTH) ? OPTION_OUTPUT_WIDTH - outLength : 2;

        for (uint64_t i = 0; i < spacing; ++i)
        {
            std::cout << " ";
        }
        std::cout << a.details.description << std::endl;

        if (a.details.type == OptionType::OPTIONAL)
        {
            for (uint64_t i = 0; i < OPTION_OUTPUT_WIDTH; ++i)
            {
                std::cout << " ";
            }
            std::cout << "default value = \'" << a.value << "\'" << std::endl;
        }
    }
    std::cout << std::endl;
    m_optionSet->m_onFailureCallback();
}

} // namespace internal
} // namespace cli
} // namespace iox
