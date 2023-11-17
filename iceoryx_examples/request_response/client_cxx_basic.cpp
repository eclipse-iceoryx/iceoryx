// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [iceoryx includes]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-request-response-client-basic";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create client]
    iox::popo::Client<AddRequest, AddResponse> client({"Example", "Request-Response", "Add"});
    //! [create client]

    //! [send requests in a loop]
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    int64_t requestSequenceId = 0;
    int64_t expectedResponseSequenceId = requestSequenceId;
    while (!iox::hasTerminationRequested())
    {
        //! [send request]
        client.loan()
            .and_then([&](auto& request) {
                request.getRequestHeader().setSequenceId(requestSequenceId);
                expectedResponseSequenceId = requestSequenceId;
                requestSequenceId += 1;
                request->augend = fibonacciLast;
                request->addend = fibonacciCurrent;
                std::cout << APP_NAME << " Send Request: " << fibonacciLast << " + " << fibonacciCurrent << std::endl;
                request.send().or_else(
                    [&](auto& error) { std::cout << "Could not send Request! Error: " << error << std::endl; });
            })
            .or_else([](auto& error) { std::cout << "Could not allocate Request! Error: " << error << std::endl; });
        //! [send request]

        // the client polls with an interval of 150ms
        constexpr std::chrono::milliseconds DELAY_TIME{150U};
        std::this_thread::sleep_for(DELAY_TIME);

        //! [take response]
        while (client.take().and_then([&](const auto& response) {
            auto receivedSequenceId = response.getResponseHeader().getSequenceId();
            if (receivedSequenceId == expectedResponseSequenceId)
            {
                fibonacciLast = fibonacciCurrent;
                fibonacciCurrent = response->sum;
                std::cout << APP_NAME << " Got Response : " << fibonacciCurrent << std::endl;
            }
            else
            {
                std::cout << "Got Response with outdated sequence ID! Expected = " << expectedResponseSequenceId
                          << "; Actual = " << receivedSequenceId << "! -> skip" << std::endl;
            }
        }))
        {
        };
        //! [take response]

        constexpr std::chrono::milliseconds SLEEP_TIME{950U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
    //! [send requests in a loop]

    return EXIT_SUCCESS;
}
