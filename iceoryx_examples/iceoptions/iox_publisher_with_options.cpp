// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-publisher-with-options";

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // create publisher with some options set
    iox::popo::PublisherOptions publisherOptions;

    // the publishers stores the last 10 samples for possible late joiners
    //! [history capacity]
    publisherOptions.historyCapacity = 10U;
    //! [history capacity]

    // when the publisher is created, it is not yet visible
    //! [offer on create]
    publisherOptions.offerOnCreate = false;
    //! [offer on create]

    // grouping of publishers and subscribers within a process
    //! [node name]
    publisherOptions.nodeName = "Pub_Node_With_Options";
    //! [node name]

    //  we allow the subscribers to block the publisher if they want to ensure that no samples are lost
    //! [too slow policy]
    publisherOptions.subscriberTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    //! [too slow policy]

    //! [create publisher with options]
    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"}, publisherOptions);
    //! [create publisher with options]

    // we have to explicitely offer the publisher for making it visible to subscribers
    //! [offer]
    publisher.offer();
    //! [offer]

    double ct = 0.0;

    std::thread publishData([&]() {
        while (!iox::hasTerminationRequested())
        {
            ++ct;

            // Retrieve a sample, construct it with the given arguments and publish it via a lambda.
            publisher.loan(ct, ct, ct).and_then([](auto& sample) { sample.publish(); });

            std::cout << APP_NAME << " sent value: " << ct << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
    });

    iox::waitForTerminationRequest();

    // this is optional, but since the iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER option is used,
    // a slow subscriber might block the shutdown and this call unblocks the publisher
    //! [shutdown]
    iox::runtime::PoshRuntime::getInstance().shutdown();
    //! [shutdown]

    publishData.join();

    return 0;
}
