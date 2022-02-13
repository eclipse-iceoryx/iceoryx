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

#include "iceoryx_hoofs/internal/cxx/command_line_parser.hpp"
#include "iceoryx_hoofs/cxx/algorithm.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

namespace iox
{
namespace cxx
{
CommandLineParser::CommandLineParser(const description_t& programDescription) noexcept
    : m_programDescription{programDescription}
{
    std::move(*this).addOption({'h', "help", "Display help.", ArgumentType::SWITCH, "", ""});
}

bool CommandLineParser::hasArguments(const int argc) const noexcept
{
    const bool hasArguments = (argc > 0);
    if (!hasArguments)
    {
        printHelpAndExit("empty");
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
        printHelpAndExit(name);
        return binaryNameFitsIntoString;
    }
    m_options.m_binaryName.unsafe_assign(name);
    return binaryNameFitsIntoString;
}

bool CommandLineParser::doesOptionStartWithMinus(const char* option) const noexcept
{
    const bool doesOptionStartWithMinus =
        (strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH) > 0 && option[0] == '-');

    if (!doesOptionStartWithMinus)
    {
        std::cout << "Every option has to start with \"-\" but \"" << option << "\" does not." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return doesOptionStartWithMinus;
}

bool CommandLineParser::hasOptionName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH);
    const bool hasOptionName = !(argIdentifierLength == 1 || (argIdentifierLength == 2 && option[1] == '-'));

    if (!hasOptionName)
    {
        std::cout << "Empty option names are forbidden" << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }

    return hasOptionName;
}

bool CommandLineParser::hasValidSwitchName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH);
    const bool hasValidSwitchName = !(argIdentifierLength > 2 && option[1] != '-');

    if (!hasValidSwitchName)
    {
        std::cout << "Only one letter allowed when using a short option name. The switch \"" << option
                  << "\" is not valid." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return hasValidSwitchName;
}

bool CommandLineParser::hasValidOptionName(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1);
    const bool hasValidOptionName = !(argIdentifierLength > 2 && option[2] == '-');

    if (!hasValidOptionName)
    {
        std::cout << "A long option name should start after \"--\". This \"" << option << "\" is not valid."
                  << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return hasValidOptionName;
}

bool CommandLineParser::doesOptionNameFitIntoString(const char* option) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1);
    const bool doesOptionNameFitIntoString = (argIdentifierLength <= CommandLineOptions::MAX_OPTION_NAME_LENGTH);

    if (!doesOptionNameFitIntoString)
    {
        std::cout << "\"" << option << "\" is longer then the maximum supported size of "
                  << CommandLineOptions::MAX_OPTION_NAME_LENGTH << " for option names." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return doesOptionNameFitIntoString;
}

bool CommandLineParser::isNextArgumentAValue(const uint64_t position) const noexcept
{
    return (m_argc > 0 && static_cast<uint64_t>(m_argc) > position + 1
            && (strnlen(m_argv[position + 1], CommandLineOptions::MAX_OPTION_NAME_LENGTH) > 0
                && m_argv[position + 1][0] != '-'));
}

