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

//! [iceoryx includes]
#include "request_and_response_types.hpp"

#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [iceoryx includes]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-request-response-server-basic";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create server]
    iox::popo::Server<AddRequest, AddResponse> server({"Example", "Request-Response", "Add"});
    //! [create server]

    //! [process requests in a loop]
    while (!iox::hasTerminationRequested())
    {
        //! [take request]
        server.take().and_then([&](const auto& request) {
            std::cout << APP_NAME << " Got Request: " << request->augend << " + " << request->addend << std::endl;

            //! [send response]
            server.loan(request)
                .and_then([&](auto& response) {
                    response->sum = request->augend + request->addend;
                    std::cout << APP_NAME << " Send Response: " << response->sum << std::endl;
                    response.send().or_else(
                        [&](auto& error) { std::cout << "Could not send Response! Error: " << error << std::endl; });
                })
                .or_else([&](auto& error) {
                    std::cout << APP_NAME << "Could not allocate Response! Error: " << error << std::endl;
                });
            //! [send response]
        });
        //! [take request]

        constexpr std::chrono::milliseconds SLEEP_TIME{100U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
    //! [process requests in a loop]

    return EXIT_SUCCESS;
}
