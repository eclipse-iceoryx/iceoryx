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

#include "iceoryx_hoofs/cxx/command_line_parser.hpp"
#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;

class CommandLineParser_test : public Test
{
  public:
    void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

template <typename T>
struct TypeToName
{
    static constexpr const char VALUE[] = "unknown";
};
template <typename T>
constexpr const char TypeToName<T>::VALUE[];

template <uint64_t N>
struct TypeToName<string<N>>
{
    static constexpr const char VALUE[] = "iox::cxx::string";
};
template <uint64_t N>
constexpr const char TypeToName<string<N>>::VALUE[];

template <>
struct TypeToName<int8_t>
{
    static constexpr const char VALUE[] = "int8_t";
};
constexpr const char TypeToName<int8_t>::VALUE[];

template <>
struct TypeToName<int16_t>
{
    static constexpr const char VALUE[] = "int16_t";
};
constexpr const char TypeToName<int16_t>::VALUE[];

template <>
struct TypeToName<int32_t>
{
    static constexpr const char VALUE[] = "int32_t";
};
constexpr const char TypeToName<int32_t>::VALUE[];

template <>
struct TypeToName<int64_t>
{
    static constexpr const char VALUE[] = "int64_t";
};
constexpr const char TypeToName<int64_t>::VALUE[];

template <>
struct TypeToName<uint8_t>
{
    static constexpr const char VALUE[] = "uint8_t";
};
constexpr const char TypeToName<uint8_t>::VALUE[];

template <>
struct TypeToName<uint16_t>
{
    static constexpr const char VALUE[] = "uint16_t";
};
constexpr const char TypeToName<uint16_t>::VALUE[];

template <>
struct TypeToName<uint32_t>
{
    static constexpr const char VALUE[] = "uint32_t";
};
constexpr const char TypeToName<uint32_t>::VALUE[];

template <>
struct TypeToName<uint64_t>
{
    static constexpr const char VALUE[] = "uint64_t";
};
constexpr const char TypeToName<uint64_t>::VALUE[];

template <>
struct TypeToName<char>
{
    static constexpr const char VALUE[] = "char";
};
constexpr const char TypeToName<char>::VALUE[];

namespace internal
{
using cmdEntries_t = vector<CommandLineParser::entry_t, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS>;
using cmdAssignments_t = vector<function<void(CommandLineOptions&)>, CommandLineOptions::MAX_NUMBER_OF_ARGUMENTS>;
} // namespace internal

template <typename T>
T addEntry(T& value,
           const char shortName,
           const CommandLineOptions::name_t& name,
           const CommandLineParser::description_t& description,
           const ArgumentType argumentType,
           const T defaultValue,
           internal::cmdEntries_t& entries,
           internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(CommandLineParser::entry_t{shortName, name, description, argumentType});
    assignments.emplace_back([&value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        auto result = options.get<T>(entries[index].longOption);
        if (result.has_error())
        {
            std::cerr << "It seems that the switch value of \"" << entries[index].longOption << "\" is not of type \""
                      << TypeToName<T>::VALUE << "\"" << std::endl;
            std::terminate();
        }

        value = result.value();
    });
    return defaultValue;
}

template <>
bool addEntry(bool& value,
              const char shortName,
              const CommandLineOptions::name_t& name,
              const CommandLineParser::description_t& description,
              const ArgumentType argumentType,
              const bool defaultValue,
              internal::cmdEntries_t& entries,
              internal::cmdAssignments_t& assignments)
{
    entries.emplace_back(CommandLineParser::entry_t{shortName, name, description, argumentType});
    assignments.emplace_back([&value, &entries, index = entries.size() - 1](CommandLineOptions& options) {
        value = options.has(entries[index].longOption);
    });
    return defaultValue;
}

void populateEntries(const internal::cmdEntries_t& entries,
                     const internal::cmdAssignments_t& assignments,
                     int argc,
                     char* argv[],
                     const uint64_t argcOffset,
                     const UnknownOption actionWhenOptionUnknown)
{
    CommandLineParser parser;
    for (auto& entry : entries)
    {
        parser.addOption(entry);
    }

    auto options = parser.parse(argc, argv, argcOffset, actionWhenOptionUnknown);

    for (auto& assignment : assignments)
    {
        assignment(options);
    }
}

#define OPTIONAL_VALUE(type, memberName, defaultValue, shortName, description)                                         \
  private:                                                                                                             \
    type m_##memberName = addEntry<type>(this->m_##memberName,                                                         \
                                         shortName,                                                                    \
                                         "memberName",                                                                 \
                                         description,                                                                  \
                                         ArgumentType::OPTIONAL_VALUE,                                                 \
                                         defaultValue,                                                                 \
                                         m_entries,                                                                    \
                                         m_assignments);                                                               \
                                                                                                                       \
  public:                                                                                                              \
    type memberName() const noexcept                                                                                   \
    {                                                                                                                  \
        return m_##memberName;                                                                                         \
    }

