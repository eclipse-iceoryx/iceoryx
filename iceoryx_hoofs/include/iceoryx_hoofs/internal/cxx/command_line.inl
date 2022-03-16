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

#ifndef IOX_HOOFS_CXX_COMMAND_LINE_INL
#define IOX_HOOFS_CXX_COMMAND_LINE_INL

#include "iceoryx_hoofs/cxx/command_line.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
template <typename T>
inline void extractValue(T& value, const cmdEntries_t& entries, const uint64_t index, const CommandLineOptions& options)
{
    auto result = options.get<T>(entries[index].longOption);
    if (result.has_error())
    {
        std::cout << "It seems that the option value of \"";
        const bool hasShortOption = (entries[index].shortOption != '\0');
        const bool hasLongOption = (!entries[index].longOption.empty());
        if (hasShortOption)
        {
            std::cout << "-" << entries[index].shortOption;
        }
        if (hasShortOption && hasLongOption)
        {
            std::cout << ", ";
        }
        if (hasLongOption)
        {
            std::cout << "--" << entries[index].longOption;
        }

        std::cout << "\" is not of type \"" << entries[index].typeName << "\"" << std::endl;
        std::terminate();
    }

    value = result.value();
}

template <typename T>
inline T addEntry(T& value,
                  const char shortName,
                  const CommandLineOptions::name_t& name,
                  const CommandLineParser::description_t& description,
                  const ArgumentType argumentType,
                  T defaultValue,
                  internal::cmdEntries_t& entries,
                  internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(
        CommandLineParser::entry_t{shortName,
                                   name,
                                   description,
                                   argumentType,
                                   {TypeInfo<T>::NAME},
                                   CommandLineOptions::value_t(TruncateToCapacity, convert::toString(defaultValue))});
    assignments.emplace_back([&value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        extractValue(value, entries, index, options);
    });
    return defaultValue;
}

template <>
inline bool addEntry(bool& value,
                     const char shortName,
                     const CommandLineOptions::name_t& name,
                     const CommandLineParser::description_t& description,
                     const ArgumentType argumentType,
                     bool defaultValue,
                     internal::cmdEntries_t& entries,
                     internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(
        CommandLineParser::entry_t{shortName,
                                   name,
                                   description,
                                   argumentType,
                                   {"true|false"},
                                   CommandLineOptions::value_t(TruncateToCapacity, (defaultValue) ? "true" : "false")});
    assignments.emplace_back([&value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        if (entries[index].type == ArgumentType::SWITCH)
        {
            value = options.has(entries[index].longOption);
        }
        else
        {
            extractValue(value, entries, index, options);
        }
    });
    return defaultValue;
}

inline void populateEntries(const cmdEntries_t& entries,
                            const cmdAssignments_t& assignments,
                            CommandLineOptions::binaryName_t& binaryName,
                            const CommandLineParser::description_t& programDescription,
                            int argc,
                            char* argv[],
                            const uint64_t argcOffset,
                            const UnknownOption actionWhenOptionUnknown,
                            const cxx::function<void()>& onFailureCallback)
{
    CommandLineParser parser(programDescription, onFailureCallback);
    for (const auto& entry : entries)
    {
        switch (entry.type)
        {
        case ArgumentType::SWITCH:
            parser.addSwitch(entry.shortOption, entry.longOption, entry.description);
            break;
        case ArgumentType::REQUIRED_VALUE:
            parser.addRequiredValue(entry.shortOption, entry.longOption, entry.description, entry.typeName);
            break;
        case ArgumentType::OPTIONAL_VALUE:
            parser.addOptionalValue(
                entry.shortOption, entry.longOption, entry.description, entry.typeName, entry.defaultValue);
            break;
        }
    }

    auto options = parser.parse(argc, argv, argcOffset, actionWhenOptionUnknown);

    for (const auto& assignment : assignments)
    {
        assignment(options);
    }

    binaryName = options.binaryName();
}
} // namespace internal
} // namespace cxx
} // namespace iox

#endif
