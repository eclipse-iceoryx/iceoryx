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

#include "iceoryx_hoofs/cxx/command_line.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
void handleError(const CommandLineParser& parser)
{
    parser.printHelpAndExit();
}

void populateEntries(CommandLineParser& parser,
                     const cmdEntries_t& entries,
                     const cmdAssignments_t& assignments,
                     CommandLineOptions::binaryName_t& binaryName,
                     int argc,
                     char* argv[],
                     const uint64_t argcOffset,
                     const UnknownOption actionWhenOptionUnknown)
{
    for (const auto& entry : entries)
    {
        switch (entry.type)
        {
        case ArgumentType::SWITCH:
            parser.addSwitch(entry.shortOption, entry.longOption, entry.description);
            break;
        case ArgumentType::REQUIRED_VALUE:
            parser.addRequiredValue(entry.shortOption, entry.longOption, entry.description, entry.typeName);
            break;
        case ArgumentType::OPTIONAL_VALUE:
            parser.addOptionalValue(
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
} // namespace cxx
} // namespace iox
