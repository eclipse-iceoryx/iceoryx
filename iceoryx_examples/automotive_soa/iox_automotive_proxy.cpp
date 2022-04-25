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
#include "iceoryx_hoofs/internal/concurrent/smart_lock.hpp"
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

    iox::concurrent::smart_lock<optional<MinimalProxy>> maybeProxy;
    optional<kom::FindServiceHandle> maybeHandle;

    // 1) Discover the available services
    core::String searchString(TruncateToCapacity, "Instance");
    std::cout << "Searching for instances called '" << searchString.c_str() << "':" << std::endl;
    auto handleContainer = MinimalProxy::FindService(searchString);

    if (!handleContainer.empty())
    {
        // 2a) If available, create proxy from handle
        // We need to make sure that all three internal services representing 'MinimalSkeleton' are available
        uint32_t numberOfServices{0U};
        constexpr uint32_t expectedNumberOfServices{3U};
        for (auto& handle : handleContainer)
        {
            if (handle.getServiceIdentifier()
                    == owl::core::String(TruncateToCapacity, MinimalProxy::m_serviceIdentifier)
                && handle.getInstanceIdentifer() == owl::core::String(TruncateToCapacity, "Instance"))
            {
                numberOfServices++;
            }
            if (numberOfServices == expectedNumberOfServices)
            {
                std::cout << "  Found service: '" << handle.getServiceIdentifier().c_str() << "', '"
                          << handle.getInstanceIdentifer().c_str() << "'" << std::endl;
                maybeProxy->emplace(handle);
            }
        }
    }
    else
    {
        // 2b) If not available yet, setup asychronous search to be notified when the service becomes available
        std::cout << "  Found no service(s), setting up asynchronous search with 'StartFindService'!" << std::endl;
        auto callback = [&](kom::ServiceHandleContainer<kom::FindServiceHandle> container,
                            kom::FindServiceHandle handle) -> void {
            if (container.empty())
            {
                return;
            }

            // We need to make sure that all three internal services representing 'MinimalSkeleton' are available
            uint32_t numberOfServices{0U};
            constexpr uint32_t expectedNumberOfServices{3U};

            for (auto& entry : container)
            {
                if (entry.getServiceIdentifier()
                        == owl::core::String(TruncateToCapacity, MinimalProxy::m_serviceIdentifier)
                    && entry.getInstanceIdentifer() == owl::core::String(TruncateToCapacity, "Instance"))
                {
                    numberOfServices++;
                }
            }
            if (!maybeProxy->has_value() && numberOfServices == expectedNumberOfServices)
            {
                std::cout << "  Found complete service: '" << handle.getServiceIdentifier().c_str() << "', '"
                          << handle.getInstanceIdentifer().c_str() << "'" << std::endl;
                maybeProxy->emplace(handle);
            }
            else
            {
                std::cout << "  Service not available yet/anymore: '" << handle.getServiceIdentifier().c_str() << "', '"
                          << handle.getInstanceIdentifer().c_str() << "'" << std::endl;
                /// @todo what to do if services are gone after being there for some time?
                maybeProxy->reset();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        };

        auto handle = MinimalProxy::StartFindService(callback, searchString);
        maybeHandle.emplace(handle);
        std::cout << "  Waiting for instance called '" << searchString.c_str() << "' to become available.."
                  << std::endl;
    }

    uint64_t addend1{0};
    uint64_t addend2{0};

    while (!iox::posix::hasTerminationRequested())
    {
        if (maybeProxy->has_value())
        {
            auto proxyGuard = maybeProxy.getScopeGuard();
            auto& proxy = proxyGuard->value();
            proxy.m_event.Subscribe(10U);

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
