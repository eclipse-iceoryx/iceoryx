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

#include "iceoryx_hoofs/internal/cli/command_line_parser.hpp"
#include "iceoryx_hoofs/cxx/algorithm.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include <algorithm>

namespace iox
{
namespace cli
{
bool CommandLineParser::hasArguments(const int argc) const noexcept
{
    const bool hasArguments = (argc > 0);
    if (!hasArguments)
    {
        printHelpAndExit();
    }
    return hasArguments;
}

bool CommandLineParser::assignBinaryName(const char* name) noexcept
{
    const bool binaryNameFitsIntoString =
        (strnlen(name, platform::IOX_MAX_PATH_LENGTH + 1) <= platform::IOX_MAX_PATH_LENGTH);
    if (!binaryNameFitsIntoString)
    {
        std::cout << "The \"" << name << "\" binary path is too long" << std::endl;
        printHelpAndExit();
        return binaryNameFitsIntoString;
    }
    m_optionValue.m_binaryName.unsafe_assign(name);
    return binaryNameFitsIntoString;
}

bool CommandLineParser::doesOptionStartWithMinus(const char* option) const noexcept
{
    const bool doesOptionStartWithMinus = (strnlen(option, MAX_OPTION_NAME_LENGTH) > 0 && option[0] == '-');

    if (!doesOptionStartWithMinus)
    {
        std::cout << "Every option has to start with \"-\" but \"" << option << "\" does not." << std::endl;
        printHelpAndExit();
    }
    return doesOptionStartWithMinus;
}

bool CommandLineParser::hasOptionName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, MAX_OPTION_NAME_LENGTH);
    const bool hasOptionName = !(argIdentifierLength == 1 || (argIdentifierLength == 2 && option[1] == '-'));

    if (!hasOptionName)
    {
        std::cout << "Empty option names are forbidden" << std::endl;
        printHelpAndExit();
    }

    return hasOptionName;
}

bool CommandLineParser::hasValidSwitchName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, MAX_OPTION_NAME_LENGTH);
    const bool hasValidSwitchName = !(argIdentifierLength > 2 && option[1] != '-');

    if (!hasValidSwitchName)
    {
        std::cout << "Only one letter allowed when using a short option name. The switch \"" << option
                  << "\" is not valid." << std::endl;
        printHelpAndExit();
    }
    return hasValidSwitchName;
}

bool CommandLineParser::hasValidOptionName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, MAX_OPTION_NAME_LENGTH + 1);
    const bool hasValidOptionName = !(argIdentifierLength > 2 && option[2] == '-');

    if (!hasValidOptionName)
    {
        std::cout << "A long option name should start after \"--\". This \"" << option << "\" is not valid."
                  << std::endl;
        printHelpAndExit();
    }
    return hasValidOptionName;
}

bool CommandLineParser::doesOptionNameFitIntoString(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, MAX_OPTION_NAME_LENGTH + 1);
    const bool doesOptionNameFitIntoString = (argIdentifierLength <= MAX_OPTION_NAME_LENGTH);

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
    return (m_argc > 0 && static_cast<uint64_t>(m_argc) > position + 1
            && (strnlen(m_argv[position + 1], MAX_OPTION_NAME_LENGTH) > 0 && m_argv[position + 1][0] != '-'));
}

