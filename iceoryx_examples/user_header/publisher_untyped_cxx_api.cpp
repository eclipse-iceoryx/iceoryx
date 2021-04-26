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

#include "user_header_and_payload_types.hpp"

#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <atomic>
#include <iostream>

std::atomic<bool> killswitch{false};
constexpr char APP_NAME[] = "iox-cpp-user-header-untyped-publisher";

static void sigHandler(int sig IOX_MAYBE_UNUSED)
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // create the publisher
    iox::popo::UntypedPublisher publisher({"Example", "User-Header", "Timestamp"});

    uint64_t timestamp = 0;
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    while (!killswitch)
    {
        auto fibonacciNext = fibonacciCurrent + fibonacciLast;
        fibonacciLast = fibonacciCurrent;
        fibonacciCurrent = fibonacciNext;
        publisher.loan(sizeof(Data), alignof(Data), sizeof(Header), alignof(Header))
            .and_then([&](auto& userPayload) {
                auto header = static_cast<Header*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
                header->publisherTimestamp = timestamp;

                auto data = static_cast<Data*>(userPayload);
                data->fibonacci = fibonacciCurrent;

                publisher.publish(userPayload);

                std::cout << APP_NAME << " sent data: " << fibonacciCurrent << " with timestamp " << timestamp << "ms"
                          << std::endl;
            })
            .or_else([](auto& error) {
                std::cout << APP_NAME << " could not loan chunk! Error code: " << static_cast<uint64_t>(error)
                          << std::endl;
            });

        constexpr uint64_t SLEEP_TIME{1000U};
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        timestamp += SLEEP_TIME;
    }

    return EXIT_SUCCESS;
}
