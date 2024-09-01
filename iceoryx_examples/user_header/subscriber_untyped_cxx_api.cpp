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

//! [iceoryx includes]
#include "user_header_and_payload_types.hpp"

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
//! [iceoryx includes]

#include <iostream>

int main()
{
    //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-user-header-untyped-subscriber";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create subscriber]
    iox::popo::UntypedSubscriber subscriber({"Example", "User-Header", "Timestamp"});
    //! [create subscriber]

    //! [poll subscriber for samples in a loop]
    while (!iox::hasTerminationRequested())
    {
        //! [take chunk]
        subscriber.take().and_then([&](auto& userPayload) {
            auto header =
                static_cast<const Header*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());

            auto data = static_cast<const Data*>(userPayload);

            std::cout << APP_NAME << " got value: " << data->fibonacci << " with timestamp "
                      << header->publisherTimestamp << "ms" << std::endl;

            subscriber.release(userPayload);
        });
        //! [take chunk]

        constexpr std::chrono::milliseconds SLEEP_TIME{100U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
    //! [poll subscriber for samples in a loop]

    return EXIT_SUCCESS;
}
