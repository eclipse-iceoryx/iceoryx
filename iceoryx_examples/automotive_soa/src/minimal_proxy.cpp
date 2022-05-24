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

#include "minimal_proxy.hpp"

constexpr char MinimalProxy::m_serviceIdentifier[];

MinimalProxy::MinimalProxy(owl::kom::ProxyHandleType& handle) noexcept
    : m_instanceIdentifier(handle.GetInstanceId())
{
    if (handle.GetServiceId() != owl::kom::ServiceIdentifier{iox::cxx::TruncateToCapacity, m_serviceIdentifier})
    {
        std::cerr << "Handle does not match MinimalProxy class. Can't construct MinimalProxy, terminating!"
                  << std::endl;
        std::terminate();
    }
}

owl::kom::FindServiceHandle
MinimalProxy::StartFindService(owl::kom::FindServiceHandler<owl::kom::ProxyHandleType> handler,
                               owl::kom::InstanceIdentifier& instanceIdentifier) noexcept
{
    owl::kom::ServiceIdentifier serviceIdentifier{iox::cxx::TruncateToCapacity, m_serviceIdentifier};
    return owl::Runtime::GetInstance().StartFindService(handler, serviceIdentifier, instanceIdentifier);
}

void MinimalProxy::StopFindService(owl::kom::FindServiceHandle handle) noexcept
{
    owl::Runtime::GetInstance().StopFindService(handle);
}

owl::kom::ServiceHandleContainer<owl::kom::ProxyHandleType>
MinimalProxy::FindService(owl::kom::InstanceIdentifier& instanceIdentifier) noexcept
{
    owl::kom::ServiceIdentifier serviceIdentifier{iox::cxx::TruncateToCapacity, m_serviceIdentifier};
    return owl::Runtime::GetInstance().FindService(serviceIdentifier, instanceIdentifier);
}
