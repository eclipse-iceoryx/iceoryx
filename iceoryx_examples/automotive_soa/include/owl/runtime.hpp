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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <tuple>

namespace owl
{
/// @note Once a handler has been set with 'StartFindService', calling 'FindService' is not thread-safe!
class Runtime
{
  private:
    using NumberOfAvailableServicesOnLastSearch = iox::cxx::optional<uint64_t>;
    using CallbackEntryType = std::tuple<kom::FindServiceHandler<kom::ProxyHandleType>,
                                         kom::FindServiceHandle,
                                         NumberOfAvailableServicesOnLastSearch>;

  public:
    static Runtime& GetInstance(const core::String& name) noexcept;
    static Runtime& GetInstance() noexcept;

    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    /// @brief Performs synchronous search for specific instance of a service
    /// @param[in] serviceIdentifier string of service to search for
    /// @param[in] instanceIdentifier string of instance to search for
    /// @return Container with an entry for each found instance
    kom::ServiceHandleContainer<kom::ProxyHandleType> FindService(kom::ServiceIdentifier& serviceIdentifier,
                                                                  kom::InstanceIdentifier& instanceIdentifier) noexcept;

    /// @brief Sets up an asychronous search for specific instance of a service
    /// @param[in] handler callback which shall be executed when the availabilty of the specific instance of a service
    /// has changed
    /// @param[in] serviceIdentifier string of service to search for
    /// @param[in] instanceIdentifier string of instance to search for
    /// @return Handle which can be used to stop an ongoing search
    /// @note ABA problem might occur: Available service which becomes unavailable during search and hence is not found
    kom::FindServiceHandle StartFindService(kom::FindServiceHandler<kom::ProxyHandleType> handler,
                                            kom::ServiceIdentifier& serviceIdentifier,
                                            kom::InstanceIdentifier& instanceIdentifier) noexcept;

    /// @brief Stops an asychronous search for specific instance of a service
    /// @param[in] handle instance of service which shall be stopped
    void StopFindService(kom::FindServiceHandle handle) noexcept;

  private:
    explicit Runtime() noexcept = default;

    bool verifyThatServiceIsComplete(kom::ServiceHandleContainer<kom::ProxyHandleType>& container) noexcept;

    static void invokeCallback(iox::runtime::ServiceDiscovery*, Runtime* self) noexcept;

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    // A vector is not the optimal data structure but used here for simplicity
    iox::cxx::vector<CallbackEntryType, iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER> m_callbacks;
};
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
