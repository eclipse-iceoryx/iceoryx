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
#include "iceoryx_utils/platform/getopt.hpp"
#include "fuzztests_roudi_wrapper.hpp"
#include "cpptoml.h"
#include <fstream>
#include <iostream>

std::string const UDS_NAME = "/tmp/";
unsigned const char TIMEOUT = 50; //5s for a Timeout
#define INTERFACE_NAME "/test"


iox::log::LogLevel m_logLevel{iox::log::LogLevel::kOff};


StringToIPCMessage::StringToIPCMessage(const iox::ProcessName_t& name, const int64_t maxMessages, const int64_t messageSize) : iox::runtime::IpcInterfaceBase(name, maxMessages, messageSize)
{
}

RouDiFuzz::RouDiFuzz(iox::roudi::RouDiMemoryInterface& roudiMemoryInterface, iox::roudi::PortManager& portManager, iox::roudi::RouDi::RoudiStartupParameters aStartupParameter = {iox::roudi::MonitoringMode::OFF, false, iox::roudi::RouDi::RuntimeMessagesThreadStart::IMMEDIATE, iox::version::CompatibilityCheckLevel::OFF}) : iox::roudi::RouDi(roudiMemoryInterface, portManager, aStartupParameter)
{
}

void RouDiFuzz::processMessageFuzz(std::string aMessage)
{
  iox::runtime::IpcMessage ipcMessage;
  StringToIPCMessage::setMessageFromString(aMessage.c_str(), ipcMessage);
  iox::runtime::IpcMessageType cmd = iox::runtime::stringToIpcMessageType(ipcMessage.getElementAtIndex(0).c_str());
  std::string processName = ipcMessage.getElementAtIndex(1);
  iox::roudi::RouDi::processMessage(ipcMessage, cmd, iox::ProcessName_t(iox::cxx::TruncateToCapacity, processName));
}


std::vector<std::string> FuzzHelper::getStdInMessages()
{
   std::vector<std::string> stdInMessages;
   for (std::string line; std::getline(std::cin, line);)
   {
	  stdInMessages.push_back(line);
   }
   return stdInMessages;
}

std::shared_ptr<RouDiFuzz> FuzzHelper::startRouDiThread()
{
	static iox::roudi::IceOryxRouDiComponents m_rouDiComponents(iox::RouDiConfig_t().setDefaults());
	static iox::RouDiConfig_t m_config = iox::RouDiConfig_t().setDefaults();
	std::shared_ptr<RouDiFuzz> aRouDi(new RouDiFuzz(m_rouDiComponents.m_rouDiMemoryManager, m_rouDiComponents.m_portManager));
	return aRouDi;
}

bool FuzzHelper::checkIsRouDiRunning()
{
	Fuzzing aFuzzer;
	int udsStatus = aFuzzer.fuzzingRouDiUDS("Hello Roudi!");
	if (udsStatus == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::vector<std::string> FuzzHelper::combineString(std::vector<std::string> allMessages)
{
	std::string tempString = "";
	for(std::string aMessage: allMessages)
	{
		tempString += aMessage + "\n";
	}
	allMessages.clear();
	allMessages.emplace_back(tempString);
	return allMessages;
}


Fuzzing::Fuzzing()
{

}

void Fuzzing::fuzzingRouDiCom(std::shared_ptr<RouDiFuzz> aRouDi, std::string aMessage)
{
	if(aRouDi != nullptr)
	{
		aRouDi->processMessageFuzz(aMessage);
	}
	else
	{
		iox::LogDebug() << "Error, the Smart Pointer for RouDI which is used to call the method 'processMessage' is NULL";
	}
}

int Fuzzing::fuzzingRouDiUDS(std::string aMessage)
{
	 int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	 struct sockaddr aSockAddr;
	 aSockAddr.sa_family = AF_LOCAL;
	 std::string roudiName = UDS_NAME + iox::roudi::IPC_CHANNEL_ROUDI_NAME;
	 strncpy(aSockAddr.sa_data, roudiName.c_str(), sizeof(aSockAddr.sa_data));
	 int connectfd = connect(sockfd, &aSockAddr, sizeof(aSockAddr));
	 if (connectfd != -1)
	 {
		 sendto(sockfd, aMessage.c_str(), aMessage.length()+1, static_cast<int>(0), nullptr, static_cast<socklen_t>(0));
	 }
	 else
	 {
		 iox::LogDebug() << "Could not connect to RoudI";
	 }
	 return connectfd;
}

	   
void Fuzzing::fuzzing_TOML_parser(std::string aMessage, std::string tempFile)
{
	std::ofstream aTomlFile;
	aTomlFile.open(tempFile);
	std::cout << "Sent to TOML: " << aMessage << std::endl;
	if(aTomlFile.is_open())
	{
		aTomlFile << aMessage;
		cpptoml::parse_file(tempFile);
		aTomlFile.close();
	}
	else
	{
		iox::LogDebug() << "Cannot open file to send it to TOML Parser: " << tempFile;
	}
}

CmdLineParserFuzzing::CmdLineParserFuzzing()
{
	fuzzing_API = fuzzingAPI::NONE;
	input_mode = inputMode::NONE;
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
				 std::cout << "-h, --help								Display help." << std::endl;
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
					  fuzzing_API = fuzzingAPI::UDS;
				  }
				  else if (strcmp(optarg, "com") == 0)
				  {
					  fuzzing_API = fuzzingAPI::COM;
				  }
				  else if (strcmp(optarg, "toml") == 0)
				  {
					  fuzzing_API = fuzzingAPI::TOML;
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
					  input_mode = inputMode::STDIN;
					  FuzzHelper aFuzzHelper;
					  allMessages = aFuzzHelper.getStdInMessages();
				  }
				  else if (strcmp(optarg, "cl") == 0)
				  {
					  input_mode = inputMode::CL;
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

	if (cmd.input_mode == inputMode::NONE)
	{
		std::cout << "Please use -m to specify the input. Please use --help to get more information." << std::endl;
		return -1;
	}

	if(cmd.input_mode == inputMode::CL and !cmd.cmdLineFlag)
	{
		std::cout << "Please use -i [INPUT_MESSAGE] or -c [PATH_To_File] to enter a String which you want to send to the interface. It is also possible to use -m stdin instead." << std::endl;
		return -1;
	}
	iox::log::LogManager::GetLogManager().SetDefaultLogLevel(m_logLevel);
	FuzzHelper aFuzzHelper;
	std::shared_ptr<RouDiFuzz> aRouDi;

	if (cmd.fuzzing_API == fuzzingAPI::TOML)
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

	if (cmd.fuzzing_API == fuzzingAPI::UDS or cmd.fuzzing_API == fuzzingAPI::COM) // Start RouDi
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

	if(cmd.input_mode == inputMode::CL or cmd.input_mode == inputMode::STDIN)
	{
		Fuzzing aFuzzer;
		for(std::string aMessage: allMessages)
		{

			switch (cmd.fuzzing_API)
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
		  			aFuzzer.fuzzing_TOML_parser(aMessage, cmd.tomlFile);
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




