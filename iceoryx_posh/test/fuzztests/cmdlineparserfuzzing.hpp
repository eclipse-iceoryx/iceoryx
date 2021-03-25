// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include<string>
#include <vector>


enum class fuzzingAPI
{
    NONE,
	UDS,
    COM,
	TOML
};

enum class inputMode
{
	NONE,
    STDIN,
    CL
};

/// @brief      CmdLineParserFuzzing is a class which parses the command lines to configure the Fuzz Wrappers for example to tell 
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
		
		bool helpFlag;
		inputMode inputMode;
		bool errorFlag;
		bool cmdLineFlag;
		fuzzingAPI fuzzingAPI;
		bool tomlFileFlag;
		std::string tomlFile;

	private:

		std::vector<std::string> allMessages;
 };
