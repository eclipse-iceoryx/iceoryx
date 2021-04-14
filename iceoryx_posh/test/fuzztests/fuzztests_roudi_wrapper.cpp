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

#include "fuzztests_roudi_wrapper.hpp"
#include "cmdlineparserfuzzing.hpp"
#include "fuzz_helper.hpp"
#include "fuzzing.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"

#include <fstream>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    constexpr unsigned char TIMEOUT = 50; // 5s for a Timeout
    CmdLineParserFuzzing cmd;
    std::vector<std::string> allMessages = cmd.parseCmd(argc, argv);

    if (cmd.getHelpFlag())
    {
        return EXIT_SUCCESS;
    }

    if (cmd.getErrorFlag())
    {
        std::cout << "No or wrong command lines were specified. Please use --help!" << std::endl;
        return EXIT_FAILURE;
    }

    if (allMessages.empty())
    {
        std::cout << "Please use -m [cl, stdin] to enter the input you want to send to the executable. If you use -m "
                     "cl, then you also need use -i [INPUT_MESSAGE] or -c [PATH_To_File] to specify the message."
                  << std::endl;
        return EXIT_FAILURE;
    }

    if (cmd.getInputMode() == InputMode::NONE)
    {
        std::cout << "Please use -m to specify the input. Please use --help to get more information." << std::endl;
        return EXIT_FAILURE;
    }

    if ((cmd.getInputMode() == InputMode::CL) && (!cmd.getCmdLineFlag()))
    {
        std::cout << "Please use -i [INPUT_MESSAGE] or -c [PATH_To_File] to enter a String which you want to send to "
                     "the interface. It is also possible to use -m stdin instead."
                  << std::endl;
        return EXIT_FAILURE;
    }
    FuzzHelper aFuzzHelper;
    std::shared_ptr<RouDiFuzz> aRouDi;

    if (cmd.getFuzzingAPI() == FuzzingApi::TOML)
    {
        if (!cmd.getTomlFileFlag())
        {
            std::cout << "Please use -t [PATH_To_File] to specify a file where the messages are written to which are "
                         "sent to the TOML configuration parser."
                      << std::endl;
            return EXIT_FAILURE;
        }
        else
        {
            allMessages = aFuzzHelper.combineString(allMessages);
        }
    }

    if ((cmd.getFuzzingAPI() == FuzzingApi::UDS) || (cmd.getFuzzingAPI() == FuzzingApi::COM)) // Start RouDi
    {
        aRouDi = aFuzzHelper.startRouDiThread();
        unsigned char timeout = 0;
        while (!aFuzzHelper.checkIsRouDiRunning())
        {
            if (timeout >= TIMEOUT)
            {
                std::cout << "RouDi could not be started, program terminates!" << std::endl;
                return EXIT_FAILURE;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 1/10 of a second
            timeout += 1;
        }
    }

    if ((cmd.getInputMode() == InputMode::CL) || (cmd.getInputMode() == InputMode::STDIN))
    {
        Fuzzing aFuzzer;
        for (std::string aMessage : allMessages)
        {
            switch (cmd.getFuzzingAPI())
            {
            case FuzzingApi::COM:
            {
                iox::LogDebug() << "Messages sent to RouDi: " << aMessage;
                aFuzzer.fuzzingRouDiCom(aRouDi, aMessage);
                break;
            }

            case FuzzingApi::UDS:
            {
                aFuzzer.fuzzingRouDiUDS(aMessage);
                iox::LogDebug() << "Messages sent to RouDi: " << aMessage;
                break;
            }

            case FuzzingApi::TOML:
            {
                aFuzzer.fuzzingTOMLParser(aMessage, cmd.getTomlFile());
                iox::LogDebug() << "Messages sent to TOML Parser: " << aMessage;
                break;
            }

            default:
            {
                std::cout << "Error: Unkown Fuzzing API parameter" << std::endl;
                return EXIT_FAILURE;
            }
            };
        }
    }

    else
    {
        std::cout << "Error: Only stdin and command line are allowed to enter an input. Please use --help to get more "
                     "information."
                  << std::endl;
        return EXIT_FAILURE;
    }
}
