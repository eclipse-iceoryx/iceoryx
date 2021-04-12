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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

bool killswitch = false;
constexpr char APP_NAME[] = "iox-cpp-publisher-complexdata";

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

// void handle(const bool hasSuccess) {
// if(!hasSuccess) {
// std::cerr << "asdasd\n";
// std::terminate();
//}
//}

int main()
{
    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // initialize publisher
    iox::popo::Publisher<ComplexDataType> publisher({"Radar", "FrontLeft", "Object"});

    // run until interrupted by Ctrl-C
    while (!killswitch)
    {
        publisher.loan()
            .and_then([&](auto& sample) {
                // forward_list<string<10>, 5>
                ////// return values
                sample->stringForwardList.push_front("hello");
                sample->stringForwardList.push_front("world");
                // list<int64_t, 5>;
                // handle(sample->integerList.push_front(30));
                sample->integerList.push_front(30);
                sample->integerList.push_front(19);
                sample->integerList.push_front(90);
                // list<optional<uint32_t>, 5>
                sample->optionalList.push_front(42U);
                sample->optionalList.push_front(nullopt);
                // stack<float, 15>
                for (uint64_t i = 0U; i < sample->floatStack.capacity(); ++i)
                {
                    sample->floatStack.push(2008.11f);
                }
                // string<15>
                sample->someString = "hello iceoryx";
                // vector<double, 10>
                sample->doubleVector.emplace_back(2021.04);
                // vector<variant<string<10>, double>, 10>;
                sample->variantVector.emplace_back(in_place_index<0>(), "hello");
                sample->variantVector.emplace_back(in_place_index<1>(), 42.0);
                sample->variantVector.emplace_back(in_place_index<0>(), "bye");

                sample.publish();
            })
            .or_else([](auto& error) {
                // do something with error
                std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}