bool CommandLineParser::isOptionSet(const CommandLineOptionSet::Value& value) const noexcept
{
    bool isOptionSet = false;
    for (const auto& option : m_optionValue.m_arguments)
    {
        if ((option.shortId != CommandLineOptionSet::NO_SHORT_OPTION && option.shortId == value.shortOption)
            || option.id == value.longOption)
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
    const bool doesOptionValueFitIntoString =
        strnlen(value, MAX_OPTION_ARGUMENT_LENGTH + 1) <= MAX_OPTION_ARGUMENT_LENGTH;

    if (!doesOptionValueFitIntoString)
    {
        std::cout << "\"" << value << "\" is longer then the maximum supported size of " << MAX_OPTION_ARGUMENT_LENGTH
                  << " for option values." << std::endl;
        printHelpAndExit();
    }

    return doesOptionValueFitIntoString;
}

void CommandLineOptionSet::sortAvailableOptions() noexcept
{
    std::sort(m_availableOptions.begin(), m_availableOptions.end(), [](const Value& lhs, const Value& rhs) {
        if (lhs.shortOption != NO_SHORT_OPTION && rhs.shortOption != NO_SHORT_OPTION)
        {
            return lhs.shortOption < rhs.shortOption;
        }
        else if (!lhs.longOption.empty() && rhs.shortOption != NO_SHORT_OPTION)
        {
            return lhs.longOption.c_str()[0] < rhs.shortOption;
        }
        else if (lhs.shortOption != NO_SHORT_OPTION && !rhs.longOption.empty())
        {
            return lhs.shortOption < rhs.longOption.c_str()[0];
        }

        return lhs.longOption < rhs.longOption;
    });
}

CommandLineOptionValue CommandLineParser::parse(const CommandLineOptionSet& optionSet,
                                                int argc,
                                                char* argv[],
                                                const uint64_t argcOffset,
                                                const UnknownOption actionWhenOptionUnknown) noexcept
{
    m_optionSet = &optionSet;

    m_argc = argc;
    m_argv = argv;
    m_argcOffset = argcOffset;
    // reset options otherwise multiple parse calls work on already parsed options
    m_optionValue = CommandLineOptionValue();

    if (!hasArguments(argc) || !assignBinaryName(argv[0]))
    {
        return m_optionValue;
    }

    for (uint64_t i = algorithm::max(argcOffset, static_cast<uint64_t>(1U)); i < static_cast<uint64_t>(argc); ++i)
    {
        const auto skipCommandLineArgument = [&] { ++i; };

        if (!doesOptionStartWithMinus(argv[i]) || !hasOptionName(argv[i]) || !hasValidSwitchName(argv[i])
            || !hasValidOptionName(argv[i]) || !doesOptionNameFitIntoString(argv[i]))
        {
            return m_optionValue;
        }

        uint64_t optionNameStart = (argv[i][1] == '-') ? 2 : 1;
        auto optionEntry = m_optionSet->getOption(OptionName_t(cxx::TruncateToCapacity, argv[i] + optionNameStart));

        if (!optionEntry)
        {
            switch (actionWhenOptionUnknown)
            {
            case UnknownOption::TERMINATE:
            {
                std::cout << "Unknown option \"" << argv[i] << "\"" << std::endl;
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
                break;
            }
            }
        }

        if (isOptionSet(*optionEntry))
        {
            return m_optionValue;
        }

        if (optionEntry->type == OptionType::SWITCH)
        {
            m_optionValue.m_arguments.emplace_back();
            m_optionValue.m_arguments.back().id.unsafe_assign(optionEntry->longOption);
            m_optionValue.m_arguments.back().shortId = optionEntry->shortOption;
        }
        else
        {
            if (!doesOptionHasSucceedingValue(*optionEntry, i))
            {
                return m_optionValue;
            }

            if (!doesOptionValueFitIntoString(argv[i + 1]))
            {
                return m_optionValue;
            }

            m_optionValue.m_arguments.emplace_back();
            m_optionValue.m_arguments.back().id.unsafe_assign(optionEntry->longOption);
            m_optionValue.m_arguments.back().shortId = optionEntry->shortOption;
            m_optionValue.m_arguments.back().value.unsafe_assign(argv[i + 1]);
            skipCommandLineArgument();
        }
    }

    setDefaultValuesToUnsetOptions();

    if (m_optionValue.has("help") || !areAllRequiredValuesPresent())
    {
        printHelpAndExit();
        return m_optionValue;
    }

    return m_optionValue;
}

bool CommandLineParser::doesOptionHasSucceedingValue(const CommandLineOptionSet::Value& value,
                                                     const uint64_t position) const noexcept
{
    bool doesOptionHasSucceedingValue = (position + 1 < static_cast<uint64_t>(m_argc));
    if (!doesOptionHasSucceedingValue)
    {
        std::cout << "The option \"" << value << "\" must be followed by a value!" << std::endl;
        printHelpAndExit();
    }
    return doesOptionHasSucceedingValue;
}


void CommandLineParser::setDefaultValuesToUnsetOptions() noexcept
{
    for (const auto& r : m_optionSet->m_availableOptions)
    {
        if (r.type != OptionType::OPTIONAL)
        {
            continue;
        }

        bool isOptionAlreadySet = false;
        for (auto& option : m_optionValue.m_arguments)
        {
            if (option.shortId == r.shortOption)
            {
                isOptionAlreadySet = true;
                break;
            }
        }

        if (!isOptionAlreadySet)
        {
            m_optionValue.m_arguments.emplace_back();
            m_optionValue.m_arguments.back().id.unsafe_assign(r.longOption);
            m_optionValue.m_arguments.back().shortId = r.shortOption;
            m_optionValue.m_arguments.back().value = r.defaultValue;
        }
    }
}

bool CommandLineParser::areAllRequiredValuesPresent() const noexcept
{
    bool areAllRequiredValuesPresent = true;
    for (const auto& r : m_optionSet->m_availableOptions)
    {
        if (r.type == OptionType::REQUIRED)
        {
            bool isValuePresent = false;
            for (const auto& o : m_optionValue.m_arguments)
            {
                if (o.id == r.longOption || (o.id.size() == 1 && o.id.c_str()[0] == r.shortOption))
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

void CommandLineParser::printHelpAndExit() const noexcept
{
    std::cout << "\n" << m_optionSet->m_programDescription << "\n" << std::endl;
    std::cout << "Usage: ";
    for (uint64_t i = 0; i < m_argcOffset && i < static_cast<uint64_t>(m_argc); ++i)
    {
        std::cout << m_argv[i] << " ";
    }
    std::cout << "[OPTIONS]\n" << std::endl;

    std::cout << "  Options:" << std::endl;

    for (const auto& a : m_optionSet->m_availableOptions)
    {
        uint64_t outLength = 4U;
        std::cout << "    ";
        if (a.shortOption != CommandLineOptionSet::NO_SHORT_OPTION)
        {
            std::cout << "-" << a.shortOption;
            outLength += 2;
        }

        if (a.shortOption != CommandLineOptionSet::NO_SHORT_OPTION && !a.longOption.empty())
        {
            std::cout << ", ";
            outLength += 2;
        }

        if (!a.longOption.empty())
        {
            std::cout << "--" << a.longOption.c_str();
            outLength += 2 + a.longOption.size();
        }

        if (a.type == OptionType::REQUIRED)
        {
            std::cout << " [" << a.typeName << "]";
            outLength += 3 + a.typeName.size();
        }
        else if (a.type == OptionType::OPTIONAL)
        {
            std::cout << " [" << a.typeName << "]";
            outLength += 3 + a.typeName.size();
        }

        uint64_t spacing = (outLength + 1 < OPTION_OUTPUT_WIDTH) ? OPTION_OUTPUT_WIDTH - outLength : 2;

        for (uint64_t i = 0; i < spacing; ++i)
        {
            std::cout << " ";
        }
        std::cout << a.description << std::endl;

        if (a.type == OptionType::OPTIONAL)
        {
            for (uint64_t i = 0; i < OPTION_OUTPUT_WIDTH; ++i)
            {
                std::cout << " ";
            }
            std::cout << "default value = \'" << a.defaultValue << "\'" << std::endl;
        }
    }
    std::cout << std::endl;
    m_optionSet->m_onFailureCallback();
}


const BinaryName_t& CommandLineOptionValue::binaryName() const noexcept
{
    return m_binaryName;
}

bool CommandLineOptionValue::has(const OptionName_t& switchName) const noexcept
{
    for (const auto& a : m_arguments)
    {
        if (a.value.empty() && (a.id == switchName || (switchName.size() == 1 && a.shortId == switchName.c_str()[0])))
        {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& stream, const CommandLineOptionSet::Value& value) noexcept
{
    if (value.shortOption != CommandLineOptionSet::NO_SHORT_OPTION)
    {
        stream << "-" << value.shortOption;
    }
    if (value.shortOption != CommandLineOptionSet::NO_SHORT_OPTION && !value.longOption.empty())
    {
        stream << ", ";
    }
    if (!value.longOption.empty())
    {
        stream << "--" << value.longOption;
    }

    return stream;
}


//////////////////////////////
/// BEGIN CommandLineOptionSet
//////////////////////////////
CommandLineOptionSet::CommandLineOptionSet(const OptionDescription_t& programDescription,
                                           const cxx::function<void()> onFailureCallback) noexcept
    : m_programDescription{programDescription}
    , m_onFailureCallback{(onFailureCallback) ? onFailureCallback : [] { std::exit(EXIT_FAILURE); }}
{
    std::move(*this).addOption({'h', {"help"}, {"Display help."}, OptionType::SWITCH, {""}, {""}});
}

cxx::optional<CommandLineOptionSet::Value> CommandLineOptionSet::getOption(const OptionName_t& name) const noexcept
{
    const auto nameSize = name.size();
    for (const auto& r : m_availableOptions)
    {
        if (name == r.longOption || (nameSize == 1 && name.c_str()[0] == r.shortOption))
        {
            return r;
        }
    }
    return cxx::nullopt;
}

CommandLineOptionSet& CommandLineOptionSet::addOption(const Value& option) noexcept
{
    if (option.longOption.empty() && option.shortOption == NO_SHORT_OPTION)
    {
        std::cout << "Unable to add option with empty short and long option." << std::endl;
        m_onFailureCallback();
        return *this;
    }

    if (!option.longOption.empty() && option.longOption.c_str()[0] == '-')
    {
        std::cout << "The first character of a long option cannot start with minus \"-\" but the option \""
                  << option.longOption << "\" starts with minus." << std::endl;
        m_onFailureCallback();
        return *this;
    }

    if (option.shortOption == '-')
    {
        std::cout << "Minus \"-\" is not a valid character for a short option." << std::endl;
        m_onFailureCallback();
        return *this;
    }

    for (const auto& registeredOption : m_availableOptions)
    {
        bool isLongOrShortOptionRegistered = false;
        if (registeredOption.longOption == option.longOption)
        {
            std::cout << "The longOption \"--" << registeredOption.longOption << "\" is already registered for option "
                      << registeredOption << ". Cannot add option \"" << option << "\"." << std::endl;
            isLongOrShortOptionRegistered = true;
        }

        if (registeredOption.shortOption == option.shortOption)
        {
            std::cout << "The shortOption \"-" << registeredOption.shortOption << "\" is already registered for option "
                      << registeredOption << ". Cannot add option \"" << option << "\"." << std::endl;
            isLongOrShortOptionRegistered = true;
        }

        if (isLongOrShortOptionRegistered)
        {
            m_onFailureCallback();
            return *this;
        }
    }

    m_availableOptions.emplace_back(option);

    /// sort options so that they are alphabetically sorted in help output
    sortAvailableOptions();

    return *this;
}

CommandLineOptionSet& CommandLineOptionSet::addSwitch(const char shortOption,
                                                      const OptionName_t& longOption,
                                                      const OptionDescription_t& description) noexcept
{
    return addOption({shortOption, longOption, description, OptionType::SWITCH, {""}, {""}});
}

CommandLineOptionSet& CommandLineOptionSet::addOptional(const char shortOption,
                                                        const OptionName_t& longOption,
                                                        const OptionDescription_t& description,
                                                        const TypeName_t& typeName,
                                                        const Argument_t& defaultValue) noexcept
{
    return addOption({shortOption, longOption, description, OptionType::OPTIONAL, typeName, defaultValue});
}
CommandLineOptionSet& CommandLineOptionSet::addMandatory(const char shortOption,
                                                         const OptionName_t& longOption,
                                                         const OptionDescription_t& description,
                                                         const TypeName_t& typeName) noexcept
{
    return addOption({shortOption, longOption, description, OptionType::REQUIRED, typeName, {""}});
}


//////////////////////////////
/// END CommandLineOptionSet
//////////////////////////////

} // namespace cli
} // namespace iox
