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

//! [include topic data]
#include "topic_data.hpp"
//! [include topic data]

//! [includes]
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [includes]

#include <iostream>

int main()
{
    //! [runtime initialization]
    constexpr char APP_NAME[] = "iox-cpp-publisher-untyped";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [runtime initialization]

    //! [create untyped publisher]
    iox::popo::UntypedPublisher publisher({"Radar", "FrontLeft", "Object"});
    //! [create untyped publisher]

    double ct = 0.0;
    while (!iox::hasTerminationRequested())
    {
        ++ct;

        //! [Loan chunk and provide logic to populate it via a lambda]
        publisher.loan(sizeof(RadarObject))
            .and_then([&](auto& userPayload) {
                //! [construct RadarObject]
                RadarObject* data = new (userPayload) RadarObject(ct, ct, ct);
                //! [construct RadarObject]

                //! [write data]
                data->x = ct;
                data->y = ct;
                data->z = ct;
                //! [write data]

                //! [publish]
                publisher.publish(userPayload);
                //! [publish]
            })
            .or_else([&](auto& error) {
                //! [print error]
                std::cerr << "Unable to loan sample, error code: " << error << std::endl;
                //! [print error]
            });
        //! [Loan chunk and provide logic to populate it via a lambda]

        std::cout << APP_NAME << " sent two times value: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
