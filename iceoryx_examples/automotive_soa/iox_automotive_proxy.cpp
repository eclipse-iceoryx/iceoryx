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

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "minimal_proxy.hpp"
#include "owl/runtime.hpp"

#include <iostream>

using namespace owl;
using namespace iox::cxx;

constexpr char APP_NAME[] = "iox-cpp-automotive-proxy";

int main()
{
    Runtime::GetInstance(APP_NAME);

    optional<MinimalProxy> maybeProxy;
    optional<kom::FindServiceHandle> maybeHandle;

    // 1) Discover the available services
    core::String searchString(TruncateToCapacity, "Instance");
    std::cout << "Searching for instances called '" << searchString.c_str() << "':" << std::endl;
    auto handleContainer = MinimalProxy::FindService(searchString);

    if (!handleContainer.empty())
    {
        // 2) If available, create proxy from handle
        for (auto& handle : handleContainer)
        {
            std::cout << "  Found service: '" << handle.serviceIdentifier.c_str() << "', '"
                      << handle.instanceIdentifier.c_str() << "'" << std::endl;
            maybeProxy.emplace(handle);
        }
    }
    else
    {
        // 2) If not available yet, setup asychronous search to be notified when the service becomes available
        std::cout << "  Found no service(s), setting up asynchronous search with 'StartFindService'!" << std::endl;
        auto callback = [&](kom::ServiceHandleContainer<kom::FindServiceHandle> container,
                            kom::FindServiceHandle) -> void {
            /// @todo What is the 2nd parameter needed for?
            if (container.empty())
            {
                return;
            }

            for (auto& entry : container)
            {
                std::cout << "  Found service: '" << entry.serviceIdentifier.c_str() << "', '"
                          << entry.instanceIdentifier.c_str() << "'" << std::endl;
            }
        };

        auto handle = MinimalProxy::StartFindService(callback, searchString);
        maybeProxy.emplace(handle);
        maybeHandle.emplace(handle);
    }

    if (maybeProxy.has_value())
    {
        auto& proxy = maybeProxy.value();
        proxy.m_event.Subscribe(10U);

        uint64_t addend1{0};
        uint64_t addend2{0};

        while (!iox::posix::hasTerminationRequested())
        {
            // Event
            proxy.m_event.GetNewSamples(
                [](const auto& topic) { std::cout << "Receiving event: " << topic->counter << std::endl; });

            // Field
            proxy.m_field.GetNewSamples(
                [](const auto& field) { std::cout << "Receiving field: " << field->counter << std::endl; });

            // Method
            auto future = proxy.computeSum(addend1, addend2);
            try
            {
                auto result = future.get();
                std::cout << "Result of " << std::to_string(addend1) << " + " << std::to_string(addend2) << " is "
                          << result.sum << std::endl;
            }
            catch (const std::future_error&)
            {
                std::cout << "Empty future received, please start the 'iox-cpp-automotive-skeleton' before '"
                          << APP_NAME << "'" << std::endl;
            }

            addend1 += addend2 + addend2;
            addend2++;

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    if (maybeHandle.has_value())
    {
        MinimalProxy::StopFindService(maybeHandle.value());
    }
    return (EXIT_SUCCESS);
}
