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

#include "fuzzing.hpp"
#include "cpptoml.h"
#include <fstream>
#include <sys/socket.h>
#include <thread>

std::string const UDS_NAME = "/tmp/";

Fuzzing::Fuzzing()
{
}

void Fuzzing::fuzzingRouDiCom(std::shared_ptr<RouDiFuzz> aRouDi, std::string aMessage)
{
    if (aRouDi != nullptr)
    {
        aRouDi->processMessageFuzz(aMessage);
    }
    else
    {
        iox::LogDebug()
            << "Error, the Smart Pointer for RouDi which is used to call the method 'processMessage' is NULL";
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
        sendto(
            sockfd, aMessage.c_str(), aMessage.length() + 1, static_cast<int>(0), nullptr, static_cast<socklen_t>(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // 0.5  second We need to wait otherwise it can happen that RouDi did not process the message
    }
    else
    {
        iox::LogDebug() << "Could not connect to RoudI";
    }
    return connectfd;
}


void Fuzzing::fuzzingTOMLParser(std::string aMessage, std::string tempFile)
{
    std::ofstream aTomlFile;
    aTomlFile.open(tempFile);
    std::cout << "Sent to TOML: " << aMessage << std::endl;
    if (aTomlFile.is_open())
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
