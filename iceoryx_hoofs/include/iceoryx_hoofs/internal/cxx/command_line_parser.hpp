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
#include "iceoryx_hoofs/cxx/function.hpp"
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

/// @brief This class provides access to the command line argument values.
///        When constructed with the default constructor it is empty. Calling
///        CommandLineParser::parse creates and returns a populated CommandLineOptions
///        object.
///        This class should never be used directly. Use the CommandLine builder
///        from `iceoryx_hoofs/cxx/command_line.hpp` to create a struct which contains
///        the values.
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
        UNABLE_TO_CONVERT_VALUE,
        NO_SUCH_VALUE
    };

    /// @brief returns the value of a specified option
    /// @tparam T the type of the value
    /// @param[in] optionName either one letter for the shortOption or the whole longOption
    /// @return the contained value if the value is present and convertable, otherwise an Result which describes the
    /// error
    template <typename T>
    cxx::expected<T, Result> get(const name_t& optionName) const noexcept;

    /// @brief returns true if the specified switch was set, otherwise false
    /// @param[in] switchName either one letter for the shortOption or the whole longOption
    bool has(const name_t& switchName) const noexcept;

    /// @brief returns the full path name of the binary
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

/// @brief Factory class for the CommandLineOptions. First one has to register
///        all switches and options before calling parse. This is required for
///        the help page which is generated and printed on failure as well as
///        for consistency and syntax checks.
class CommandLineParser
{
  public:
    static constexpr uint64_t MAX_DESCRIPTION_LENGTH = 1024;
    static constexpr uint64_t OPTION_OUTPUT_WIDTH = 45;
    static constexpr uint64_t MAX_TYPE_LENGTH = 16;
    static constexpr char NO_SHORT_OPTION = '\0';

    using description_t = cxx::string<MAX_DESCRIPTION_LENGTH>;
    using typeName_t = cxx::string<MAX_TYPE_LENGTH>;

    /// @brief The constructor.
    /// @param[in] programDescription The description to the program. Will be printed in the help.
    /// @param[in] onFailureCallback callback which is called when parse fails, if nothing is defined std::exit(1) is
    ///            called
    CommandLineParser(
        const description_t& programDescription,
        const cxx::function<void()> onFailureCallback = [] { std::exit(EXIT_FAILURE); }) noexcept;

    /// @brief Adds a command line switch argument
    ///        Calls the error handler when the option was already added or the shortOption and longOption are empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with minus as long option name
    /// @param[in] description the description to the argument
    CommandLineParser& addSwitch(const char shortOption,
                                 const CommandLineOptions::name_t& longOption,
                                 const description_t& description) noexcept;

    /// @brief Adds a command line optional value argument.
    ///        Calls the error handler when the option was already added or the shortOption and longOption are empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with minus as long option name
    /// @param[in] description the description to the argument
    /// @param[in] typeName the name of the value type
    /// @param[in] defaultValue the value which will be set to the option when it is not set by the user
    CommandLineParser& addOptionalValue(const char shortOption,
                                        const CommandLineOptions::name_t& longOption,
                                        const description_t& description,
                                        const typeName_t& typeName,
                                        const CommandLineOptions::value_t& defaultValue) noexcept;

    /// @brief Adds a command line required value argument
    ///        Calls the error handler when the option was already added or the shortOption and longOption are empty.
    /// @param[in] shortOption a single letter as short option
    /// @param[in] longOption a multi letter word which does not start with minus as long option name
    /// @param[in] description the description to the argument
    /// @param[in] typeName the name of the value type
    CommandLineParser& addRequiredValue(const char shortOption,
                                        const CommandLineOptions::name_t& longOption,
                                        const description_t& description,
                                        const typeName_t& typeName) noexcept;

    /// @brief Parses the arguments from the command line.
    ///        Calls the error handler when the command line arguments contain illegal syntax or required values are
    ///        not provided
    /// @param[in] argc number of arguments, see int main(int argc, char*argv[])
    /// @param[in] argv the string array of arguments, see int main(int argc, char*argv[])
    /// @param[in] argcOffset the starting point for the parsing. 1U starts at the first argument.
    /// @param[in] actionWhenOptionUnknown defines the action which should be performed when the user sets a
    ///             option/switch which is unknown
    CommandLineOptions parse(int argc,
                             char* argv[],
                             const uint64_t argcOffset = 1U,
                             const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE) noexcept;

    struct entry_t
    {
        char shortOption = NO_SHORT_OPTION;
        CommandLineOptions::name_t longOption;
        description_t description;
        ArgumentType type = ArgumentType::SWITCH;
        typeName_t typeName;
        CommandLineOptions::value_t defaultValue;
    };

  private:
    CommandLineParser& addOption(const entry_t& option) noexcept;
    cxx::optional<entry_t> getOption(const CommandLineOptions::name_t& name) const noexcept;
    void printHelpAndExit(const char* binaryName) const noexcept;

    /// BEGIN only used in parse to improve readability
    ///
    /// Do not use those internal methods outside of parse. They were written
    /// to improve the readability of the code. None of those functions verify
    /// the pre conditions they require, this has to be done by calling them
    /// in the correct order.
    bool areAllRequiredValuesPresent() const noexcept;
    bool hasArguments(const int argc) const noexcept;
    bool assignBinaryName(const char* name) noexcept;
    bool doesOptionStartWithMinus(const char* option) const noexcept;
    bool isOptionNameEmpty(const char* option) const noexcept;
    bool hasValidSwitchName(const char* option) const noexcept;
    bool hasValidOptionName(const char* option) const noexcept;
    bool doesOptionNameFitIntoString(const char* option) const noexcept;
    bool isNextArgumentAValue(const uint64_t position) const noexcept;
    bool isOptionSet(const entry_t& entry) const noexcept;
    bool doesOptionValueFitIntoString(const char* value) const noexcept;
    bool doesOptionHasSucceedingValue(const entry_t& entry, const uint64_t position) const noexcept;
    /// END only used in parse to improve readability

    void sortAvailableOptions() noexcept;
    void setDefaultValuesToUnsetOptions() noexcept;

  private:
    int m_argc = 0;
    char** m_argv = nullptr;
    description_t m_programDescription;
    cxx::vector<entry_t, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS> m_availableOptions;
    cxx::function<void()> m_onFailureCallback;
    CommandLineOptions m_options;
};

std::ostream& operator<<(std::ostream& stream, const CommandLineParser::entry_t& entry) noexcept;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/command_line_parser.inl"
