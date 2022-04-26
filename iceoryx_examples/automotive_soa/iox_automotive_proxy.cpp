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
    core::String searchString(TruncateToCapacity, "Example");
    std::cout << "Searching for instances of '" << MinimalProxy::m_serviceIdentifier << "' called '"
              << searchString.c_str() << "':" << std::endl;
    auto handleContainer = MinimalProxy::FindService(searchString);

    if (!handleContainer.empty())
    {
        // 2a) If available, create proxy from MinimalProxyHandle
        for (auto& handle : handleContainer)
        {
            std::cout << "  Found instance of service: '" << MinimalProxy::m_serviceIdentifier << "', '"
                      << handle.GetInstanceId().c_str() << "'" << std::endl;
            maybeProxy->emplace(handle);
        }
    }
    else
    {
        // 2b) If not available yet, setup asychronous search to be notified when the service becomes available
        std::cout << "  Found no service(s), setting up asynchronous search with 'StartFindService'!" << std::endl;
        auto callback = [&](kom::ServiceHandleContainer<owl::kom::ProxyHandleType> container,
                            kom::FindServiceHandle) -> void {
            if (container.empty())
            {
                if (!maybeProxy->has_value())
                {
                    std::cout << "  No instance of service '" << MinimalProxy::m_serviceIdentifier
                              << "' is available yet." << std::endl;
                    return;
                }
                std::cout << "  Instance '" << maybeProxy->value().m_instanceIdentifier.c_str() << "' of service '"
                          << MinimalProxy::m_serviceIdentifier << "' has disappeared." << std::endl;
                maybeProxy->reset();
                return;
            }

            for (auto& proxyHandle : container)
            {
                if (!maybeProxy->has_value())
                {
                    std::cout << "  Found instance of service: '" << MinimalProxy::m_serviceIdentifier << "', '"
                              << proxyHandle.GetInstanceId().c_str() << "'" << std::endl;
                    maybeProxy->emplace(proxyHandle);
                }
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
        {
            auto proxyGuard = maybeProxy.getScopeGuard();
            if (proxyGuard->has_value())
            {
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
                    std::cout << "Empty future received, please start the 'iox-cpp-automotive-skeleton'." << std::endl;
                }

                addend1 += addend2 + addend2;
                addend2++;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (maybeHandle.has_value())
    {
        MinimalProxy::StopFindService(maybeHandle.value());
    }
    return (EXIT_SUCCESS);
}
