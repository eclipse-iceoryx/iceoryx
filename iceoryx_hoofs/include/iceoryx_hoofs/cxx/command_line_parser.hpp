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

class CommandLineOptions
{
  public:
    static constexpr uint64_t MAX_NUMBER_OF_ARGUMENTS = 16;
    static constexpr uint64_t MAX_OPTION_NAME_LENGTH = 32;
    static constexpr uint64_t MAX_OPTION_VALUE_LENGTH = 128;
    static constexpr uint64_t MAX_BINARY_NAME_LENGTH = 1024;

    using name_t = cxx::string<MAX_OPTION_NAME_LENGTH>;
    using value_t = cxx::string<MAX_OPTION_VALUE_LENGTH>;
    using binaryName_t = cxx::string<MAX_BINARY_NAME_LENGTH>;

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
    static constexpr char NO_SHORT_OPTION = '\0';

    using description_t = cxx::string<MAX_DESCRIPTION_LENGTH>;

    struct entry_t
    {
        char shortOption = NO_SHORT_OPTION;
        CommandLineOptions::name_t longOption;
        description_t description;
        ArgumentType type = ArgumentType::SWITCH;
    };

    CommandLineParser() noexcept;

    CommandLineParser&& addOption(const entry_t& option) && noexcept;
    CommandLineOptions parse(int argc, char* argv[]) && noexcept;

  private:
    cxx::optional<entry_t> getOption(const CommandLineOptions::name_t& name) const noexcept;
    bool areAllRequiredValuesPresent(const CommandLineOptions& options) const noexcept;
    void printHelpAndExit(const char* binaryName) const noexcept;

  private:
    cxx::vector<entry_t, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS> m_availableOptions;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/command_line_parser.inl"
