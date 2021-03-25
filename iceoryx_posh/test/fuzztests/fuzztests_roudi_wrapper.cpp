// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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


#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "fuzztests_roudi_wrapper.hpp"
#include "fuzzing.hpp"
#include "fuzz_helper.hpp"
#include "cmdlineparserfuzzing.hpp"
#include <fstream>
#include <iostream>

unsigned const char TIMEOUT = 50; //5s for a Timeout
#define INTERFACE_NAME "/test"

int main (int argc, char* argv[])
{
	CmdLineParserFuzzing cmd;
	std::vector<std::string> allMessages = cmd.parseCmd(argc, argv);

	if(cmd.helpFlag == true)
	{
		return 1;
	}

	if(cmd.errorFlag == true)
	{
		std::cout << "No or wrong command lines were specified. Please use --help!" << std::endl;
		return -1;
	}

	if(allMessages.empty())
	{
		std::cout << "Please use -m [cl, stdin] to enter the input you want to send to the executable. If you use -m cl, then you also need use -i [INPUT_MESSAGE] or -c [PATH_To_File] to specify the message." << std::endl;
		return -1;
	}

	if (cmd.inputMode == inputMode::NONE)
	{
		std::cout << "Please use -m to specify the input. Please use --help to get more information." << std::endl;
		return -1;
	}

	if(cmd.inputMode == inputMode::CL and !cmd.cmdLineFlag)
	{
		std::cout << "Please use -i [INPUT_MESSAGE] or -c [PATH_To_File] to enter a String which you want to send to the interface. It is also possible to use -m stdin instead." << std::endl;
		return -1;
	}
	FuzzHelper aFuzzHelper;
	std::shared_ptr<RouDiFuzz> aRouDi;

	if (cmd.fuzzingAPI == fuzzingAPI::TOML)
	{
		if(!cmd.tomlFileFlag)
		{
			std::cout << "Please use -t [PATH_To_File] to specify a file where the messages are written to which are sent to the TOML configuration parser." << std::endl;
			return -1;
		}
		else
		{
			allMessages = aFuzzHelper.combineString(allMessages);
		}
	}

	if (cmd.fuzzingAPI == fuzzingAPI::UDS or cmd.fuzzingAPI == fuzzingAPI::COM) // Start RouDi
	{
		aRouDi = aFuzzHelper.startRouDiThread();
		unsigned char timeout = 0;
		while(!aFuzzHelper.checkIsRouDiRunning())
		{
			if(timeout >= TIMEOUT)
			{
				std::cout << "RouDI could not be started, program terminates!" << std::endl;
				return -1;
			}
			usleep(100000); //1/10 of a second
			timeout += 1;
		}
	}

	if(cmd.inputMode == inputMode::CL or cmd.inputMode == inputMode::STDIN)
	{
		Fuzzing aFuzzer;
		for(std::string aMessage: allMessages)
		{

			switch (cmd.fuzzingAPI)
		  	{
		  		case fuzzingAPI::COM:
		  	  	{
		  	  		iox::LogDebug() << "Messages sent to RouDi: " << aMessage;
		  	  		aFuzzer.fuzzingRouDiCom(aRouDi, aMessage);
		  	  	} break;

		  		case fuzzingAPI::UDS:
		  	  	{
					aFuzzer.fuzzingRouDiUDS(aMessage);
					iox::LogDebug() << "Messages sent to RouDi: " << aMessage;
		  	  	}  break;

		  		case fuzzingAPI::TOML:
		  		{
		  			aFuzzer.fuzzingTOMLParser(aMessage, cmd.tomlFile);
		  			iox::LogDebug() << "Messages sent to TOML Parser: " << aMessage;
		  		} break;

		  		default:
		  		{

		  			std::cout << "Error: Unkown Fuzzing API parameter" << std::endl;
		  			return -1;

		  		} break;
		  	};
		}

	}

	else
	{
		std::cout << "Error: Only stdin and command line are allowed to enter an input. Please use --help to get more information." << std::endl;
		return -1;
	}

}




