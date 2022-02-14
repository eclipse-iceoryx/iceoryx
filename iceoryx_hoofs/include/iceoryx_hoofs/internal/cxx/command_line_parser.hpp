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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/platform/platform_settings.hpp"

#include <cstdint>

namespace iox
{
namespace cxx
{
enum class ArgumentType
{
    SWITCH,
    REQUIRED_VALUE,
    OPTIONAL_VALUE
};

enum class UnknownOption
{
    IGNORE,
    TERMINATE
};

class IOX_NO_DISCARD CommandLineOptions
{
  public:
    static constexpr uint64_t MAX_NUMBER_OF_ARGUMENTS = 16;
    static constexpr uint64_t MAX_OPTION_NAME_LENGTH = 32;
    static constexpr uint64_t MAX_OPTION_VALUE_LENGTH = 128;

    using name_t = cxx::string<MAX_OPTION_NAME_LENGTH>;
    using value_t = cxx::string<MAX_OPTION_VALUE_LENGTH>;
    using binaryName_t = cxx::string<platform::IOX_MAX_PATH_LENGTH>;

    enum class Result
    {
        NO_SUCH_VALUE,
        UNABLE_TO_CONVERT_VALUE
    };

    template <typename T>
    cxx::expected<T, Result> get(const name_t& optionName) const noexcept;
    bool has(const name_t& switchName) const noexcept;
    const binaryName_t& binaryName() const noexcept;

    friend class CommandLineParser;

  private:
    struct argument_t
    {
        char shortId;
        name_t id;
        value_t value;
    };

    binaryName_t m_binaryName;
    cxx::vector<argument_t, MAX_NUMBER_OF_ARGUMENTS> m_arguments;
};

class CommandLineParser
{
  public:
    static constexpr uint64_t MAX_DESCRIPTION_LENGTH = 1024;
    static constexpr uint64_t OPTION_OUTPUT_WIDTH = 45;
    static constexpr uint64_t MAX_TYPE_LENGTH = 16;
    static constexpr char NO_SHORT_OPTION = '\0';

    using description_t = cxx::string<MAX_DESCRIPTION_LENGTH>;
    using typeName_t = cxx::string<MAX_TYPE_LENGTH>;

    struct entry_t
    {
        char shortOption = NO_SHORT_OPTION;
        CommandLineOptions::name_t longOption;
        description_t description;
        ArgumentType type = ArgumentType::SWITCH;
        typeName_t typeName;
        CommandLineOptions::value_t defaultValue;
    };

    explicit CommandLineParser(const description_t& programDescription) noexcept;

    CommandLineParser& addSwitch(const char shortOption,
                                 const CommandLineOptions::name_t& longOption,
                                 const description_t& description) noexcept;

    CommandLineParser& addOptionalValue(const char shortOption,
                                        const CommandLineOptions::name_t& longOption,
                                        const description_t& description,
                                        const typeName_t& typeName,
                                        const CommandLineOptions::value_t& defaultValue) noexcept;

    CommandLineParser& addRequiredValue(const char shortOption,
                                        const CommandLineOptions::name_t& longOption,
                                        const description_t& description,
                                        const typeName_t& typeName) noexcept;

    CommandLineOptions parse(int argc,
                             char* argv[],
                             const uint64_t argcOffset = 1U,
                             const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE) noexcept;

  private:
    CommandLineParser& addOption(const entry_t& option) noexcept;
    cxx::optional<entry_t> getOption(const CommandLineOptions::name_t& name) const noexcept;
    bool areAllRequiredValuesPresent() const noexcept;
    void printHelpAndExit(const char* binaryName) const noexcept;
    void sortAvailableOptions() noexcept;
    void setDefaultValuesToUnsetOptions() noexcept;
    bool hasArguments(const int argc) const noexcept;
    bool assignBinaryName(const char* name) noexcept;
    bool doesOptionStartWithMinus(const char* option) const noexcept;
    bool hasOptionName(const char* option) const noexcept;
    bool hasValidSwitchName(const char* option) const noexcept;
    bool hasValidOptionName(const char* option) const noexcept;
    bool doesOptionNameFitIntoString(const char* option) const noexcept;
    bool isNextArgumentAValue(const uint64_t position) const noexcept;
    static void printOption(const entry_t& entry) noexcept;
    bool isValueOptionFollowedByValue(const entry_t& entry, const bool isNextArgumentAValue) const noexcept;
    bool isOptionSet(const entry_t& entry) const noexcept;
    bool doesOptionValueFitIntoString(const char* value) const noexcept;
    bool failWhenEntryIsSwitch(const entry_t& entry, const char* nextArgument) const noexcept;

  private:
    int m_argc = 0;
    char** m_argv = nullptr;
    description_t m_programDescription;
    cxx::vector<entry_t, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS> m_availableOptions;
    CommandLineOptions m_options;
};

std::ostream& operator<<(std::ostream& stream, const CommandLineParser::entry_t& entry) noexcept;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/command_line_parser.inl"