#define REQUIRED_VALUE(type, memberName, shortName, description)                                                       \
  private:                                                                                                             \
    type m_##memberName = addEntry<type>(this->m_##memberName,                                                         \
                                         shortName,                                                                    \
                                         "memberName",                                                                 \
                                         description,                                                                  \
                                         ArgumentType::REQUIRED_VALUE,                                                 \
                                         type(),                                                                       \
                                         m_entries,                                                                    \
                                         m_assignments);                                                               \
                                                                                                                       \
  public:                                                                                                              \
    type memberName() const noexcept                                                                                   \
    {                                                                                                                  \
        return m_##memberName;                                                                                         \
    }

#define SWITCH(memberName, shortName, description)                                                                     \
  private:                                                                                                             \
    bool m_##memberName = addEntry<bool>(this->m_##memberName,                                                         \
                                         shortName,                                                                    \
                                         "memberName",                                                                 \
                                         description,                                                                  \
                                         ArgumentType::OPTIONAL_VALUE,                                                 \
                                         false,                                                                        \
                                         m_entries,                                                                    \
                                         m_assignments);                                                               \
                                                                                                                       \
  public:                                                                                                              \
    bool memberName() const noexcept                                                                                   \
    {                                                                                                                  \
        return m_##memberName;                                                                                         \
    }

#define COMMAND_LINE_STRUCT(Name)                                                                                      \
  private:                                                                                                             \
    internal::cmdEntries_t m_entries;                                                                                  \
    internal::cmdAssignments_t m_assignments;                                                                          \
                                                                                                                       \
  public:                                                                                                              \
    Name(int argc,                                                                                                     \
         char* argv[],                                                                                                 \
         const uint64_t argcOffset = 1U,                                                                               \
         const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE)                                       \
    {                                                                                                                  \
        populateEntries(m_entries, m_assignments, argc, argv, argcOffset, actionWhenOptionUnknown);                    \
    }

#define COMMAND_LINE_STRUCT_ALT(Name, ...)                                                                             \
    struct Name                                                                                                        \
    {                                                                                                                  \
      private:                                                                                                         \
        internal::cmdEntries_t m_entries;                                                                              \
        internal::cmdAssignments_t m_assignments;                                                                      \
        __VA_ARGS__                                                                                                    \
      public:                                                                                                          \
        Name(int argc,                                                                                                 \
             char* argv[],                                                                                             \
             const uint64_t argcOffset = 1U,                                                                           \
             const UnknownOption actionWhenOptionUnknown = UnknownOption::TERMINATE)                                   \
        {                                                                                                              \
            populateEntries(m_entries, m_assignments, argc, argv, argcOffset, actionWhenOptionUnknown);                \
        }                                                                                                              \
    };

struct CommandLine
{
    COMMAND_LINE_STRUCT(CommandLine);

    OPTIONAL_VALUE(string<100>, service, {""}, 's', "some description");
    REQUIRED_VALUE(string<100>, instance, 's', "some description");
    SWITCH(doStuff, 'd', "do some stuff - some description");
    OPTIONAL_VALUE(uint64_t, version, 0, 'o', "sadasd");
};

COMMAND_LINE_STRUCT_ALT(CommandLineAlt, OPTIONAL_VALUE(string<100>, service, {""}, 's', "some description");
                        REQUIRED_VALUE(string<100>, instance, 's', "some description");
                        SWITCH(doStuff, 'd', "do some stuff - some description");
                        OPTIONAL_VALUE(uint64_t, version, 0, 'o', "sadasd"););

TEST_F(CommandLineParser_test, asd)
{
    int argc = 0;
    char** argv = nullptr;

    CommandLine cmd(argc, argv);
    CommandLineAlt cmdF(argc, argv);

    cmd.doStuff();

    cmdF.service();
}
} // namespace
