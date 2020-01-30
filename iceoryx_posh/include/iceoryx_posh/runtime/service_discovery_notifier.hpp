// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"

#include <atomic>

namespace iox
{
namespace runtime
{
/// @brief descriptor to store per service, which is registered for SD notification
struct ServiceHandlerDescription
{
    // callback handler , registered by Proxy
    FindServiceHandler callbackHandler;
    // serviceId , which is registered for SD notification
    IdString serviceId;
    // Last known available instances (updated using FindService())
    InstanceContainer instances;
    // handle, that needs to be passed to callback
    FindServiceHandle handle;
};

/// @brief This runtime extension provides service discovery notification in PoshRuntime.
/// This can not be used independently, always associated with PoshRuntime.
class ServiceDiscoveryNotifier
{
  public:
    /// @brief register handler , which will be called when the service availability , as specified by
    /// serviceDescription, changes
    /// @param[in] handler , to be called when the service availability changes
    /// @param[in] serviceId , service to monitor
    /// @param[out] a handle for this search/find request, which shall be used to stop the availability monitoring and
    /// related firing of the given handler.
    cxx::expected<FindServiceHandle, Error> startFindService(const FindServiceHandler& handler,
                                                             const IdString& serviceId) noexcept;

    /// @brief Method to stop finding service request (see above)
    /// @param[in] handle , identifier for the startFindService request
    /// @note worng handle will be silently ignored
    void stopFindService(const FindServiceHandle handle) noexcept;

    /// @brief Constructor
    /// @param[in] name , application name.
    /// @param[in] serviceRegistryChangeCounter, pointer to serviceRegistryChangeCounter
    /// PoshRuntime object)
    ServiceDiscoveryNotifier(const std::string& name,
                             const std::atomic<uint64_t>* serviceRegistryChangeCounter) noexcept;

  public:
    ServiceDiscoveryNotifier(const ServiceDiscoveryNotifier&) = delete;
    ServiceDiscoveryNotifier& operator=(const ServiceDiscoveryNotifier&) = delete;
    ServiceDiscoveryNotifier(ServiceDiscoveryNotifier&&) = delete;
    ServiceDiscoveryNotifier& operator=(ServiceDiscoveryNotifier&&) = delete;

  private:
    // Cyclic function, this calculates delta of service instances , in case of change in service registry
    void serviceDiscoveryNotifier() noexcept;

    // name , identifier for the instance.
    std::string m_appName;

    // Pointer to service registry change counter. This points to an object in shared memory.
    // Pointer to this is fetched during initialisation.
    const std::atomic<uint64_t>* m_serviceRegistryChangeCounter{nullptr};

    // Last known value of the counter
    uint64_t m_changeCountAtLastServiceDiscovery{0};


    // Store descriptor per registered service
    iox::cxx::vector<ServiceHandlerDescription, MAX_START_FIND_SERVICE_CALLBACKS> m_handlers;

    // Mutex to synchronise access to m_handlers
    mutable std::mutex m_queueAccessMutex;

    // used for starting the timer, first time StartFindService() is called.
    /// @todo check if isRunning() method in Timer might be a better idea
    bool m_serviceDiscoveryTimerActive;

    // counter used to generate handle for startFindService request
    FindServiceHandle m_handleCounter;

    /// @note the m_serviceDiscovery should always be the last member, so that it will be the first member to be
    /// detroyed
    iox::posix::Timer m_serviceDiscovery{DISCOVERY_INTERVAL, [this]() { this->serviceDiscoveryNotifier(); }};
};

} // namespace runtime
} // namespace iox
