// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <iostream>

bool killswitch = false;
constexpr char APP_NAME[] = "iox-ex-publisher-untyped";

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // Register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::popo::UntypedPublisher publisher({"Radar", "FrontLeft", "Object"});

    double ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // API Usage #1
        //  * Loaned chunk can be held until ready to publish
        auto result = publisher.loan(sizeof(RadarObject));
        if (!result.has_error())
        {
            // In the untyped API we get a void pointer to the user-payload, therefore the data must be constructed
            // in place
            void* chunk = result.value();
            RadarObject* data = new (chunk) RadarObject(ct, ct, ct);
            iox::cxx::Expects(chunk == data);

            data->x = 1.0;
            data->y = 2.0;
            data->z = 3.0;
            publisher.publish(chunk);
        }
        else
        {
            auto error = result.get_error();
            // Ignore unused variable warning
            std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            // Do something with the error
        }


        // API Usage #2
        // * Loan chunk and provide logic to populate it via a lambda
        publisher.loan(sizeof(RadarObject))
            .and_then([&](auto& chunk) {
                RadarObject* data = new (chunk) RadarObject(ct, ct, ct);
                iox::cxx::Expects(chunk == data);

                data->x = 1.0;
                data->y = 2.0;
                data->z = 3.0;
                publisher.publish(chunk);
            })
            .or_else([&](iox::popo::AllocationError error) {
                // Ignore unused variable warning
                std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
                // Do something with the error
            });

        std::cout << APP_NAME << " sent two times value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
