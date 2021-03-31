// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#ifndef CMDLINEPARSERFUZZING_HPP
#define CMDLINEPARSERFUZZING_HPP

#include <string>
#include <vector>

enum class FuzzingApi
{
    NONE,
    UDS,
    COM,
    TOML
};

enum class InputMode
{
    NONE,
    STDIN,
    CL
};

/// @brief      CmdLineParserFuzzing is a class which parses the command lines to configure the Fuzz Wrappers for
/// example to tell
///				which interface shall be fuzzed.
class CmdLineParserFuzzing
{
  public:
    CmdLineParserFuzzing();

    /// @brief	Parses the command line parameters whiich are entered by starting the fuzz wrappers
    /// @param[in] 	amount of arguments given to the method
    ///	@param[in]	containing the command line parameters
    ///	@param[out]	Containing the messages which shall be sent to the interface
    std::vector<std::string> parseCmd(int argc, char* argv[]) noexcept;

    /// @brief	Getter to return m_helpFlag
    ///	@param[out]	Containing a flag showing if the help menu was displayed.
    bool getHelpFlag();

    /// @brief	Getter to return m inputMode
    ///	@param[out]	Containing enum InputMode to show if messages are sent to the APi via stdin or command line (cl).
    InputMode getInputMode();

    /// @brief	Getter to return m_errorFlag
    ///	@param[out]	Containing a flag showing if an error happened and fuzzing cannot be started
    bool getErrorFlag();

    /// @brief	Getter to return m_cmdLineFlag
    ///	@param[out]	Containing a flag showing if a command line parameter was given after InputMode::CL was set
    bool getCmdLineFlag();

    /// @brief	Getter to return m_fuzzingAPI
    ///	@param[out]	Containing enum FuzzingFlag indicating which API wants to be fuzzed.
    FuzzingApi getFuzzingAPI();

    /// @brief	Getter to return m_tomlFileFlag
    ///	@param[out]	Containing a flag showing if TOML API wants to be fuzzed.
    bool getTomlFileFlag();

    /// @brief	Getter to return m_tomlFile
    ///	@param[out]	Containing an std::string to a file which can be used to temporarily write a TOML configuration to
    ///the file.
    std::string getTomlFile();

  private:
    std::vector<std::string> m_allMessages;
    bool m_helpFlag;
    InputMode m_inputMode;
    bool m_errorFlag;
    bool m_cmdLineFlag;
    FuzzingApi m_fuzzingAPI;
    bool m_tomlFileFlag;
    std::string m_tomlFile;
};
#endif // CMDLINEPARSERFUZZING_HPP
