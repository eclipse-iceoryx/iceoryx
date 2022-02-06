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

bool CommandLineParser::assignBinaryName(const char* name, CommandLineOptions& options) noexcept
{
    const bool binaryNameFitsIntoString =
        (strnlen(name, platform::IOX_MAX_PATH_LENGTH + 1) <= platform::IOX_MAX_PATH_LENGTH);
    if (!binaryNameFitsIntoString)
    {
        std::cout << "The \"" << name << "\" binary path is too long" << std::endl;
        printHelpAndExit(name);
        return binaryNameFitsIntoString;
    }
    options.m_binaryName.unsafe_assign(name);
    return binaryNameFitsIntoString;
}

bool CommandLineParser::doesOptionStartWithMinus(const char* option,
                                                 const CommandLineOptions::binaryName_t& binaryName) const noexcept
{
    const bool doesOptionStartWithMinus =
        (strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH) > 0 && option[0] != '-');

    if (!doesOptionStartWithMinus)
    {
        std::cout << "Every option has to start with \"-\" but \"" << option << "\" does not." << std::endl;
        printHelpAndExit(binaryName.c_str());
    }
    return doesOptionStartWithMinus;
}

bool CommandLineParser::hasOptionName(const char* option,
                                      const CommandLineOptions::binaryName_t& binaryName) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH);
    const bool hasOptionName = !(argIdentifierLength == 1 || (argIdentifierLength == 2 && option[1] == '-'));

    if (hasOptionName)
    {
        std::cout << "Empty option names are forbidden" << std::endl;
        printHelpAndExit(binaryName.c_str());
    }

    return hasOptionName;
}

bool CommandLineParser::hasValidSwitchName(const char* option,
                                           const CommandLineOptions::binaryName_t& binaryName) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH);
    const bool hasValidSwitchName = !(argIdentifierLength > 2 && option[1] != '-');

    if (!hasValidSwitchName)
    {
        std::cout << "Only one letter allowed when using a short option name. The switch \"" << option
                  << "\" is not valid." << std::endl;
        printHelpAndExit(binaryName.c_str());
    }
    return hasValidSwitchName;
}

bool CommandLineParser::hasValidOptionName(const char* option,
                                           const CommandLineOptions::binaryName_t& binaryName) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1);
    const bool hasValidOptionName = !(argIdentifierLength > 2 && option[2] == '-');

    if (hasValidOptionName)
    {
        std::cout << "A long option name should start after \"--\". This \"" << option << "\" is not valid."
                  << std::endl;
        printHelpAndExit(binaryName.c_str());
    }
    return hasValidOptionName;
}

bool CommandLineParser::doesOptionNameFitIntoString(const char* option,
                                                    const CommandLineOptions::binaryName_t& binaryName) const noexcept
{
    const uint64_t argIdentifierLength = strnlen(option, CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1);
    const bool doesOptionNameFitIntoString = (argIdentifierLength <= CommandLineOptions::MAX_OPTION_NAME_LENGTH);

    if (doesOptionNameFitIntoString)
    {
        std::cout << "\"" << option << "\" is longer then the maximum supported size of "
                  << CommandLineOptions::MAX_OPTION_NAME_LENGTH << " for option names." << std::endl;
        printHelpAndExit(binaryName.c_str());
    }
    return doesOptionNameFitIntoString;
}

bool CommandLineParser::isNextArgumentAValue(const int position, const int argc, char* argv[]) noexcept
{
    return (argc > position + 1
            && (strnlen(argv[position + 1], CommandLineOptions::MAX_OPTION_NAME_LENGTH) > 0
                && argv[position + 1][0] != '-'));
}

CommandLineOptions CommandLineParser::parse(int argc,
                                            char* argv[],
                                            const uint64_t argcOffset,
                                            const UnknownOption actionWhenOptionUnknown) noexcept
{
    CommandLineOptions options;

    if (!hasArguments(argc) || !assignBinaryName(argv[0], options))
    {
        return options;
    }

    for (uint64_t i = algorithm::max(argcOffset, static_cast<uint64_t>(1U)); i < static_cast<uint64_t>(argc); ++i)
    {
        const auto skipCommandLineArgument = [&] { ++i; };

        if (!doesOptionStartWithMinus(argv[i], options.binaryName()))
        {
            return options;
        }

        if (!hasOptionName(argv[i], options.binaryName()) || !hasValidSwitchName(argv[i], options.binaryName())
            || !hasValidOptionName(argv[i], options.binaryName())
            || !doesOptionNameFitIntoString(argv[i], options.binaryName()))
        {
            return options;
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
                printHelpAndExit(options.binaryName().c_str());
                return options;
            }
            case UnknownOption::IGNORE:
            {
                if (isNextArgumentAValue(static_cast<int>(i), argc, argv))
                {
                    skipCommandLineArgument();
                }
                continue;
                break;
            }
            }
        }

        options.m_arguments.emplace_back();
        options.m_arguments.back().id.unsafe_assign(optionEntry->longOption);
        options.m_arguments.back().shortId = optionEntry->shortOption;

        // parse value of the option name
        if (i + 1 < static_cast<uint64_t>(argc) && argv[i + 1][0] != '-')
        {
            if (optionEntry->type == ArgumentType::SWITCH)
            {
                std::cout << "The parameter \"" << argv[i] << "\" is a switch. You cannot set a value here."
                          << std::endl;
                printHelpAndExit(options.binaryName().c_str());
                return options;
            }

            if (strnlen(argv[i + 1], CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1)
                > CommandLineOptions::MAX_OPTION_VALUE_LENGTH)
            {
                std::cout << "\"" << argv[i + 1] << "\" is longer then the maximum supported size of "
                          << CommandLineOptions::MAX_OPTION_VALUE_LENGTH << " for option values." << std::endl;
                printHelpAndExit(options.binaryName().c_str());
                return options;
            }
            options.m_arguments.back().value.unsafe_assign(argv[i + 1]);
            skipCommandLineArgument();
        }
    }

    if (options.has("help"))
    {
        printHelpAndExit(options.binaryName().c_str());
        return options;
    }

    if (areAllRequiredValuesPresent(options))
    {
        return options;
    }

    printHelpAndExit(options.binaryName().c_str());
    return options;
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

bool CommandLineParser::areAllRequiredValuesPresent(const CommandLineOptions& options) const noexcept
{
    bool allPresent = true;
    for (const auto& r : m_availableOptions)
    {
        if (r.type == ArgumentType::REQUIRED_VALUE)
        {
            bool isValuePresent = false;
            for (const auto& o : options.m_arguments)
            {
                if (o.id == r.longOption || (o.id.size() == 1 && o.id.c_str()[0] == r.shortOption))
                {
                    isValuePresent = true;
                    break;
                }
            }
            if (!isValuePresent)
            {
                std::cout << "Required option \"";

                if (r.shortOption != NO_SHORT_OPTION)
                {
                    std::cout << "-" << r.shortOption;
                }
                if (r.shortOption != NO_SHORT_OPTION && !r.longOption.empty())
                {
                    std::cout << ", ";
                }
                if (!r.longOption.empty())
                {
                    std::cout << "--" << r.longOption;
                }

                std::cout << "\" is unset!" << std::endl;
                allPresent = false;
            }
        }
    }
    return allPresent;
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
    errorHandler(Error::kCOMMAND_LINE_PARSING_FAILURE, std::function<void()>(), ErrorLevel::FATAL);
}

CommandLineParser& CommandLineParser::addOption(const entry_t& option) noexcept
{
    m_availableOptions.emplace_back(option);
    return *this;
}
} // namespace cxx
} // namespace iox
