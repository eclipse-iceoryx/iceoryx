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

#include "iceoryx_hoofs/cxx/command_line_parser.hpp"

namespace iox
{
namespace cxx
{
CommandLineParser::CommandLineParser() noexcept
{
    std::move(*this).addOption({'h', "help", "Display help.", ArgumentType::SWITCH});
}

CommandLineOptions CommandLineParser::parse(int argc, char* argv[]) && noexcept
{
    CommandLineOptions options;
    for (uint64_t i = 0U; i < static_cast<uint64_t>(argc); ++i)
    {
        if (i == 0)
        {
            if (strnlen(argv[i], CommandLineOptions::MAX_BINARY_NAME_LENGTH + 1)
                > CommandLineOptions::MAX_BINARY_NAME_LENGTH)
            {
                std::cerr << "The \"" << argv[i] << "\" binary path is too long" << std::endl;
                printHelpAndExit(argv[0]);
            }
            options.m_binaryName.unsafe_assign(argv[i]);
        }
        else
        {
            if (argv[i][0] != '-')
            {
                std::cerr << "Every option has to start with \"-\" but \"" << argv[i] << "\" does not." << std::endl;
                printHelpAndExit(argv[0]);
            }

            uint64_t argIdentifierLength = strnlen(argv[i], CommandLineOptions::MAX_OPTION_NAME_LENGTH + 1);

            if (argIdentifierLength == 1 || (argIdentifierLength == 2 && argv[i][1] == '-'))
            {
                std::cerr << "Empty option names are forbidden" << std::endl;
                printHelpAndExit(argv[0]);
            }
            else if (argIdentifierLength > 2 && argv[i][1] != '-')
            {
                std::cerr << "Only one letter allowed when using a short option name. This \"" << argv[i]
                          << "\" is not valid." << std::endl;
                printHelpAndExit(argv[0]);
            }
            else if (argIdentifierLength > 2 && argv[i][2] == '-')
            {
                std::cerr << "A long option name should start after \"--\". This \"" << argv[i] << "\" is not valid."
                          << std::endl;
                printHelpAndExit(argv[0]);
            }
            else if (argIdentifierLength > CommandLineOptions::MAX_OPTION_NAME_LENGTH)
            {
                std::cerr << "\"" << argv[i] << "\" is longer then the maximum supported size of "
                          << CommandLineOptions::MAX_OPTION_NAME_LENGTH << " for option names." << std::endl;
                printHelpAndExit(argv[0]);
            }

            uint64_t optionNameStart = (argv[i][1] == '-') ? 2 : 1;
            auto optionEntry =
                getOption(CommandLineOptions::name_t(cxx::TruncateToCapacity, argv[i] + optionNameStart));

            if (!optionEntry)
            {
                std::cerr << "Unknown option \"" << argv[i] << "\"" << std::endl;
                printHelpAndExit(argv[0]);
            }

            options.m_arguments.emplace_back();
            options.m_arguments.back().id.unsafe_assign(optionEntry->longOption);
            options.m_arguments.back().shortId = optionEntry->shortOption;

            // parse value of the option name
            if (i + 1 < static_cast<uint64_t>(argc) && argv[i + 1][0] != '-')
            {
                if (optionEntry->type == ArgumentType::SWITCH)
                {
                    std::cerr << "The parameter \"" << argv[i] << "\" is a switch. You cannot set a value here."
                              << std::endl;
                    printHelpAndExit(argv[0]);
                }

                if (strnlen(argv[i + 1], CommandLineOptions::MAX_OPTION_VALUE_LENGTH + 1)
                    > CommandLineOptions::MAX_OPTION_VALUE_LENGTH)
                {
                    std::cerr << "\"" << argv[i + 1] << "\" is longer then the maximum supported size of "
                              << CommandLineOptions::MAX_OPTION_VALUE_LENGTH << " for option values." << std::endl;
                    printHelpAndExit(argv[0]);
                }
                options.m_arguments.back().value.unsafe_assign(argv[i + 1]);
                ++i;
            }
        }
    }

    if (areAllRequiredValuesPresent(options))
    {
        if (options.has("help"))
        {
            printHelpAndExit(argv[0]);
        }
        return options;
    }

    printHelpAndExit(argv[0]);
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
    std::cout << "\nUsage: " << binaryName << " [OPTIONS]\n" << std::endl;
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
            std::cout << " [REQUIRED_VALUE]";
            outLength += 17;
        }
        else if (a.type == ArgumentType::OPTIONAL_VALUE)
        {
            std::cout << " [OPTIONAL_VALUE]";
            outLength += 17;
        }

        uint64_t spacing = (outLength + 1 < OPTION_OUTPUT_WIDTH) ? OPTION_OUTPUT_WIDTH - outLength : 2;

        for (uint64_t i = 0; i < spacing; ++i)
        {
            std::cout << " ";
        }
        std::cout << a.description << std::endl;
    }
    std::cout << std::endl;
    std::exit(EXIT_FAILURE);
}

CommandLineParser&& CommandLineParser::addOption(const entry_t& option) && noexcept
{
    m_availableOptions.emplace_back(option);
    return std::move(*this);
}
} // namespace cxx
} // namespace iox
