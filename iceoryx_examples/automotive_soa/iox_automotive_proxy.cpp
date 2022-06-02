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
//! [include proxy]
#include "minimal_proxy.hpp"
//! [include proxy]
#include "owl/runtime.hpp"

#include <iostream>

using namespace owl;
using namespace iox::cxx;

constexpr char APP_NAME[] = "iox-cpp-automotive-proxy";

int main()
{
    //! [create runtime]
    Runtime::GetInstance(APP_NAME);
    //! [create runtime]

    //! [wrap proxy]
    iox::concurrent::smart_lock<optional<MinimalProxy>> maybeProxy;
    //! [wrap proxy]

    optional<kom::FindServiceCallbackHandle> maybeHandle;

    //! [synchronous discovery]
    kom::InstanceIdentifier exampleInstanceSearchQuery(TruncateToCapacity, "ExampleInstance");
    std::cout << "Searching for instances of '" << MinimalProxy::SERVICE_IDENTIFIER << "' called '"
              << exampleInstanceSearchQuery.c_str() << "':" << std::endl;
    auto handleContainer = MinimalProxy::FindService(exampleInstanceSearchQuery);
    //! [synchronous discovery]

    if (!handleContainer.empty())
    {
        //! [create proxy]
        for (auto& handle : handleContainer)
        {
            std::cout << "  Found instance of service: '" << MinimalProxy::SERVICE_IDENTIFIER << "', '"
                      << handle.GetInstanceId().c_str() << "'" << std::endl;
            maybeProxy->emplace(handle);
            break;
        }
        //! [create proxy]
    }
    else
    {
        std::cout << "  Found no service(s), setting up asynchronous search with 'EnableFindServiceCallback'!"
                  << std::endl;
        auto callback = [&](kom::ServiceHandleContainer<owl::kom::ProxyHandleType> container,
                            kom::FindServiceCallbackHandle) -> void {
            //! [destroy proxy asynchronously]
            if (container.empty())
            {
                std::cout << "  Instance '" << maybeProxy->value().m_instanceIdentifier.c_str() << "' of service '"
                          << MinimalProxy::SERVICE_IDENTIFIER << "' has disappeared." << std::endl;
                maybeProxy->value().m_event.UnsetReceiveCallback();
                maybeProxy->reset();
                return;
            }
            //! [destroy proxy asynchronously]

            //! [create proxy asynchronously]
            for (auto& proxyHandle : container)
            {
                if (!maybeProxy->has_value())
                {
                    std::cout << "  Found instance of service: '" << MinimalProxy::SERVICE_IDENTIFIER << "', '"
                              << proxyHandle.GetInstanceId().c_str() << "'" << std::endl;
                    maybeProxy->emplace(proxyHandle);
                    break;
                }
            }
            //! [create proxy asynchronously]
        };

        //! [set up asynchronous search]
        auto handle = MinimalProxy::EnableFindServiceCallback(callback, exampleInstanceSearchQuery);
        //! [set up asynchronous search]
        maybeHandle.emplace(handle);
        std::cout << "  Waiting for instance called '" << exampleInstanceSearchQuery.c_str()
                  << "' to become available.." << std::endl;
    }
    //! [Field: create ints for computeSum]
    uint64_t addend1{0};
    uint64_t addend2{0};
    //! [Field: create ints for computeSum]

    while (!iox::posix::hasTerminationRequested())
    {
        {
            //! [gain exclusive access to proxy]
            auto proxyGuard = maybeProxy.getScopeGuard();
            if (proxyGuard->has_value())
            {
                auto& proxy = proxyGuard->value();
                //! [gain exclusive access to proxy]

                //! [Event: receiveCallback]
                auto onReceive = [&]() -> void {
                    proxy.m_event.TakeNewSamples([](const auto& topic) {
                        auto finish = std::chrono::steady_clock::now();
                        std::cout << "Event: value is " << topic->counter << std::endl;
                        auto duration =
                            std::chrono::duration_cast<std::chrono::nanoseconds>(finish - topic->sendTimestamp);
                        std::cout << "Event: latency (ns) is " << duration.count() << std::endl;
                    });
                };
                //! [Event: receiveCallback]

                //! [Event: set receiveCallback]
                if (!proxy.m_event.HasReceiveCallback())
                {
                    proxy.m_event.Subscribe(10U);
                    proxy.m_event.SetReceiveCallback(onReceive);
                }
                //! [Event: set receiveCallback]

                //! [Field: get and set data]
                auto fieldFuture = proxy.m_field.Get();
                try
                {
                    auto result = fieldFuture.get();
                    std::cout << "Field: value is " << result.counter << std::endl;

                    if (result.counter >= 4242)
                    {
                        result.counter++;
                        proxy.m_field.Set(result);
                        std::cout << "Field: value set to " << result.counter << std::endl;
                    }
                }
                catch (const std::future_error&)
                {
                    std::cout << "Empty future from field received, please start the 'iox-cpp-automotive-skeleton'."
                              << std::endl;
                }
                //! [Field: get and set data]

                //! [Method: call computeSum remotely]
                auto methodFuture = proxy.computeSum(addend1, addend2);
                try
                {
                    auto result = methodFuture.get();
                    std::cout << "Method: result of " << std::to_string(addend1) << " + " << std::to_string(addend2)
                              << " = " << result.sum << std::endl;
                }
                catch (const std::future_error&)
                {
                    std::cout << "Empty future from method received, please start the 'iox-cpp-automotive-skeleton'."
                              << std::endl;
                }
                //! [Method: call computeSum remotely]

                addend1 += addend2 + addend2;
                addend2++;

                std::cout << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //! [stop find service]
    if (maybeHandle.has_value())
    {
        MinimalProxy::DisableFindServiceCallback(maybeHandle.value());
    }
    //! [stop find service]
    return (EXIT_SUCCESS);
}
