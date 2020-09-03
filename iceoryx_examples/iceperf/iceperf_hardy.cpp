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

#include "iceoryx.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "mq.hpp"
#include "topic_data.hpp"
#include "uds.hpp"

#include <chrono>
#include <iostream>

constexpr char APP_NAME[] = "/hardy";
constexpr char PUBLISHER[] = "Hardy";
constexpr char SUBSCRIBER[] = "Laurel";

void followerDo(IcePerfBase& ipcTechnology)
{
    ipcTechnology.initFollower();

    ipcTechnology.pingPongFollower();

    ipcTechnology.shutdown();
}

int main()
{
#ifndef __APPLE__
    MQ mq("/" + std::string(PUBLISHER), "/" + std::string(SUBSCRIBER));
    std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
    followerDo(mq);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // ensure leader first
#endif

    std::cout << std::endl << "****** UNIX DOMAIN SOCKET ********" << std::endl;
    UDS uds("/tmp/" + std::string(PUBLISHER), "/tmp/" + std::string(SUBSCRIBER));
    followerDo(uds);

    std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
    iox::runtime::PoshRuntime::getInstance(APP_NAME); // runtime for registering with the RouDi daemon
    Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);
    followerDo(iceoryx);

    return (EXIT_SUCCESS);
}
