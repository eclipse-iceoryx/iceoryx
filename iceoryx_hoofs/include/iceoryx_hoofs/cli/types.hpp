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

#ifndef IOX_HOOFS_CLI_TYPES_HPP
#define IOX_HOOFS_CLI_TYPES_HPP

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/platform/platform_settings.hpp"

#include <cstdint>

namespace iox
{
namespace cli
{
/// @brief defines the type of command line argument option
enum class OptionType
{
    /// @brief option when provided is true
    SWITCH,
    /// @brief option with value which has to be provided
    REQUIRED,
    /// @brief option with value which can be provided
    OPTIONAL
};

/// @brief defines the action when an unknown option is encountered
enum class UnknownOption
{
    IGNORE,
    TERMINATE
};

static constexpr uint64_t MAX_OPTION_NAME_LENGTH = 32;
static constexpr uint64_t MAX_OPTION_ARGUMENT_LENGTH = 128;
static constexpr uint64_t MAX_OPTION_DESCRIPTION_LENGTH = 1024;
static constexpr uint64_t MAX_TYPE_NAME_LENGTH = 16;
static constexpr char NO_SHORT_OPTION = '\0';

using OptionName_t = cxx::string<MAX_OPTION_NAME_LENGTH>;
using OptionDescription_t = cxx::string<MAX_OPTION_DESCRIPTION_LENGTH>;
using Argument_t = cxx::string<MAX_OPTION_ARGUMENT_LENGTH>;
using BinaryName_t = cxx::string<platform::IOX_MAX_PATH_LENGTH>;
using TypeName_t = cxx::string<MAX_TYPE_NAME_LENGTH>;

struct Option
{
    bool isSwitch() const noexcept;
    bool hasOptionName(const OptionName_t& name) const noexcept;
    bool isSameOption(const Option& rhs) const noexcept;
    bool operator<(const Option& rhs) const noexcept;
    bool isEmpty() const noexcept;
    bool longOptionNameDoesStartWithDash() const noexcept;
    bool shortOptionNameIsEqualDash() const noexcept;
    bool hasLongOptionName(const OptionName_t& value) const noexcept;
    bool hasShortOptionName(const char value) const noexcept;
    bool hasShortOption() const noexcept;
    bool hasLongOption() const noexcept;

    char shortOption = NO_SHORT_OPTION;
    OptionName_t longOption;
    Argument_t value;
};

struct OptionWithDetails : public Option
{
    OptionWithDetails(const Option& option,
                      const OptionDescription_t& description,
                      const OptionType type,
                      const TypeName_t& typeName) noexcept;

    bool operator<(const OptionWithDetails& rhs) const noexcept;

    struct
    {
        OptionDescription_t description;
        OptionType type = OptionType::SWITCH;
        TypeName_t typeName;
    } details;
};

} // namespace cli
} // namespace iox
#endif
