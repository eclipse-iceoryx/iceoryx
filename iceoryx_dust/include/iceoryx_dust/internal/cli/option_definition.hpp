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

#ifndef IOX_DUST_CLI_OPTION_DEFINITION_HPP
#define IOX_DUST_CLI_OPTION_DEFINITION_HPP

#include "iceoryx_dust/cli/types.hpp"
#include "iceoryx_dust/internal/cli/arguments.hpp"
#include "iox/function.hpp"
#include "iox/vector.hpp"
#include <cstdint>

namespace iox
{
namespace cli
{
namespace internal
{
/// @brief A set of options which is provided to the CommandLineParser.
///     Description, short and long name as well as type and value can be defined for every
///     command line option which the application provides.
///     The parser uses this set to populate the Arguments.
class OptionDefinition
{
  public:
    /// @brief The constructor.
    /// @param[in] programDescription The description to the program. Will be printed in the help.
    /// @param[in] onFailureCallback callback which is called when parse fails, if nothing is
    ///            defined std::exit(EXIT_FAILURE) is called
    explicit OptionDefinition(
        const OptionDescription_t& programDescription,
        const function<void()> onFailureCallback = [] { std::exit(EXIT_FAILURE); }) noexcept;

    /// @brief Adds a command line switch argument
    ///        Calls the onFailureCallback when the option was already added or the shortOption and longOption are
    ///        empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with dash as long option name
    /// @param[in] description the description to the argument
    OptionDefinition&
    addSwitch(const char shortOption, const OptionName_t& longOption, const OptionDescription_t& description) noexcept;

    /// @brief Adds a command line optional value argument.
    ///        Calls the onFailureCallback when the option was already added or the shortOption and longOption are
    ///        empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with dash as long option name
    /// @param[in] description the description to the argument
    /// @param[in] typeName the name of the value type
    /// @param[in] defaultValue the value which will be set to the option when it is not set by the user
    OptionDefinition& addOptional(const char shortOption,
                                  const OptionName_t& longOption,
                                  const OptionDescription_t& description,
                                  const TypeName_t& typeName,
                                  const Argument_t& defaultValue) noexcept;

    /// @brief Adds a command line required value argument
    ///        Calls the onFailureCallback when the option was already added or the shortOption and longOption are
    ///        empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with dash as long option name
    /// @param[in] description the description to the argument
    /// @param[in] typeName the name of the value type
    OptionDefinition& addRequired(const char shortOption,
                                  const OptionName_t& longOption,
                                  const OptionDescription_t& description,
                                  const TypeName_t& typeName) noexcept;

  private:
    friend class OptionManager;
    friend class CommandLineParser;
    friend std::ostream& operator<<(std::ostream&, const OptionWithDetails&) noexcept;

    OptionDefinition& addOption(const OptionWithDetails& option) noexcept;
    optional<OptionWithDetails> getOption(const OptionName_t& name) const noexcept;

  private:
    OptionDescription_t m_programDescription;
    vector<OptionWithDetails, MAX_NUMBER_OF_ARGUMENTS> m_availableOptions;
    function<void()> m_onFailureCallback;
};

std::ostream& operator<<(std::ostream& stream, const OptionWithDetails& value) noexcept;
} // namespace internal
} // namespace cli
} // namespace iox

#endif
