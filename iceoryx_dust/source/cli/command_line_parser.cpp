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

#include "iceoryx_dust/internal/cli/command_line_parser.hpp"
#include "iceoryx_hoofs/cxx/algorithm.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include <algorithm>

namespace iox
{
namespace cli
{
namespace internal
{
Arguments
parseCommandLineArguments(const OptionDefinition& optionSet, int argc, char* argv[], const uint64_t argcOffset) noexcept
{
    return CommandLineParser().parse(optionSet, argc, argv, argcOffset);
}

bool CommandLineParser::hasArguments(const uint64_t argc) const noexcept
{
    const bool hasArguments = (argc > 0);
    if (!hasArguments)
    {
        printHelpAndExit();
    }
    return hasArguments;
}

bool CommandLineParser::doesOptionStartWithDash(const char* option) const noexcept
{
    const bool doesOptionStartWithDash = (strnlen(option, 1) > 0 && option[0] == '-');

    if (!doesOptionStartWithDash)
    {
        std::cout << "Every option has to start with \"-\" but \"" << option << "\" does not." << std::endl;
        printHelpAndExit();
    }
    return doesOptionStartWithDash;
}

bool CommandLineParser::hasNonEmptyOptionName(const char* option) const noexcept
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

bool CommandLineParser::doesNotHaveLongOptionDash(const char* option) const noexcept
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

bool CommandLineParser::doesNotExceedLongOptionDash(const char* option) const noexcept
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

bool CommandLineParser::doesFitIntoString(const char* value, const uint64_t maxLength) const noexcept
{
    return (strnlen(value, maxLength + 1) <= maxLength);
}

bool CommandLineParser::doesOptionNameFitIntoString(const char* option) const noexcept
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

bool CommandLineParser::isNextArgumentAValue(const uint64_t position) const noexcept
{
    uint64_t nextPosition = position + 1;
    return (m_argc > 0 && m_argc > nextPosition
            && (strnlen(m_argv[nextPosition], MAX_OPTION_NAME_LENGTH) > 0 && m_argv[nextPosition][0] != '-'));
}

bool CommandLineParser::isOptionSet(const OptionWithDetails& value) const noexcept
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

bool CommandLineParser::doesOptionValueFitIntoString(const char* value) const noexcept
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

bool CommandLineParser::hasLexicallyValidOption(const char* value) const noexcept
{
    return doesOptionStartWithDash(value) && hasNonEmptyOptionName(value) && doesNotHaveLongOptionDash(value)
           && doesNotExceedLongOptionDash(value) && doesOptionNameFitIntoString(value);
}

Arguments
CommandLineParser::parse(const OptionDefinition& optionSet, int argc, char* argv[], const uint64_t argcOffset) noexcept
{
    m_optionSet = &optionSet;

    m_argc = static_cast<uint64_t>(algorithm::maxVal(0, argc));
    m_argv = argv;
    m_argcOffset = argcOffset;
    // reset options otherwise multiple parse calls work on already parsed options
    m_optionValue = Arguments();

    if (!hasArguments(m_argc))
    {
        return m_optionValue;
    }

    m_optionValue.m_binaryName = m_argv[0];

    for (uint64_t i = algorithm::maxVal(argcOffset, static_cast<uint64_t>(1U)); i < m_argc; ++i)
    {
        const auto skipCommandLineArgument = [&] { ++i; };

        if (!hasLexicallyValidOption(m_argv[i]))
        {
            return m_optionValue;
        }

        uint64_t optionNameStart = (m_argv[i][1] == '-') ? 2 : 1;
        auto optionEntry = m_optionSet->getOption(OptionName_t(TruncateToCapacity, m_argv[i] + optionNameStart));

        if (!optionEntry)
        {
            std::cout << "Unknown option \"" << m_argv[i] << "\"" << std::endl;
            printHelpAndExit();
            return m_optionValue;
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

bool CommandLineParser::doesOptionHasSucceedingValue(const OptionWithDetails& value,
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


void CommandLineParser::setDefaultValuesToUnsetOptions() noexcept // rename
{
    for (const auto& availableOption : m_optionSet->m_availableOptions)
    {
        if (availableOption.details.type != OptionType::OPTIONAL)
        {
            continue;
        }

        bool isOptionAlreadySet = false;
        for (auto& option : m_optionValue.m_arguments)
        {
            if (option.isSameOption(availableOption))
            {
                isOptionAlreadySet = true;
                break;
            }
        }

        if (!isOptionAlreadySet)
        {
            m_optionValue.m_arguments.emplace_back(availableOption);
        }
    }
}

bool CommandLineParser::areAllRequiredValuesPresent() const noexcept
{
    bool areAllRequiredValuesPresent = true;
    for (const auto& availableOption : m_optionSet->m_availableOptions)
    {
        if (availableOption.details.type == OptionType::REQUIRED)
        {
            bool isValuePresent = false;
            for (const auto& option : m_optionValue.m_arguments)
            {
                if (option.isSameOption(availableOption))
                {
                    isValuePresent = true;
                    break;
                }
            }
            if (!isValuePresent)
            {
                std::cout << "Required option \"" << availableOption << "\" is unset!" << std::endl;
                areAllRequiredValuesPresent = false;
            }
        }
    }

    return areAllRequiredValuesPresent;
}

void CommandLineParser::printHelpAndExit() const noexcept
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

    for (const auto& option : sortedAvailableOptions)
    {
        uint64_t outLength = 4U;
        std::cout << "    ";
        if (option.hasShortOption())
        {
            std::cout << "-" << option.shortOption;
            outLength += 2;
        }

        if (option.hasShortOption() && option.hasLongOption())
        {
            std::cout << ", ";
            outLength += 2;
        }

        if (option.hasLongOption())
        {
            std::cout << "--" << option.longOption.c_str();
            outLength += 2 + option.longOption.size();
        }

        if (option.details.type == OptionType::REQUIRED)
        {
            std::cout << " [" << option.details.typeName << "]";
            outLength += 3 + option.details.typeName.size();
        }
        else if (option.details.type == OptionType::OPTIONAL)
        {
            std::cout << " [" << option.details.typeName << "]";
            outLength += 3 + option.details.typeName.size();
        }

        uint64_t spacing = (outLength + 1 < OPTION_OUTPUT_WIDTH) ? OPTION_OUTPUT_WIDTH - outLength : 2;

        for (uint64_t i = 0; i < spacing; ++i)
        {
            std::cout << " ";
        }
        std::cout << option.details.description << std::endl;

        if (option.details.type == OptionType::OPTIONAL)
        {
            for (uint64_t i = 0; i < OPTION_OUTPUT_WIDTH; ++i)
            {
                std::cout << " ";
            }
            std::cout << "default value = \'" << option.value << "\'" << std::endl;
        }
    }
    std::cout << std::endl;
    m_optionSet->m_onFailureCallback();
}

} // namespace internal
} // namespace cli
} // namespace iox
