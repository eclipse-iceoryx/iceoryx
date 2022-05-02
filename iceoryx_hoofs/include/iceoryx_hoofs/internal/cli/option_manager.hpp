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
#ifndef IOX_HOOFS_CLI_OPTION_MANAGER_HPP
#define IOX_HOOFS_CLI_OPTION_MANAGER_HPP

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cli/command_line_argument_parser.hpp"
#include "iceoryx_hoofs/internal/cli/command_line_option_set.hpp"

namespace iox
{
namespace cli
{
namespace internal
{
using CmdAssignments_t =
    cxx::vector<cxx::function<void(CommandLineOptionValue&)>, CommandLineOptionValue::MAX_NUMBER_OF_ARGUMENTS>;

class OptionManager
{
  public:
    OptionManager(const OptionDescription_t& programDescription, const cxx::function<void()> onFailureCallback);

    void handleError() const;

    template <typename T>
    T extractOptionArgumentValue(const CommandLineOptionValue& options, const char shortName, const OptionName_t& name);

    template <typename T>
    T defineOption(T& referenceToMember, // not a pointer since it must be always valid
                   const char shortName,
                   const OptionName_t& name,
                   const OptionDescription_t& description,
                   const OptionType optionType,
                   T defaultArgumentValue // not const to enable RTVO
    );

    void populateEntries(BinaryName_t& binaryName,
                         int argc,
                         char* argv[],
                         const uint64_t argcOffset,
                         const UnknownOption actionWhenOptionUnknown);

  private:
    CommandLineArgumentParser m_parser;
    CommandLineOptionSet m_optionSet;
    CmdAssignments_t m_assignments;
};

} // namespace internal
} // namespace cli
} // namespace iox

#include "iceoryx_hoofs/internal/cli/option_manager.inl"
#endif
