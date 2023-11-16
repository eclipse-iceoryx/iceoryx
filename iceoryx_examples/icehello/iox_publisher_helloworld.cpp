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

//! [include topic]
#include "topic_data.hpp"
//! [include topic]

//! [include sig watcher]
#include "iox/signal_watcher.hpp"
//! [include sig watcher]

//! [include]
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
//! [include]

#include <iostream>


int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-publisher-helloworld";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create publisher]
    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
    //! [create publisher]

    double ct = 0.0;
    //! [wait for term]
    while (!iox::hasTerminationRequested())
    //! [wait for term]
    {
        ++ct;

        // Retrieve a sample from shared memory
        //! [loan]
        auto loanResult = publisher.loan();
        //! [loan]
        //! [publish]
        if (loanResult.has_value())
        {
            auto& sample = loanResult.value();
            // Sample can be held until ready to publish
            sample->x = ct;
            sample->y = ct;
            sample->z = ct;
            sample.publish();
        }
        //! [publish]
        //! [error]
        else
        {
            auto error = loanResult.error();
            // Do something with error
            std::cerr << "Unable to loan sample, error code: " << error << std::endl;
        }
        //! [error]

        //! [msg]
        std::cout << APP_NAME << " sent value: " << ct << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        //! [msg]
    }

    return 0;
}
