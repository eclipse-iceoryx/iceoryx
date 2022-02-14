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
#ifndef IOX_HOOFS_CXX_COMMAND_LINE_HPP
#define IOX_HOOFS_CXX_COMMAND_LINE_HPP

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/command_line_parser.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
using cmdEntries_t = vector<CommandLineParser::entry_t, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS>;
using cmdAssignments_t = vector<function<void(CommandLineOptions&)>, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS>;

template <typename T>
T addEntry(T& value,
           const char shortName,
           const CommandLineOptions::name_t& name,
           const CommandLineParser::description_t& description,
           const ArgumentType argumentType,
           T defaultValue, // not const to enable RTVO
           internal::cmdEntries_t& entries,
           internal::cmdAssignments_t& assignments);

void populateEntries(const internal::cmdEntries_t& entries,
                     const internal::cmdAssignments_t& assignments,
                     ::iox::cxx::CommandLineOptions::binaryName_t& binaryName,
                     const CommandLineParser::description_t& programDescription,
                     int argc,
                     char* argv[],
                     const uint64_t argcOffset,
                     const iox::cxx::UnknownOption actionWhenOptionUnknown);

#define INTERNAL_CMD_LINE_VALUE(type, memberName, defaultValue, shortName, longName, description, argumentType)        \
  private:                                                                                                             \
    type m_##memberName = iox::cxx::internal::addEntry<type>(                                                          \
        this->m_##memberName, shortName, longName, description, argumentType, defaultValue, m_entries, m_assignments); \
                                                                                                                       \
  public:                                                                                                              \
    const type& memberName() const noexcept                                                                            \
    {                                                                                                                  \
        return m_##memberName;                                                                                         \
    }
} // namespace internal

/// @brief Adds an optional value to the command line
/// @param[in] type the type of the required value
/// @param[in] memberName the name under which the switch is accessible
/// @param[in] defaultValue the value when it is not set from outside
/// @param[in] shortName a single character for the short option like `-s` for instance
/// @param[in] longName a long option name under which this can be accessed like `--some-name` for instance
/// @param[in] description a description of the switch
#define OPTIONAL_VALUE(type, memberName, defaultValue, shortName, longName, description)                               \
    INTERNAL_CMD_LINE_VALUE(                                                                                           \
        type, memberName, defaultValue, shortName, longName, description, iox::cxx::ArgumentType::OPTIONAL_VALUE)

/// @brief Adds a required value to the command line, if it is not provided the program will print the help and
///        terminate
/// @param[in] type the type of the required value
/// @param[in] memberName the name under which the switch is accessible
/// @param[in] shortName a single character for the short option like `-s` for instance
/// @param[in] longName a long option name under which this can be accessed like `--some-name` for instance
/// @param[in] description a description of the switch
#define REQUIRED_VALUE(type, memberName, shortName, longName, description)                                             \
    INTERNAL_CMD_LINE_VALUE(                                                                                           \
        type, memberName, type(), shortName, longName, description, iox::cxx::ArgumentType::REQUIRED_VALUE)

/// @brief Adds a switch to the command line
/// @param[in] memberName the name under which the switch is accessible
/// @param[in] shortName a single character for the short option like `-s` for instance
/// @param[in] longName a long option name under which this can be accessed like `--some-name` for instance
/// @param[in] description a description of the switch
#define SWITCH(memberName, shortName, longName, description)                                                           \
    INTERNAL_CMD_LINE_VALUE(                                                                                           \
        bool, memberName, false, shortName, longName, description, iox::cxx::ArgumentType::OPTIONAL_VALUE)

/// @brief Helper macro to create a struct with full command line parsing from argc, argv.
/// @param[in] Name the name of the class/struct
/// @param[in] ProgramDescription a description which describes the task of the program
/// @code
/// // With those macros a struct can be generated easily like this:
/// struct CommandLine
/// {
///     COMMAND_LINE(CommandLine, "My program description");
///
///     OPTIONAL_VALUE(string<100>, stringValue, {"default Value"}, 's', "string-value", "some description");
///     REQUIRED_VALUE(string<100>, anotherString, 'a', "another-string", "some description");
///     SWITCH(doStuff, 'd', "do-stuff", "do some stuff - some description");
///     OPTIONAL_VALUE(uint64_t, version, 0, 'v', "version", "some description");
/// };
///
/// // This struct stores parses all command line arguments and stores them. In
/// // the example above the struct provides access to
/// //   .stringValue()
/// //   .anotherString()
/// //   .doStuff()
/// //   .version()
/// // Via the command line parameters
/// //   -s or --string-value
/// //   -a or --another-string
/// //   -d or --do-stuff
/// //   -v or --version
///
/// int main(int argc, char* argv[]) {
///   CommandLine cmd(argc, argv);
///   std::cout << cmd.stringValue() << " " << cmd.anotherString() << std::endl;
/// }
/// @endcode
#define COMMAND_LINE(Name, ProgramDescription)                                                                         \
  private:                                                                                                             \
    ::iox::cxx::internal::cmdEntries_t m_entries;                                                                      \
    ::iox::cxx::internal::cmdAssignments_t m_assignments;                                                              \
    ::iox::cxx::CommandLineOptions::binaryName_t m_binaryName;                                                         \
                                                                                                                       \
  public:                                                                                                              \
    Name(int argc,                                                                                                     \
         char* argv[],                                                                                                 \
         const uint64_t argcOffset = 1U,                                                                               \
         const iox::cxx::UnknownOption actionWhenOptionUnknown = iox::cxx::UnknownOption::TERMINATE)                   \
    {                                                                                                                  \
        ::iox::cxx::internal::populateEntries(m_entries,                                                               \
                                              m_assignments,                                                           \
                                              m_binaryName,                                                            \
                                              ProgramDescription,                                                      \
                                              argc,                                                                    \
                                              argv,                                                                    \
                                              argcOffset,                                                              \
                                              actionWhenOptionUnknown);                                                \
    }                                                                                                                  \
                                                                                                                       \
    const ::iox::cxx::CommandLineOptions::binaryName_t& binaryName() const noexcept                                    \
    {                                                                                                                  \
        return m_binaryName;                                                                                           \
    }
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/command_line.inl"
#endif