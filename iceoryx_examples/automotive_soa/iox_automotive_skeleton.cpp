// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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


#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"

//#include "owl/com/event_publisher.hpp"
#include "owl/runtime.hpp"
#include "minimal_skeleton.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-automotive-skeleton";

// owl::kom::InstanceIdentifier needed?

int main()
{
    owl::Runtime::GetInstance(APP_NAME);

    MinimalSkeleton skeleton;

    skeleton.OfferService();

    uint64_t counter = 0;
    while (!iox::posix::hasTerminationRequested())
    {
        ++counter;

        // // Event
        // auto sample = skeleton.m_event.Allocate();
        // (*sample).counter = counter;
        // skeleton.m_event.Send(std::move(sample));

        // // Field
        // if (counter > 30)
        // {
        //     Topic field{counter};
        //     skeleton.m_field.Update(field);
        // }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    skeleton.StopOfferService();

    return 0;
}
