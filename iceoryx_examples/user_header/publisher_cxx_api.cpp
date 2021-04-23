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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <atomic>
#include <iostream>

std::atomic<bool> killswitch{false};
constexpr char APP_NAME[] = "iox-cpp-user-header-publisher";

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

    // for the 1.0 release, the Publisher alias for the PublisherImpl does not have the second parameter for the Header,
    // therefore the PublisherImpl must be used directly
    iox::popo::PublisherImpl<Data, Header> publisher({"Example", "User-Header", "Timestamp"});

    auto startTime = std::chrono::steady_clock::now();
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    while (!killswitch)
    {
        auto fibonacciNext = fibonacciCurrent + fibonacciLast;
        fibonacciLast = fibonacciCurrent;
        fibonacciCurrent = fibonacciNext;

        publisher.loan(Data{fibonacciCurrent})
            .and_then([&](auto& sample) {
                auto elapsedTime = std::chrono::steady_clock::now() - startTime;
                auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
                sample.getUserHeader().publisherTimestamp = elapsedMilliseconds;
                sample.publish();

                std::cout << APP_NAME << " sent data: " << fibonacciCurrent << " with timestamp " << elapsedMilliseconds
                          << "ms" << std::endl;
            })
            .or_else([](auto& error) {
                std::cout << APP_NAME << " could not loan sample! Error code: " << static_cast<uint64_t>(error)
                          << std::endl;
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
