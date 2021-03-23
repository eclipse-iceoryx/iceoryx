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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <iostream>

bool killswitch = false;
constexpr char APP_NAME[] = "iox-ex-publisher";

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

void getRadarObject(RadarObject* const object, const double& val) noexcept
{
    *object = RadarObject(val, val, val);
}

int main()
{
    // Register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});

    double ct = 0.0;
    while (!killswitch)
    {
        ++ct;

        // API Usage #1
        //  * Retrieve a typed sample from shared memory.
        //  * Sample can be held until ready to publish.
        //  * Data is default constructed during loan
        auto result = publisher.loan();
        if (!result.has_error())
        {
            auto& sample = result.value();
            sample->x = ct;
            sample->y = ct;
            sample->z = ct;
            sample.publish();
        }
        else
        {
            auto error = result.get_error();
            // Ignore unused variable warning
            std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            // Do something with error
        }

        // API Usage #2
        //  * Retrieve a typed sample from shared memory and construct data in-place
        //  * Sample can be held until ready to publish.
        //  * Data is constructed with the aruments provided.
        result = publisher.loan(ct, ct, ct);
        if (!result.has_error())
        {
            result.value().publish();
        }
        else
        {
            auto error = result.get_error();
            // Ignore unused variable warning
            std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            // Do something with error
        }

        // API Usage #3
        //  * Retrieve a sample and provide the logic to immediately populate and publish it via a lambda.
        //
        publisher.loan()
            .and_then([&](auto& sample) {
                auto object = sample.get();
                // Do some stuff leading to eventually generating the data in the samples loaned memory...
                *object = RadarObject(ct, ct, ct);
                // ...then publish the sample
                sample.publish();
            })
            .or_else([](iox::popo::AllocationError) {
                // Do something with error.
            });


        // API Usage #4
        //  * Basic copy-and-publish. Useful for smaller data types.
        //
        auto object = RadarObject(ct, ct, ct);
        publisher.publishCopyOf(object).or_else([](iox::popo::AllocationError) {
            // Do something with error.
        });

        // API Usage #5
        //  * Provide a callable that will be used to populate the loaned sample.
        //  * The first argument of the callable must be T* and is the location that the callable should
        //      write its result to.
        //
        publisher.publishResultOf(getRadarObject, ct).or_else([](iox::popo::AllocationError) {
            // Do something with error.
        });
        publisher.publishResultOf([&ct](RadarObject* object) { *object = RadarObject(ct, ct, ct); })
            .or_else([](iox::popo::AllocationError) {
                // Do something with error.
            });


        std::cout << APP_NAME << " sent six times value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
