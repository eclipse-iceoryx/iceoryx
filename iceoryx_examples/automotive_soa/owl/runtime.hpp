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

#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/pair.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <string>

namespace owl
{
class Runtime
{
  public:
    static Runtime& GetInstance(const std::string& name) noexcept
    {
        iox::runtime::PoshRuntime::initRuntime(iox::RuntimeName_t(iox::cxx::TruncateToCapacity, name));
        static Runtime runtime;
        return runtime;
    }

    static Runtime& GetInstance() noexcept
    {
        static Runtime runtime;
        return runtime;
    }

    owl::kom::ServiceHandleContainer<owl::kom::FindServiceHandle>
    FindService(owl::core::String& serviceIdentifier, owl::core::String& instanceIdentifier) noexcept
    {
        owl::kom::ServiceHandleContainer<owl::kom::FindServiceHandle> serviceContainer;

        m_discovery.findService(
            serviceIdentifier,
            instanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) {
                serviceContainer.push_back({service.getServiceIDString(),
                                            service.getInstanceIDString(),
                                            iox::popo::MessagingPattern::PUB_SUB});
            },
            iox::popo::MessagingPattern::PUB_SUB);

        m_discovery.findService(
            serviceIdentifier,
            instanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) {
                serviceContainer.push_back({service.getServiceIDString(),
                                            service.getInstanceIDString(),
                                            iox::popo::MessagingPattern::REQ_RES});
            },
            iox::popo::MessagingPattern::REQ_RES);

        return serviceContainer;
    }

    owl::kom::FindServiceHandle StartFindService(owl::kom::FindServiceHandler<owl::kom::FindServiceHandle> handler,
                                                 owl::core::String& serviceIdentifier,
                                                 owl::core::String& instanceIdentifier) noexcept
    {
        m_callbacks.push_back({handler, {serviceIdentifier, instanceIdentifier, iox::popo::MessagingPattern::PUB_SUB}});
        m_callbacks.push_back({handler, {serviceIdentifier, instanceIdentifier, iox::popo::MessagingPattern::REQ_RES}});

        if (m_callbacks.size() == 2)
        {
            auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
            m_listener.attachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
                .or_else([](auto) {
                    std::cerr << "unable to attach discovery" << std::endl;
                    std::exit(EXIT_FAILURE);
                });
        }

        // We return a PUB_SUB FindServiceHandle, because the user can't access it and the message pattern is not
        // considered when passing it to the MinimalProxy c'tor
        return owl::kom::FindServiceHandle(
            {serviceIdentifier, instanceIdentifier, iox::popo::MessagingPattern::PUB_SUB});
    }

    void StopFindService(owl::kom::FindServiceHandle handle) noexcept
    {
        /// @todo use unique integer id in FindServiceHandle for easier adding/removal?
        ///       it is necessary to delete both PUB_SUB and REQ_RES!
        /// auto iter = std::find(m_callbacks.begin(), m_callbacks.end(), handle);
        auto iter = m_callbacks.begin();
        for (; iter != m_callbacks.end(); iter++)
        {
            if (iter->second.getServiceIdentifier() == handle.getInstanceIdentifer()
                && iter->second.getInstanceIdentifer() == handle.getInstanceIdentifer())
            {
                break;
            }
        }
        if (iter != m_callbacks.end())
        {
            m_callbacks.erase(iter);
        }

        if (m_callbacks.empty())
        {
            m_listener.detachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
        }
    }

  private:
    explicit Runtime() noexcept = default;

    /// @todo why is this method not called when all services are removed?
    static void invokeCallback(iox::runtime::ServiceDiscovery*, Runtime* self)
    {
        // 1) Has the availability of one of the registered services changed?
        // 2) If yes, call the user-defined callback
        for (auto& callback : self->m_callbacks)
        {
            auto container =
                self->FindService(callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier);
            if (container.empty())
            {
                continue;
            }
            (callback.first)(container,
                             owl::kom::FindServiceHandle({callback.second.m_serviceIdentifier,
                                                          callback.second.m_instanceIdentifier,
                                                          callback.second.m_pattern}));
        }
    }

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    iox::cxx::vector<
        iox::cxx::pair<owl::kom::FindServiceHandler<owl::kom::FindServiceHandle>, owl::kom::FindServiceHandle>,
        100U>
        m_callbacks;
};
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
