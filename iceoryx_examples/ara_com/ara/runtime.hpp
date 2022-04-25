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

#ifndef IOX_EXAMPLES_ARA_COM_RUNTIME_HPP
#define IOX_EXAMPLES_ARA_COM_RUNTIME_HPP

#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/pair.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <string>

namespace ara
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

    ara::com::ServiceHandleContainer<ara::com::FindServiceHandle>
    FindService(ara::core::String& serviceIdentifier, ara::core::String& instanceIdentifier) noexcept
    {
        ara::com::ServiceHandleContainer<ara::com::FindServiceHandle> serviceContainer;
        m_discovery.findService(
            serviceIdentifier,
            instanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) {
                serviceContainer.push_back({service.getServiceIDString(), service.getInstanceIDString()});
            },
            iox::popo::MessagingPattern::PUB_SUB);

        return serviceContainer;
    }

    ara::com::FindServiceHandle StartFindService(ara::com::FindServiceHandler<ara::com::FindServiceHandle> handler,
                                                 ara::core::String& serviceIdentifier,
                                                 ara::core::String& instanceIdentifier) noexcept
    {
        m_callbacks.push_back({handler, {serviceIdentifier, instanceIdentifier}});

        if (m_callbacks.size() == 1)
        {
            auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
            m_listener.attachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
                .or_else([](auto) {
                    std::cerr << "unable to attach discovery" << std::endl;
                    std::exit(EXIT_FAILURE);
                });
        }

        return ara::com::FindServiceHandle({serviceIdentifier, instanceIdentifier});
    }

    void StopFindService(ara::com::FindServiceHandle) noexcept
    {
        /// @todo use unique integer id in FindServiceHandle for easier adding/removal
        // auto iter = std::find(m_callbacks.begin(), m_callbacks.end(), handle);
        // if (iter != m_callbacks.end())
        // {
        //     m_callbacks.erase(iter);
        // }

        if (m_callbacks.empty())
        {
            m_listener.detachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
        }
    }

  private:
    explicit Runtime() noexcept = default;

    static void invokeCallback(iox::runtime::ServiceDiscovery*, Runtime* self)
    {
        // 1) Has the availability of one of the registered services changed?
        // 2) If yes, call the user-defined callback
        for (auto& callback : self->m_callbacks)
        {
            auto container = self->FindService(callback.second.serviceIdentifier, callback.second.instanceIdentifier);
            if (container.empty())
            {
                continue;
            }
            (callback.first)(
                container,
                ara::com::FindServiceHandle({callback.second.serviceIdentifier, callback.second.instanceIdentifier}));
        }
    }

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    iox::cxx::vector<
        iox::cxx::pair<ara::com::FindServiceHandler<ara::com::FindServiceHandle>, ara::com::FindServiceHandle>,
        100U>
        m_callbacks;
};
} // namespace ara

#endif // IOX_EXAMPLES_ARA_COM_RUNTIME_HPP
