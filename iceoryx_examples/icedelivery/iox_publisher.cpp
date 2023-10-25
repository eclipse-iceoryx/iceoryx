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

//! [include publisher]
#include "iceoryx_posh/popo/publisher.hpp"
//! [include publisher]
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-publisher";

void getRadarObject(RadarObject* const object, const double& val) noexcept
{
    *object = RadarObject(val, val, val);
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create publisher]
    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
    //! [create publisher]

    double ct = 0.0;
    while (!iox::hasTerminationRequested())
    {
        ++ct;
        double sampleValue1 = ct + 89;
        double sampleValue2 = ct + 144;
        double sampleValue3 = ct + 233;
        double sampleValue4 = ct + 377;

        //! [API Usage #1]
        //  * Retrieve a typed sample from shared memory.
        //  * Sample can be held until ready to publish.
        //  * Data is default constructed during loan
        publisher.loan()
            .and_then([&](auto& sample) {
                sample->x = sampleValue1;
                sample->y = sampleValue1;
                sample->z = sampleValue1;
                sample.publish();
            })
            .or_else([](auto& error) {
                // Do something with error
                std::cerr << "Unable to loan sample, error: " << error << std::endl;
            });
        //! [API Usage #1]


        //! [API Usage #2]
        //  * Retrieve a typed sample from shared memory and construct data in-place
        //  * Sample can be held until ready to publish.
        //  * Data is constructed with the arguments provided.
        publisher.loan(sampleValue2, sampleValue2, sampleValue2)
            .and_then([](auto& sample) { sample.publish(); })
            .or_else([](auto& error) {
                // Do something with error
                std::cerr << "Unable to loan sample, error: " << error << std::endl;
            });
        //! [API Usage #2]

        //! [API Usage #3]
        //  * Basic copy-and-publish. Useful for smaller data types.
        auto object = RadarObject(sampleValue3, sampleValue3, sampleValue3);
        publisher.publishCopyOf(object).or_else([](auto& error) {
            // Do something with error.
            std::cerr << "Unable to publishCopyOf, error: " << error << std::endl;
        });
        //! [API Usage #3]

        //! [API Usage #4]
        //  * Provide a callable that will be used to populate the loaned sample.
        //  * The first argument of the callable must be T* and is the location that the callable should
        //      write its result to.
        publisher.publishResultOf(getRadarObject, ct).or_else([](auto& error) {
            // Do something with error.
            std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
        });
        publisher
            .publishResultOf([&sampleValue4](RadarObject* object) {
                *object = RadarObject(sampleValue4, sampleValue4, sampleValue4);
            })
            .or_else([](auto& error) {
                // Do something with error.
                std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
            });
        //! [API Usage #4]


        std::cout << APP_NAME << " sent values: " << sampleValue1 << ", " << sampleValue2 << ", " << sampleValue3
                  << ", " << ct << ", " << sampleValue4 << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
