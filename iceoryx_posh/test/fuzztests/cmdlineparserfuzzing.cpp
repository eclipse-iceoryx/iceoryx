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

#include "cmdlineparserfuzzing.hpp"
#include "iceoryx_utils/platform/getopt.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "fuzz_helper.hpp"
#include <iostream>
#include <string>
#include <fstream>

iox::log::LogLevel m_logLevel{iox::log::LogLevel::kOff};

CmdLineParserFuzzing::CmdLineParserFuzzing()
{
	fuzzingAPI = fuzzingAPI::NONE;
	inputMode = inputMode::NONE;
	errorFlag = true;
	cmdLineFlag = false;
	helpFlag = false;
	tomlFileFlag = false;
	tomlFile = "";

}

std::vector<std::string> CmdLineParserFuzzing::parseCmd(int argc, char* argv[]) noexcept
{
	constexpr option longOptions[] = {{"help", no_argument, nullptr, 'h'},
									  {"fuzzing-API", required_argument, nullptr, 'f'},
									  {"input-mode", required_argument, nullptr, 'm'},
									  {"command-line-file", required_argument, nullptr, 'c'},
									  {"command-line-input", required_argument, nullptr, 'i'},
									  {"toml-file", required_argument, nullptr, 't'},
									  {"log-level", required_argument, nullptr, 'l'},
									  {nullptr, 0, nullptr, 0}};
	constexpr const char* shortOptions = "hf:m:i:l:c:t:";
	int32_t index;
	int32_t opt{-1};
	while ((opt = getopt_long(argc, argv, shortOptions, longOptions, &index), opt != -1))
	{
		 errorFlag = false;
		 switch (opt)
		 {
			 case 'h':
			 {
				 std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
				 std::cout << "Options:" << std::endl;
				 std::cout << "-h, --help                              Display help." << std::endl;
				 std::cout << "-f, --fuzzing-api <API>               	Specify API which will be fuzzed." << std::endl;
				 std::cout << "                                      	<API> {uds, com, toml}" << std::endl;
				 std::cout << "                                      	uds: Starts RouDi and sends messages via Unix Domain Sockets. Multiple messages can be sent. (e.g.: register message first and then offer service)." << std::endl;
				 std::cout << "                                      	com: Invokes the processMessage method in RouDi directly. This abstracts the IPC and is faster but multiple messages are not supported." << std::endl;
				 std::cout << "                                      	toml: Send inputs to test the TOML config file parser. A file is created in your current working directory and the path is sent to the Parser." << std::endl;
				 std::cout << "-m, --input-mode <MODE>              	<MODE> {stdin, cl}" << std::endl;
				 std::cout << "                                      	stdin: Send input via stdin." << std::endl;
				 std::cout << "                                      	cl: Send input via commandline. Needs parameter i to send the input." << std::endl;
				 std::cout << "-c, --command-line-file <PATH_TO_FILE> 	<PATH_TO_FILE> : Read the specified file and send the input to the interface." << std::endl;
				 std::cout << "-i, --command-line-input <INPUT>      	<INPUT> : Send the input via this command line, requires to use input-mode cl. It's possible to send several commands with several -i commands." << std::endl;
				 std::cout << "-t, --toml-file <PATH_TO_FILE>          <PATH_TO_FILE> : Needs to be used when TOML is parsed. The file is used to write messages which will be parsed by the TOML configuration parser." << std::endl;
				 std::cout << "-l, --log-level                         <LogLevel>  {off, fatal, debug} : Set the log level. Off is standard;" << std::endl;
				 helpFlag = true;
			 } break;

			 case 'f':
			 {
				 if (strcmp(optarg, "uds") == 0)
				  {
					 fuzzingAPI = fuzzingAPI::UDS;
				  }
				  else if (strcmp(optarg, "com") == 0)
				  {
					  fuzzingAPI = fuzzingAPI::COM;
				  }
				  else if (strcmp(optarg, "toml") == 0)
				  {
					  fuzzingAPI = fuzzingAPI::TOML;
				  }
				  else
				  {
					  std::cout << "Options for fuzzing-api are 'uds', 'com' and 'toml'!" << std::endl;
				  }
			 } break;

			 case 'm':
			 {
				 if (strcmp(optarg, "stdin") == 0)
				  {
					  inputMode = inputMode::STDIN;
					  FuzzHelper aFuzzHelper;
					  allMessages = aFuzzHelper.getStdInMessages();
				  }
				  else if (strcmp(optarg, "cl") == 0)
				  {
					  inputMode = inputMode::CL;
				  }
				  else
				  {
					  std::cout << "Options for input-mode are 'stdin' and 'cl'!" << std::endl;
				  }
			 } break;
			 case 'i':
			 {
				  cmdLineFlag = true;
				  allMessages.emplace_back(optarg);

			 }  break;

			 case 'c':
			 {
				  cmdLineFlag = true;
				  std::ifstream ifile;
				  ifile.open(optarg);
				  if(ifile)
				  {
					  std::string tempFileContent( (std::istreambuf_iterator<char>(ifile) ), (std::istreambuf_iterator<char>()));
					  allMessages.emplace_back(tempFileContent);
				  }

				  else
				  {
					  std::cout<<"Error cannot open file. Either file does not exist or I don't have the permissions to open it.";
					  errorFlag = true;
					  return allMessages;
				  }
			 }break;

			 case 'l':
			 {
				 if (strcmp(optarg, "off") == 0)
				 {
					 m_logLevel = iox::log::LogLevel::kOff;
				 }
				 else if (strcmp(optarg, "fatal") == 0)
				 {
					 m_logLevel = iox::log::LogLevel::kFatal;
				 }
				 else if (strcmp(optarg, "debug") == 0)
				 {
					 m_logLevel = iox::log::LogLevel::kDebug;
				 }
				 else
				 {
					 std::cout << "Options for Logging are 'off', 'fatal' and 'debug'!" << std::endl;
				 }
				 iox::log::LogManager::GetLogManager().SetDefaultLogLevel(m_logLevel);
			 } break;
			 case 't':
			 {
				 tomlFileFlag = true;
				 tomlFile = optarg;
			 } break;
			 default:
			 {
				std::cout << "Unknown command.\n" << std::endl;
				errorFlag = true;
				return allMessages;
			 } break;
		 };
	}
	return allMessages;
}
