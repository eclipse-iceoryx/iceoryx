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

#ifndef IOX_HOOFS_CLI_OPTION_MANAGER_INL
#define IOX_HOOFS_CLI_OPTION_MANAGER_INL

#include "iceoryx_hoofs/internal/cli/option_manager.hpp"

namespace iox
{
namespace cli
{
namespace internal
{
template <typename T>
inline T OptionManager::extractOptionArgumentValue(const CommandLineOptionValue& options,
                                                   const char shortName,
                                                   const OptionName_t& name)
{
    if (shortName != CommandLineOptionSet::NO_SHORT_OPTION)
    {
        return options.get<T>(OptionName_t{cxx::TruncateToCapacity, &shortName, 1})
            .or_else([this](auto&) { handleError(); })
            .value();
    }
    else
    {
        return options.get<T>(name).or_else([this](auto&) { handleError(); }).value();
    }
}

template <typename T>
inline T OptionManager::defineOption(T& referenceToMember,
                                     const char shortName,
                                     const OptionName_t& name,
                                     const OptionDescription_t& description,
                                     const OptionType optionType,
                                     T defaultArgumentValue)
{
    m_optionSet.addOption(
        CommandLineOptionSet::Value{shortName,
                                    name,
                                    description,
                                    optionType,
                                    {cxx::TypeInfo<T>::NAME},
                                    Argument_t(cxx::TruncateToCapacity, cxx::convert::toString(defaultArgumentValue))});

    m_assignments.emplace_back([this, &referenceToMember, shortName, name](CommandLineOptionValue& options) {
        referenceToMember = extractOptionArgumentValue<T>(options, shortName, name);
    });

    return defaultArgumentValue;
}

template <>
inline bool OptionManager::defineOption(bool& referenceToMember,
                                        const char shortName,
                                        const OptionName_t& name,
                                        const OptionDescription_t& description,
                                        const OptionType optionType,
                                        bool defaultArgumentValue)
{
    m_optionSet.addOption(
        CommandLineOptionSet::Value{shortName,
                                    name,
                                    description,
                                    optionType,
                                    {cxx::TypeInfo<bool>::NAME},
                                    Argument_t(cxx::TruncateToCapacity, cxx::convert::toString(defaultArgumentValue))});

    m_assignments.emplace_back(
        [this, &referenceToMember, optionType, shortName, name](CommandLineOptionValue& options) {
            if (optionType == OptionType::SWITCH)
            {
                if (shortName != CommandLineOptionSet::NO_SHORT_OPTION)
                {
                    referenceToMember = options.has(OptionName_t{cxx::TruncateToCapacity, &shortName, 1});
                }
                else
                {
                    referenceToMember = options.has(name);
                }
            }
            else
            {
                referenceToMember = extractOptionArgumentValue<bool>(options, shortName, name);
            }
        });
    return defaultArgumentValue;
}
} // namespace internal
} // namespace cli
} // namespace iox

#endif
