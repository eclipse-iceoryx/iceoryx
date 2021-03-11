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


class StringToIPCMessage : public iox::runtime::IpcInterfaceBase
{
   public:
      StringToIPCMessage(const iox::ProcessName_t& name, const int64_t maxMessages, const int64_t messageSize);
   using iox::runtime::IpcInterfaceBase::setMessageFromString;
};


class RouDiFuzz : iox::roudi::RouDi
{
   public:
      RouDiFuzz(iox::roudi::RouDiMemoryInterface& roudiMemoryInterface, iox::roudi::PortManager& portManager, iox::roudi::RouDi::RoudiStartupParameters aStartupParameter);
      void processMessageFuzz( std::string aMessage);
};

class FuzzHelper
{

	public:
		std::vector<std::string> getStdInMessages();
		std::shared_ptr<RouDiFuzz>  startRouDiThread();
		std::vector<std::string> combineString(std::vector<std::string> allMessages);
		bool checkIsRouDiRunning();


};

class Fuzzing
{

   public:
      Fuzzing();
      void fuzzingRouDiCom(std::shared_ptr<RouDiFuzz> aRouDi, std::string aMessage);
      int fuzzingRouDiUDS(std::string aMessage);	   
      void fuzzing_TOML_parser(std::string toml_file, std::string temp_file);
};


class CmdLineParserFuzzing
{
	public:

		CmdLineParserFuzzing();
		std::vector<std::string> parseCmd(int argc, char* argv[]) noexcept;
		
		fuzzingAPI fuzzing_API;
		inputMode input_mode;
		bool errorFlag;
		bool cmdLineFlag;
		bool helpFlag;
		bool tomlFileFlag;
		std::vector<std::string> allMessages;
		std::string tomlFile;
 };


int main (int argc, char* argv[]);







