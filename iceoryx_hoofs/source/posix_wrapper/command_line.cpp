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

#include "iceoryx_hoofs/posix_wrapper/command_line.hpp"

namespace iox
{
namespace posix
{
namespace internal
{
void handleError(const CommandLineParser& parser)
{
    parser.printHelpAndExit();
}

void populateEntries(CommandLineParser& parser,
                     const CmdEntries_t& entries,
                     const CmdAssignments_t& assignments,
                     CommandLineOption::BinaryName_t& binaryName,
                     int argc,
                     char* argv[],
                     const uint64_t argcOffset,
                     const UnknownOption actionWhenOptionUnknown)
{
    for (const auto& entry : entries)
    {
        switch (entry.type)
        {
        case OptionType::SWITCH:
            parser.addSwitch(entry.shortOption, entry.longOption, entry.description);
            break;
        case OptionType::REQUIRED:
            parser.addMandatory(entry.shortOption, entry.longOption, entry.description, entry.typeName);
            break;
        case OptionType::OPTIONAL:
            parser.addOptional(
                entry.shortOption, entry.longOption, entry.description, entry.typeName, entry.defaultValue);
            break;
        }
    }

    auto options = parser.parse(argc, argv, argcOffset, actionWhenOptionUnknown);

    for (const auto& assignment : assignments)
    {
        assignment(options);
    }

    binaryName = options.binaryName();
}
} // namespace internal
} // namespace posix
} // namespace iox
