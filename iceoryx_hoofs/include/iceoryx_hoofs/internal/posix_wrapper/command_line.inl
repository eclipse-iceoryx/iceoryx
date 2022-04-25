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

#ifndef IOX_HOOFS_POSIX_WRAPPER_COMMAND_LINE_INL
#define IOX_HOOFS_POSIX_WRAPPER_COMMAND_LINE_INL

#include "iceoryx_hoofs/posix_wrapper/command_line.hpp"

namespace iox
{
namespace posix
{
namespace internal
{
template <typename T>
inline void extractValue(const CommandLineParser& parser,
                         T& value,
                         const cmdEntries_t& entries,
                         const uint64_t index,
                         const CommandLineOptions& options)
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

        handleError(parser);
    }

    value = result.value();
}

template <typename T>
inline T addEntry(const CommandLineParser& parser,
                  T& value,
                  const char shortName,
                  const CommandLineOptions::name_t& name,
                  const CommandLineParser::description_t& description,
                  const OptionType optionType,
                  T defaultValue,
                  internal::cmdEntries_t& entries,
                  internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(CommandLineParser::Entry{
        shortName,
        name,
        description,
        optionType,
        {cxx::TypeInfo<T>::NAME},
        CommandLineOptions::value_t(cxx::TruncateToCapacity, cxx::convert::toString(defaultValue))});
    assignments.emplace_back([&parser, &value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        extractValue(parser, value, entries, index, options);
    });
    return defaultValue;
}

template <>
inline bool addEntry(const CommandLineParser& parser,
                     bool& value,
                     const char shortName,
                     const CommandLineOptions::name_t& name,
                     const CommandLineParser::description_t& description,
                     const OptionType optionType,
                     bool defaultValue,
                     internal::cmdEntries_t& entries,
                     internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(CommandLineParser::Entry{
        shortName,
        name,
        description,
        optionType,
        {"true|false"},
        CommandLineOptions::value_t(cxx::TruncateToCapacity, (defaultValue) ? "true" : "false")});
    assignments.emplace_back([&parser, &value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        if (entries[index].type == OptionType::SWITCH)
        {
            value = options.has(entries[index].longOption);
        }
        else
        {
            extractValue(parser, value, entries, index, options);
        }
    });
    return defaultValue;
}
} // namespace internal
} // namespace posix
} // namespace iox

#endif
