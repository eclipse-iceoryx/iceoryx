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
#include "user_header_and_payload_types.hpp"

#include "iox/signal_watcher.hpp"
//! [include differs from typed C++ API]
#include "iceoryx_posh/popo/untyped_publisher.hpp"
//! [include differs from typed C++ API]
#include "iceoryx_posh/runtime/posh_runtime.hpp"
//! [iceoryx includes]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-user-header-untyped-publisher";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create publisher]
    iox::popo::UntypedPublisher publisher({"Example", "User-Header", "Timestamp"});
    //! [create publisher]

    //! [send samples in a loop]
    uint64_t timestamp = 73;
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    while (!iox::hasTerminationRequested())
    {
        auto fibonacciNext = fibonacciCurrent + fibonacciLast;
        fibonacciLast = fibonacciCurrent;
        fibonacciCurrent = fibonacciNext;

        //! [loan chunk]
        publisher.loan(sizeof(Data), alignof(Data), sizeof(Header), alignof(Header))
            .and_then([&](auto& userPayload) {
                //! [loan was successful]
                auto header = static_cast<Header*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
                header->publisherTimestamp = timestamp;

                auto data = static_cast<Data*>(userPayload);
                data->fibonacci = fibonacciCurrent;

                publisher.publish(userPayload);

                std::cout << APP_NAME << " sent data: " << fibonacciCurrent << " with timestamp " << timestamp << "ms"
                          << std::endl;
                //! [loan was successful]
            })
            .or_else([&](auto& error) {
                //! [loan failed]
                std::cout << APP_NAME << " could not loan chunk! Error code: " << error << std::endl;
                //! [loan failed]
            });
        //! [loan chunk]

        constexpr uint64_t MILLISECONDS_SLEEP{1000U};
        std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_SLEEP));
        timestamp += MILLISECONDS_SLEEP;
    }
    //! [send samples in a loop]

    return EXIT_SUCCESS;
}
