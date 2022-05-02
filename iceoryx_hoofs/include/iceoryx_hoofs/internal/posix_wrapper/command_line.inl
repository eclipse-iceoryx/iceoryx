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
inline void
OptionManager::extractOptionArgumentValue(T& referenceToMember, const uint64_t index, const CommandLineOption& options)
{
    auto result = options.get<T>(m_entries[index].longOption);
    if (result.has_error())
    {
        std::cout << "It seems that the option value of \"";
        const bool hasShortOption = (m_entries[index].shortOption != '\0');
        const bool hasLongOption = (!m_entries[index].longOption.empty());
        if (hasShortOption)
        {
            std::cout << "-" << m_entries[index].shortOption;
        }
        if (hasShortOption && hasLongOption)
        {
            std::cout << ", ";
        }
        if (hasLongOption)
        {
            std::cout << "--" << m_entries[index].longOption;
        }

        std::cout << "\" is not of type \"" << m_entries[index].typeName << "\"" << std::endl;

        handleError();
    }

    referenceToMember = result.value();
}

template <typename T>
inline T OptionManager::defineOption(T& referenceToMember,
                                     const char shortName,
                                     const CommandLineOption::Name_t& name,
                                     const CommandLineParser::Description_t& description,
                                     const OptionType optionType,
                                     T defaultArgumentValue)
{
    m_entries.emplace_back(CommandLineParser::Entry{
        shortName,
        name,
        description,
        optionType,
        {cxx::TypeInfo<T>::NAME},
        CommandLineOption::Argument_t(cxx::TruncateToCapacity, cxx::convert::toString(defaultArgumentValue))});
    m_assignments.emplace_back([this, &referenceToMember, index = m_entries.size() - 1](CommandLineOption& options) {
        this->extractOptionArgumentValue(referenceToMember, index, options);
    });
    return defaultArgumentValue;
}

template <>
inline bool OptionManager::defineOption(bool& referenceToMember,
                                        const char shortName,
                                        const CommandLineOption::Name_t& name,
                                        const CommandLineParser::Description_t& description,
                                        const OptionType optionType,
                                        bool defaultArgumentValue)
{
    m_entries.emplace_back(CommandLineParser::Entry{
        shortName,
        name,
        description,
        optionType,
        {"true|false"},
        CommandLineOption::Argument_t(cxx::TruncateToCapacity, (defaultArgumentValue) ? "true" : "false")});
    m_assignments.emplace_back([this, &referenceToMember, index = m_entries.size() - 1](CommandLineOption& options) {
        if (m_entries[index].type == OptionType::SWITCH)
        {
            referenceToMember = options.has(m_entries[index].longOption);
        }
        else
        {
            this->extractOptionArgumentValue(referenceToMember, index, options);
        }
    });
    return defaultArgumentValue;
}
} // namespace internal
} // namespace posix
} // namespace iox

#endif