bool CommandLineParser::isValueOptionFollowedByValue(const entry_t& entry,
                                                     const bool isNextArgumentAValue) const noexcept
{
    const bool isValueOptionFollowedByValue =
        !((entry.type == ArgumentType::OPTIONAL_VALUE || entry.type == ArgumentType::REQUIRED_VALUE)
          && !isNextArgumentAValue);

    if (!isValueOptionFollowedByValue)
    {
        std::cout << "The option \"" << entry << "\" requires a value but none were provided." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return isValueOptionFollowedByValue;
}

bool CommandLineParser::isOptionSet(const entry_t& entry) const noexcept
{
    bool isOptionSet = false;
    for (const auto& option : m_options.m_arguments)
    {
        if ((option.shortId != NO_SHORT_OPTION && option.shortId == entry.shortOption) || option.id == entry.longOption)
        {
            isOptionSet = true;
            break;
        }
    }

    if (isOptionSet)
    {
        std::cout << "The option \"" << entry << "\" is already set!" << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }

    return isOptionSet;
}

bool CommandLineParser::doesOptionValueFitIntoString(const char* value) const noexcept
{
    const bool doesOptionValueFitIntoString =
        strnlen(value, CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1) <= CommandLineOptions::MAX_OPTION_VALUE_LENGTH;

    if (!doesOptionValueFitIntoString)
    {
        std::cout << "\"" << value << "\" is longer then the maximum supported size of "
                  << CommandLineOptions::MAX_OPTION_VALUE_LENGTH << " for option values." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }

    return doesOptionValueFitIntoString;
}

bool CommandLineParser::failWhenEntryIsSwitch(const entry_t& entry, const char* nextArgument) const noexcept
{
    if (entry.type == ArgumentType::SWITCH)
    {
        std::cout << "The parameter \"" << entry << "\" is a switch. You cannot set the value \"" << nextArgument
                  << "\" here." << std::endl;
        printHelpAndExit(m_options.binaryName().c_str());
    }
    return entry.type == ArgumentType::SWITCH;
}

CommandLineOptions CommandLineParser::parse(int argc,
                                            char* argv[],
                                            const uint64_t argcOffset,
                                            const UnknownOption actionWhenOptionUnknown) noexcept
{
    m_argc = argc;
    m_argv = argv;

    if (!hasArguments(argc) || !assignBinaryName(argv[0]))
    {
        return m_options;
    }

    for (uint64_t i = algorithm::max(argcOffset, static_cast<uint64_t>(1U)); i < static_cast<uint64_t>(argc); ++i)
    {
        const auto skipCommandLineArgument = [&] { ++i; };

        if (!doesOptionStartWithMinus(argv[i]) || !hasOptionName(argv[i]) || !hasValidSwitchName(argv[i])
            || !hasValidOptionName(argv[i]) || !doesOptionNameFitIntoString(argv[i]))
        {
            return m_options;
        }

        uint64_t optionNameStart = (argv[i][1] == '-') ? 2 : 1;
        auto optionEntry = getOption(CommandLineOptions::name_t(cxx::TruncateToCapacity, argv[i] + optionNameStart));

        if (!optionEntry)
        {
            switch (actionWhenOptionUnknown)
            {
            case UnknownOption::TERMINATE:
            {
                std::cout << "Unknown option \"" << argv[i] << "\"" << std::endl;
                printHelpAndExit(m_options.binaryName().c_str());
                return m_options;
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

        if (!isValueOptionFollowedByValue(*optionEntry, isNextArgumentAValue(i)) || isOptionSet(*optionEntry))
        {
            return m_options;
        }

        m_options.m_arguments.emplace_back();
        m_options.m_arguments.back().id.unsafe_assign(optionEntry->longOption);
        m_options.m_arguments.back().shortId = optionEntry->shortOption;

        // parse value of the option name
        if (i + 1 < static_cast<uint64_t>(argc) && argv[i + 1][0] != '-')
        {
            if (failWhenEntryIsSwitch(*optionEntry, argv[i + 1]) || !doesOptionValueFitIntoString(argv[i + 1]))
            {
                return m_options;
            }

            m_options.m_arguments.back().value.unsafe_assign(argv[i + 1]);
            skipCommandLineArgument();
        }
    }

    if (m_options.has("help") || !areAllRequiredValuesPresent())
    {
        printHelpAndExit(m_options.binaryName().c_str());
        return m_options;
    }

    return m_options;
}

cxx::optional<CommandLineParser::entry_t>
CommandLineParser::getOption(const CommandLineOptions::name_t& name) const noexcept
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

bool CommandLineParser::areAllRequiredValuesPresent() const noexcept
{
    bool areAllRequiredValuesPresent = true;
    for (const auto& r : m_availableOptions)
    {
        if (r.type == ArgumentType::REQUIRED_VALUE)
        {
            bool isValuePresent = false;
            for (const auto& o : m_options.m_arguments)
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

const CommandLineOptions::binaryName_t& CommandLineOptions::binaryName() const noexcept
{
    return m_binaryName;
}

bool CommandLineOptions::has(const name_t& switchName) const noexcept
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

void CommandLineParser::printHelpAndExit(const char* binaryName) const noexcept
{
    std::cout << "\n" << m_programDescription << "\n" << std::endl;
    std::cout << "Usage: " << binaryName << " [OPTIONS]\n" << std::endl;
    std::cout << "  Options:" << std::endl;
    for (const auto& a : m_availableOptions)
    {
        uint64_t outLength = 4U;
        std::cout << "    ";
        if (a.shortOption != NO_SHORT_OPTION)
        {
            std::cout << "-" << a.shortOption;
            outLength += 2;
        }

        if (a.shortOption != NO_SHORT_OPTION && !a.longOption.empty())
        {
            std::cout << ", ";
            outLength += 2;
        }

        if (!a.longOption.empty())
        {
            std::cout << "--" << a.longOption.c_str();
            outLength += 2 + a.longOption.size();
        }

        if (a.type == ArgumentType::REQUIRED_VALUE)
        {
            std::cout << " [" << a.typeName << "]";
            outLength += 3 + a.typeName.size();
        }
        else if (a.type == ArgumentType::OPTIONAL_VALUE)
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

        if (a.type == ArgumentType::OPTIONAL_VALUE)
        {
            for (uint64_t i = 0; i < OPTION_OUTPUT_WIDTH; ++i)
            {
                std::cout << " ";
            }
            std::cout << "default value = \'" << a.defaultValue << "\'" << std::endl;
        }
    }
    std::cout << std::endl;
    errorHandler(Error::kCXX__COMMAND_LINE_PARSING_FAILURE, std::function<void()>(), ErrorLevel::FATAL);
}

CommandLineParser& CommandLineParser::addOption(const entry_t& option) noexcept
{
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
            errorHandler(
                Error::kCXX__COMMAND_LINE_OPTION_ALREADY_REGISTERED, std::function<void()>(), ErrorLevel::FATAL);
            return *this;
        }
    }
    m_availableOptions.emplace_back(option);
    return *this;
}

std::ostream& operator<<(std::ostream& stream, const CommandLineParser::entry_t& entry) noexcept
{
    if (entry.shortOption != CommandLineParser::NO_SHORT_OPTION)
    {
        stream << "-" << entry.shortOption;
    }
    if (entry.shortOption != CommandLineParser::NO_SHORT_OPTION && !entry.longOption.empty())
    {
        stream << ", ";
    }
    if (!entry.longOption.empty())
    {
        stream << "--" << entry.longOption;
    }

    return stream;
}
} // namespace cxx
} // namespace iox
