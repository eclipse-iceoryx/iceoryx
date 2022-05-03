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

#include "iceoryx_hoofs/internal/cli/command_line_option_set.hpp"

namespace iox
{
namespace cli
{
namespace internal
{
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
CommandLineOptionSet& CommandLineOptionSet::addRequired(const char shortOption,
                                                        const OptionName_t& longOption,
                                                        const OptionDescription_t& description,
                                                        const TypeName_t& typeName) noexcept
{
    return addOption({shortOption, longOption, description, OptionType::REQUIRED, typeName, {""}});
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
} // namespace internal
} // namespace cli
} // namespace iox
