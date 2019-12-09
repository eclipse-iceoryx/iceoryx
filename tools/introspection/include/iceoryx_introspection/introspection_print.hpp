// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "introspection_types.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include <map>
#include <ncurses.h>
#include <unistd.h>
#include <vector>

namespace iox
{
namespace client
{
namespace introspection
{
/// @brief color pairs for terminal printing
enum class ColorPairs : uint8_t
{
    redOnBlack = 1,
    whiteOnRed
};

/// @brief formatting options for terminal printing
static const std::map<PrettyOptions, uint32_t> prettyMap = {
    {PrettyOptions::title, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(ColorPairs::redOnBlack))},
    {PrettyOptions::highlight, A_BOLD | A_UNDERLINE},
    {PrettyOptions::error, A_BOLD | COLOR_PAIR(static_cast<uint8_t>(ColorPairs::whiteOnRed))},
    {PrettyOptions::bold, A_BOLD},
    {PrettyOptions::normal, A_NORMAL}};

/// @brief initializes ncurses terminal
void initTerminal();

/// @brief closes ncurses terminal
void closeTerminal();

/// @brief prints buffered data to the terminal
void refreshTerminal();

/// @brief updates the first pad coordinates to display
void updateDisplayYX();

/// @brief waits until user input or timeout
/// @param[in] timeoutMs timeout in milliseconds (-1 to wait forever)
void waitForUserInput(int32_t timeoutMs);

/// @brief prints to the terminal
/// @param[in] str string to print
/// @param[in] pr formatting options
void prettyPrint(const std::string& str, const PrettyOptions pr = PrettyOptions::normal);

/// @brief prints active process IDs and names
void printProcessIntrospectionData(const ProcessIntrospectionFieldTopic& processIntrospectionField);

/// @brief prints table showing current mempool usage
void printMemPoolInfo(const MemPoolIntrospectionTopic& topic);

void printPortIntrospectionData(const std::vector<ComposedSenderPortData>& senderPortData,
                                const std::vector<ComposedReceiverPortData>& receiverPortData);

} // namespace introspection
} // namespace client
} // namespace iox
